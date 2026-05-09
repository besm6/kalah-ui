#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <glibmm/main.h>

#include "gamecontroller.h"

// ── Helper ────────────────────────────────────────────────────────────────

static void advanceTo(GameController& gc, GameController::AppState target)
{
    if (target >= GameController::AppState::EnterName)
        gc.proceedFromWelcome();
    if (target >= GameController::AppState::SelectGender)
        gc.submitName("Tester");
    if (target >= GameController::AppState::SelectDifficulty)
        gc.selectGender(0);
    if (target >= GameController::AppState::Playing)
        gc.selectLevel(1);
}

// ── Fixture ───────────────────────────────────────────────────────────────

class ControllerTest : public ::testing::Test {
protected:
    GameController* gc;

    void SetUp() override
    {
        gc = new GameController();
        advanceTo(*gc, GameController::AppState::Playing);
    }

    void TearDown() override
    {
        delete gc; // destructor disconnects AI timer via m_ai_timer_conn.disconnect()
    }
};

// ── S: State Machine ──────────────────────────────────────────────────────

TEST(StateTest, Initial_IsWelcome)
{
    GameController gc;
    EXPECT_EQ(gc.appState(), static_cast<int>(GameController::AppState::Welcome));
}

TEST(StateTest, ProceedFromWelcome_GoesToEnterName)
{
    GameController gc;
    int sigCount = 0;
    gc.signal_app_state_changed.connect([&] { sigCount++; });
    gc.proceedFromWelcome();
    EXPECT_EQ(gc.appState(), static_cast<int>(GameController::AppState::EnterName));
    EXPECT_EQ(sigCount, 1);
}

TEST(StateTest, SubmitName_GoesToSelectGender_AndSetsName)
{
    GameController gc;
    gc.proceedFromWelcome();
    gc.submitName("Alice");
    EXPECT_EQ(gc.appState(), static_cast<int>(GameController::AppState::SelectGender));
    EXPECT_EQ(gc.userName(), "Alice");
}

TEST(StateTest, SelectGender_GoesToSelectDifficulty)
{
    GameController gc;
    gc.proceedFromWelcome();
    gc.submitName("Alice");
    gc.selectGender(1);
    EXPECT_EQ(gc.appState(), static_cast<int>(GameController::AppState::SelectDifficulty));
}

TEST(StateTest, SelectLevel_GoesToPlaying_EmitsBoardChanged)
{
    GameController gc;
    gc.proceedFromWelcome();
    gc.submitName("Alice");
    gc.selectGender(0);
    int boardSignals = 0;
    gc.signal_board_changed.connect([&] { boardSignals++; });
    gc.selectLevel(1);
    EXPECT_EQ(gc.appState(), static_cast<int>(GameController::AppState::Playing));
    EXPECT_GE(boardSignals, 1);
}

TEST(StateTest, NewGame_ResetsToPlaying)
{
    GameController gc;
    advanceTo(gc, GameController::AppState::Playing);
    gc.sow(0); // EXTRA_TURN, no AI timer
    gc.newGame();
    EXPECT_EQ(gc.appState(), static_cast<int>(GameController::AppState::Playing));
    EXPECT_FALSE(gc.gameOver());
    EXPECT_EQ(gc.pits()[6],  0);  // USER kalah reset
    EXPECT_EQ(gc.pits()[13], 0);  // JINN kalah reset
    EXPECT_FALSE(gc.aiThinking());
}

// ── P: pits Property ──────────────────────────────────────────────────────

TEST_F(ControllerTest, Pits_Size14)
{
    EXPECT_EQ(gc->pits().size(), 14u);
}

TEST_F(ControllerTest, Pits_UserPitsAllSix)
{
    for (int i = 0; i < 6; ++i)
        EXPECT_EQ(gc->pits()[i], 6) << "USER pit " << i;
}

TEST_F(ControllerTest, Pits_JinnPitsAllSix)
{
    for (int i = 7; i < 13; ++i)
        EXPECT_EQ(gc->pits()[i], 6) << "JINN pit " << i;
}

TEST_F(ControllerTest, Pits_KalahsAreZero)
{
    EXPECT_EQ(gc->pits()[6],  0);  // USER kalah
    EXPECT_EQ(gc->pits()[13], 0);  // JINN kalah
}

TEST_F(ControllerTest, Pits_TotalIs72)
{
    int sum = 0;
    for (int i = 0; i < 14; ++i)
        sum += gc->pits()[i];
    EXPECT_EQ(sum, 72);
}

