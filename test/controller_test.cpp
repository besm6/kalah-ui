#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QEventLoop>
#include <QObject>
#include "gamecontroller.h"

// Custom main — QCoreApplication must exist before any QObject is constructed.
int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// ── Helper ────────────────────────────────────────────────────────────────

static void advanceTo(GameController& gc, GameController::AppState target)
{
    if (target >= GameController::EnterName)        gc.proceedFromWelcome();
    if (target >= GameController::SelectGender)     gc.submitName("Tester");
    if (target >= GameController::SelectDifficulty) gc.selectGender(0);
    if (target >= GameController::Playing)          gc.selectLevel(1);
}

// ── Fixture ───────────────────────────────────────────────────────────────

class ControllerTest : public ::testing::Test {
protected:
    GameController* gc;

    void SetUp() override
    {
        gc = new GameController();
        advanceTo(*gc, GameController::Playing);
    }

    void TearDown() override
    {
        delete gc; // QTimer is a child QObject — deleted and stopped with gc
    }
};

// ── S: State Machine ──────────────────────────────────────────────────────

TEST(StateTest, Initial_IsWelcome)
{
    GameController gc;
    EXPECT_EQ(gc.appState(), static_cast<int>(GameController::Welcome));
}

TEST(StateTest, ProceedFromWelcome_GoesToEnterName)
{
    GameController gc;
    int sigCount = 0;
    QObject::connect(&gc, &GameController::appStateChanged, [&]{ sigCount++; });
    gc.proceedFromWelcome();
    EXPECT_EQ(gc.appState(), static_cast<int>(GameController::EnterName));
    EXPECT_EQ(sigCount, 1);
}

TEST(StateTest, SubmitName_GoesToSelectGender_AndSetsName)
{
    GameController gc;
    gc.proceedFromWelcome();
    gc.submitName("Alice");
    EXPECT_EQ(gc.appState(), static_cast<int>(GameController::SelectGender));
    EXPECT_EQ(gc.userName(), "Alice");
}

TEST(StateTest, SelectGender_GoesToSelectDifficulty)
{
    GameController gc;
    gc.proceedFromWelcome();
    gc.submitName("Alice");
    gc.selectGender(1);
    EXPECT_EQ(gc.appState(), static_cast<int>(GameController::SelectDifficulty));
}

TEST(StateTest, SelectLevel_GoesToPlaying_EmitsBoardChanged)
{
    GameController gc;
    gc.proceedFromWelcome();
    gc.submitName("Alice");
    gc.selectGender(0);
    int boardSignals = 0;
    QObject::connect(&gc, &GameController::boardChanged, [&]{ boardSignals++; });
    gc.selectLevel(1);
    EXPECT_EQ(gc.appState(), static_cast<int>(GameController::Playing));
    EXPECT_GE(boardSignals, 1);
}

TEST(StateTest, NewGame_ResetsToPlaying)
{
    GameController gc;
    advanceTo(gc, GameController::Playing);
    gc.sow(0); // make a move to dirty state (EXTRA_TURN, no AI timer)
    gc.newGame();
    EXPECT_EQ(gc.appState(), static_cast<int>(GameController::Playing));
    EXPECT_FALSE(gc.gameOver());
    EXPECT_EQ(gc.pits()[6].toInt(), 0);  // USER kalah reset
    EXPECT_EQ(gc.pits()[13].toInt(), 0); // JINN kalah reset
    EXPECT_FALSE(gc.aiThinking());
}

// ── P: pits Property ──────────────────────────────────────────────────────

TEST_F(ControllerTest, Pits_Size14)
{
    EXPECT_EQ(gc->pits().size(), 14);
}

TEST_F(ControllerTest, Pits_UserPitsAllSix)
{
    for (int i = 0; i < 6; ++i)
        EXPECT_EQ(gc->pits()[i].toInt(), 6) << "USER pit " << i;
}

TEST_F(ControllerTest, Pits_JinnPitsAllSix)
{
    for (int i = 7; i < 13; ++i)
        EXPECT_EQ(gc->pits()[i].toInt(), 6) << "JINN pit " << i;
}

TEST_F(ControllerTest, Pits_KalahsAreZero)
{
    EXPECT_EQ(gc->pits()[6].toInt(), 0);  // USER kalah
    EXPECT_EQ(gc->pits()[13].toInt(), 0); // JINN kalah
}

TEST_F(ControllerTest, Pits_TotalIs72)
{
    int sum = 0;
    for (int i = 0; i < 14; ++i) sum += gc->pits()[i].toInt();
    EXPECT_EQ(sum, 72);
}

// sow(0) on fresh board: USER[0]=6 → EXTRA_TURN (last stone in USER kalah)
TEST_F(ControllerTest, Pits_UpdateAfterExtraTurnMove)
{
    gc->sow(0);
    EXPECT_EQ(gc->pits()[0].toInt(), 0); // pit 0 emptied
    EXPECT_EQ(gc->pits()[1].toInt(), 7); // pit 1 incremented
    EXPECT_EQ(gc->pits()[6].toInt(), 1); // USER kalah got the extra stone
}

// ── M: Signals ────────────────────────────────────────────────────────────

// sow(0) = EXTRA_TURN: boardChanged must fire
TEST_F(ControllerTest, Signals_ValidMove_EmitsBoardChanged)
{
    int count = 0;
    QObject::connect(gc, &GameController::boardChanged, [&]{ count++; });
    gc->sow(0);
    EXPECT_GE(count, 1);
}

// sow(0) = EXTRA_TURN: currentPlayerChanged must NOT fire
TEST_F(ControllerTest, Signals_ExtraTurn_NoCurrentPlayerChanged)
{
    int count = 0;
    QObject::connect(gc, &GameController::currentPlayerChanged, [&]{ count++; });
    gc->sow(0);
    EXPECT_EQ(count, 0);
}

// sow(1) = SWITCH_TURN: currentPlayerChanged must fire exactly once
// (6 stones from USER pit 1: lands in pit2..5, kalah, JINN[0])
TEST_F(ControllerTest, Signals_SwitchTurn_CurrentPlayerChanged)
{
    int count = 0;
    QObject::connect(gc, &GameController::currentPlayerChanged, [&]{ count++; });
    gc->sow(1);
    EXPECT_EQ(count, 1);
    // newGame called by TearDown to stop the AI timer
}

// sow empty pit → illegalMove signal with correct index; no boardChanged
TEST_F(ControllerTest, Signals_IllegalMove_EmitsIllegalMove)
{
    gc->sow(0); // empties USER pit 0 (EXTRA_TURN)
    int illegal = 0, board = 0;
    QObject::connect(gc, &GameController::illegalMove, [&](int){ illegal++; });
    QObject::connect(gc, &GameController::boardChanged, [&]{ board++; });
    gc->sow(0); // pit 0 is now empty
    EXPECT_EQ(illegal, 1);
    EXPECT_EQ(board, 0);
}

TEST_F(ControllerTest, Signals_IllegalMove_CarriesPitIndex)
{
    gc->sow(0); // empties pit 0
    int idx = -999;
    QObject::connect(gc, &GameController::illegalMove, [&](int pitIndex){ idx = pitIndex; });
    gc->sow(0);
    EXPECT_EQ(idx, 0);
}

// While AI is thinking, sow is blocked — no boardChanged for that second call
TEST_F(ControllerTest, Signals_AiThinking_BlocksSow)
{
    gc->sow(1); // SWITCH_TURN → aiThinking=true
    int boardCount = 0;
    QObject::connect(gc, &GameController::boardChanged, [&]{ boardCount++; });
    gc->sow(2); // blocked by m_aiThinking
    EXPECT_EQ(boardCount, 0);
}

// ── W: Winner ─────────────────────────────────────────────────────────────

// Fresh board: both kalahs are 0 → tie
TEST_F(ControllerTest, Winner_EqualKalahs_ReturnsMinus1)
{
    EXPECT_EQ(gc->winner(), -1);
}

// After sow(0) EXTRA_TURN: USER kalah = 1, JINN kalah = 0 → USER wins
TEST_F(ControllerTest, Winner_UserKalahLeads_Returns0)
{
    gc->sow(0);
    EXPECT_EQ(gc->winner(), 0);
}

// ── T: Timing-Dependent ───────────────────────────────────────────────────
// TIMING: these tests allow the 700ms QTimer to fire via processEvents.

// Immediately after SWITCH_TURN, aiThinking must be true
TEST_F(ControllerTest, Timer_AiThinkingTrue_AfterSwitchTurn)
{
    gc->sow(1);
    EXPECT_TRUE(gc->aiThinking());
    // TearDown stops the timer
}

// After the timer fires, aiThinking must become false
TEST_F(ControllerTest, Timer_AiThinkingFalse_AfterTimerFires)
{
    // Run a local event loop until aiThinking clears or 2s safety-net fires.
    QEventLoop loop;
    QTimer guard;
    guard.setSingleShot(true);
    guard.start(2000);
    QObject::connect(&guard, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(gc, &GameController::aiThinkingChanged, [&]{
        if (!gc->aiThinking()) loop.quit();
    });
    gc->sow(1); // SWITCH_TURN → starts 700ms AI timer
    if (gc->aiThinking()) loop.exec();
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