// sow(0) on fresh board: USER[0]=6 → EXTRA_TURN (last stone in USER kalah)
TEST_F(ControllerTest, Pits_UpdateAfterExtraTurnMove)
{
    gc->sow(0);
    EXPECT_EQ(gc->pits()[0], 0); // pit 0 emptied
    EXPECT_EQ(gc->pits()[1], 7); // pit 1 incremented
    EXPECT_EQ(gc->pits()[6], 1); // USER kalah got the extra stone
}

// ── M: Signals ────────────────────────────────────────────────────────────

// sow(0) = EXTRA_TURN: boardChanged must fire
TEST_F(ControllerTest, Signals_ValidMove_EmitsBoardChanged)
{
    int count = 0;
    gc->signal_board_changed.connect([&] { count++; });
    gc->sow(0);
    EXPECT_GE(count, 1);
}

// sow(0) = EXTRA_TURN: currentPlayerChanged must NOT fire
TEST_F(ControllerTest, Signals_ExtraTurn_NoCurrentPlayerChanged)
{
    int count = 0;
    gc->signal_current_player_changed.connect([&] { count++; });
    gc->sow(0);
    EXPECT_EQ(count, 0);
}

// sow(1) = SWITCH_TURN: currentPlayerChanged must fire exactly once
TEST_F(ControllerTest, Signals_SwitchTurn_CurrentPlayerChanged)
{
    int count = 0;
    gc->signal_current_player_changed.connect([&] { count++; });
    gc->sow(1);
    EXPECT_EQ(count, 1);
}

// sow empty pit → illegalMove signal with correct index; no boardChanged
TEST_F(ControllerTest, Signals_IllegalMove_EmitsIllegalMove)
{
    gc->sow(0); // empties USER pit 0 (EXTRA_TURN)
    int illegal = 0, board = 0;
    gc->signal_illegal_move.connect([&](int) { illegal++; });
    gc->signal_board_changed.connect([&] { board++; });
    gc->sow(0); // pit 0 is now empty
    EXPECT_EQ(illegal, 1);
    EXPECT_EQ(board,   0);
}

TEST_F(ControllerTest, Signals_IllegalMove_CarriesPitIndex)
{
    gc->sow(0); // empties pit 0
    int idx = -999;
    gc->signal_illegal_move.connect([&](int pitIndex) { idx = pitIndex; });
    gc->sow(0);
    EXPECT_EQ(idx, 0);
}

// While AI is thinking, sow is blocked
TEST_F(ControllerTest, Signals_AiThinking_BlocksSow)
{
    gc->sow(1); // SWITCH_TURN → aiThinking=true
    int boardCount = 0;
    gc->signal_board_changed.connect([&] { boardCount++; });
    gc->sow(2); // blocked by m_aiThinking
    EXPECT_EQ(boardCount, 0);
}

// ── W: Winner ─────────────────────────────────────────────────────────────

TEST_F(ControllerTest, Winner_EqualKalahs_ReturnsMinus1)
{
    EXPECT_EQ(gc->winner(), -1);
}

TEST_F(ControllerTest, Winner_UserKalahLeads_Returns0)
{
    gc->sow(0);
    EXPECT_EQ(gc->winner(), 0);
}

// ── T: Timing-Dependent ───────────────────────────────────────────────────

// Immediately after SWITCH_TURN, aiThinking must be true
TEST_F(ControllerTest, Timer_AiThinkingTrue_AfterSwitchTurn)
{
    gc->sow(1);
    EXPECT_TRUE(gc->aiThinking());
}

// After the timer fires, aiThinking must become false
TEST_F(ControllerTest, Timer_AiThinkingFalse_AfterTimerFires)
{
    gc->sow(1); // SWITCH_TURN → starts 700ms AI timer
    auto ctx = Glib::MainContext::get_default();
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (gc->aiThinking() && std::chrono::steady_clock::now() < deadline) {
        ctx->iteration(false);
        if (gc->aiThinking())
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_FALSE(gc->aiThinking());
}

// newGame() immediately cancels the pending timer and clears aiThinking
TEST_F(ControllerTest, Timer_NewGame_StopsTimer)
{
    gc->sow(1);
    EXPECT_TRUE(gc->aiThinking());
    gc->newGame();
    EXPECT_FALSE(gc->aiThinking());
}
