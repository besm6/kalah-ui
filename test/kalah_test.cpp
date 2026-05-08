#include "kalah.h"

#include <gtest/gtest.h>

// ── Helpers ──────────────────────────────────────────────────────────────

static void setBoard(KalahGame &g, std::array<int, 6> userPits, int userKalah,
                     std::array<int, 6> jinnPits, int jinnKalah, Player current = Player::USER)
{
    Position &pos = g.getPosition();
    for (int i = 0; i < PITS_PER_SIDE; ++i)
        pos[Player::USER][i] = userPits[i];
    pos[Player::USER].setKalah(userKalah);
    for (int i = 0; i < PITS_PER_SIDE; ++i)
        pos[Player::JINN][i] = jinnPits[i];
    pos[Player::JINN].setKalah(jinnKalah);
    pos.currentPlayer = current;
}

static int totalStones(const Position &pos)
{
    int sum = 0;
    for (int p = 0; p < 2; ++p)
        for (int i = 0; i < TOTAL_PITS; ++i)
            sum += pos.sides[p].pits[i];
    return sum;
}

// ── Fixture ───────────────────────────────────────────────────────────────

class KalahTest : public ::testing::Test {
protected:
    KalahGame game;

    void SetUp() override { game.initializeBoard(); }
};

// ── A: Initialization ─────────────────────────────────────────────────────

TEST_F(KalahTest, Init_AllPitsAreSix)
{
    const Position &pos = game.getPosition();
    for (int i = 0; i < PITS_PER_SIDE; ++i) {
        EXPECT_EQ(pos[Player::USER][i], INITIAL_STONES) << "USER pit " << i;
        EXPECT_EQ(pos[Player::JINN][i], INITIAL_STONES) << "JINN pit " << i;
    }
}

TEST_F(KalahTest, Init_KalahsAreZero)
{
    const Position &pos = game.getPosition();
    EXPECT_EQ(pos[Player::USER].getKalah(), 0);
    EXPECT_EQ(pos[Player::JINN].getKalah(), 0);
}

TEST_F(KalahTest, Init_TotalStonesIs72)
{
    EXPECT_EQ(totalStones(game.getPosition()), 72);
}

TEST_F(KalahTest, Init_CurrentPlayerIsUser)
{
    EXPECT_EQ(game.getPosition().currentPlayer, Player::USER);
}

TEST_F(KalahTest, Init_CanResetDirtyBoard)
{
    game.makeMove(Player::USER, 0);
    game.initializeBoard();
    const Position &pos = game.getPosition();
    for (int i = 0; i < PITS_PER_SIDE; ++i) {
        EXPECT_EQ(pos[Player::USER][i], INITIAL_STONES);
        EXPECT_EQ(pos[Player::JINN][i], INITIAL_STONES);
    }
    EXPECT_EQ(pos[Player::USER].getKalah(), 0);
    EXPECT_EQ(pos[Player::JINN].getKalah(), 0);
    EXPECT_EQ(totalStones(pos), 72);
}

// ── B: Sowing Mechanics ───────────────────────────────────────────────────

// 1 stone from pit 0 → lands in pit 1
TEST_F(KalahTest, Sow_OneStone_LandsInNextPit)
{
    setBoard(game, { 1, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0 }, 0);
    MoveResult r = game.makeMove(Player::USER, 0);
    EXPECT_EQ(r, MoveResult::SWITCH_TURN);
    const Position &pos = game.getPosition();
    EXPECT_EQ(pos[Player::USER][0], 0);
    EXPECT_EQ(pos[Player::USER][1], 1);
}

// 6 stones from pit 1 on default board: lands in pit2..5, kalah, JINN[0] → SWITCH_TURN
TEST_F(KalahTest, Sow_DefaultBoard_Pit1_SwitchTurn)
{
    MoveResult r = game.makeMove(Player::USER, 1);
    EXPECT_EQ(r, MoveResult::SWITCH_TURN);
    const Position &pos = game.getPosition();
    EXPECT_EQ(pos[Player::USER][1], 0);
    EXPECT_EQ(pos[Player::USER][2], 7);
    EXPECT_EQ(pos[Player::USER][5], 7);
    EXPECT_EQ(pos[Player::USER].getKalah(), 1);
    EXPECT_EQ(pos[Player::JINN][0], 7);
    EXPECT_EQ(totalStones(pos), 72);
}

// 7 stones from pit 0 → wraps onto JINN side; USER kalah gets 1, JINN[0] gets 1
TEST_F(KalahTest, Sow_SevenStones_WrapsToJinnSide)
{
    setBoard(game, { 7, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0 }, 0);
    MoveResult r = game.makeMove(Player::USER, 0);
    EXPECT_EQ(r, MoveResult::SWITCH_TURN);
    const Position &pos = game.getPosition();
    EXPECT_EQ(pos[Player::USER][0], 0);
    for (int i = 1; i < PITS_PER_SIDE; ++i)
        EXPECT_EQ(pos[Player::USER][i], 1) << "USER pit " << i;
    EXPECT_EQ(pos[Player::USER].getKalah(), 1);
    EXPECT_EQ(pos[Player::JINN][0], 1);
    EXPECT_EQ(pos[Player::JINN].getKalah(), 0);
    EXPECT_EQ(totalStones(pos), 7);
}

// 12 stones from pit 0 traverses all of JINN side; JINN kalah must stay 0
TEST_F(KalahTest, Sow_TwelveStones_SkipsJinnKalah)
{
    setBoard(game, { 12, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0 }, 0);
    game.makeMove(Player::USER, 0);
    const Position &pos = game.getPosition();
    EXPECT_EQ(pos[Player::USER].getKalah(), 1);
    for (int i = 0; i < PITS_PER_SIDE; ++i)
        EXPECT_EQ(pos[Player::JINN][i], 1) << "JINN pit " << i;
    EXPECT_EQ(pos[Player::JINN].getKalah(), 0);
}

// Sowing from JINN side skips USER's kalah
TEST_F(KalahTest, Sow_JinnSide_SkipsUserKalah)
{
    setBoard(game, { 0, 0, 0, 0, 0, 0 }, 0, { 8, 0, 0, 0, 0, 0 }, 0, Player::JINN);
    game.makeMove(Player::JINN, 0);
    const Position &pos = game.getPosition();
    EXPECT_EQ(pos[Player::USER].getKalah(), 0);
    EXPECT_EQ(pos[Player::JINN].getKalah(), 1);
}

// Stone count invariant maintained over alternating moves
TEST_F(KalahTest, Sow_StoneInvariantAfterMoves)
{
    struct Move {
        Player player;
        int pit;
    };
    std::vector<Move> moves = {
        { Player::USER, 1 }, { Player::JINN, 1 }, { Player::USER, 2 },
        { Player::JINN, 2 }, { Player::USER, 3 }, { Player::JINN, 3 },
    };
    for (auto &m : moves) {
        MoveResult r = game.makeMove(m.player, m.pit);
        if (r != MoveResult::INVALID)
            EXPECT_EQ(totalStones(game.getPosition()), 72) << "after move pit " << m.pit;
    }
}

// ── C: Extra Turn Rule ────────────────────────────────────────────────────

// 6 stones from pit 0 → last stone lands in USER kalah
TEST_F(KalahTest, ExtraTurn_Pit0With6Stones)
{
    setBoard(game, { 6, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0 }, 0);
    EXPECT_EQ(game.makeMove(Player::USER, 0), MoveResult::EXTRA_TURN);
    EXPECT_EQ(game.getPosition()[Player::USER].getKalah(), 1);
}

// 1 stone from pit 5 → lands directly in USER kalah
TEST_F(KalahTest, ExtraTurn_Pit5With1Stone)
{
    setBoard(game, { 0, 0, 0, 0, 0, 1 }, 0, { 0, 0, 0, 0, 0, 0 }, 0);
    EXPECT_EQ(game.makeMove(Player::USER, 5), MoveResult::EXTRA_TURN);
    EXPECT_EQ(game.getPosition()[Player::USER].getKalah(), 1);
}

// Extra turn must not advance currentPlayer
TEST_F(KalahTest, ExtraTurn_DoesNotChangeCurrentPlayer)
{
    setBoard(game, { 0, 0, 0, 0, 0, 1 }, 0, { 0, 0, 0, 0, 0, 0 }, 0);
    game.makeMove(Player::USER, 5);
    EXPECT_EQ(game.getPosition().currentPlayer, Player::USER);
}

// 1 stone from pit 0 lands in pit 1 — not extra turn
TEST_F(KalahTest, ExtraTurn_NotGranted_WhenNotLandingInKalah)
{
    setBoard(game, { 1, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0 }, 0);
    EXPECT_EQ(game.makeMove(Player::USER, 0), MoveResult::SWITCH_TURN);
}

// JINN also gets extra turn when last stone lands in JINN kalah
TEST_F(KalahTest, ExtraTurn_JinnSide_Pit5With1Stone)
{
    setBoard(game, { 0, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 1 }, 0, Player::JINN);
    EXPECT_EQ(game.makeMove(Player::JINN, 5), MoveResult::EXTRA_TURN);
    EXPECT_EQ(game.getPosition().currentPlayer, Player::JINN);
}

// ── D: Capture Rule ───────────────────────────────────────────────────────

// USER pit 4 has 1 stone → lands in pit 5 (empty); oppositePit = 5-5 = 0; JINN[0]=9 captured
TEST_F(KalahTest, Capture_Basic)
{
    setBoard(game, { 0, 0, 0, 0, 1, 0 }, 0, { 9, 0, 0, 0, 0, 0 }, 0);
    MoveResult r = game.makeMove(Player::USER, 4);
    EXPECT_EQ(r, MoveResult::SWITCH_TURN);
    const Position &pos = game.getPosition();
    EXPECT_EQ(pos[Player::USER].getKalah(), 10); // 9 captured + 1 landing stone
    EXPECT_EQ(pos[Player::USER][5], 0);          // landing pit cleared by capture
    EXPECT_EQ(pos[Player::JINN][0], 0);          // opposite pit cleared
    EXPECT_EQ(totalStones(pos), 10);
}

// Landing pit already has stones → position[p][landing] != 1 → no capture
TEST_F(KalahTest, Capture_LandingPitNonEmpty_NoCapture)
{
    setBoard(game, { 1, 3, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 5, 0 }, 0);
    game.makeMove(Player::USER, 0); // 1 stone → lands USER[1] which had 3 (now 4)
    const Position &pos = game.getPosition();
    EXPECT_EQ(pos[Player::USER].getKalah(), 0);
    EXPECT_EQ(pos[Player::JINN][4], 5); // unchanged
}

// Landing pit empty but opposite JINN pit also empty → no capture
TEST_F(KalahTest, Capture_OppositePitEmpty_NoCapture)
{
    setBoard(game, { 1, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0 }, 0);
    game.makeMove(Player::USER, 0);
    EXPECT_EQ(game.getPosition()[Player::USER].getKalah(), 0);
}

// Verify oppositePit = PITS_PER_SIDE-1-landingPit for each landing pit 1..5
TEST_F(KalahTest, Capture_AllOppositePitPairs)
{
    for (int p = 1; p <= 5; ++p) {
        game.initializeBoard();
        Position &pos = game.getPosition();
        for (int i = 0; i < PITS_PER_SIDE; ++i) {
            pos[Player::USER][i] = 0;
            pos[Player::JINN][i] = 0;
        }
        pos[Player::USER].setKalah(0);
        pos[Player::JINN].setKalah(0);
        pos.currentPlayer = Player::USER;

        pos[Player::USER][p - 1] = 1;                     // 1 stone → lands in USER[p]
        int opp                  = PITS_PER_SIDE - 1 - p; // expected opposite JINN pit
        pos[Player::JINN][opp]   = 7;

        game.makeMove(Player::USER, p - 1);

        EXPECT_EQ(pos[Player::USER][p], 0) << "landing pit cleared, p=" << p;
        EXPECT_EQ(pos[Player::JINN][opp], 0) << "opposite pit cleared, p=" << p;
        EXPECT_EQ(pos[Player::USER].getKalah(), 8) << "kalah = 7+1, p=" << p;
    }
}

// Last stone wraps onto JINN side → currentP != player → no capture
TEST_F(KalahTest, Capture_OnOpponentSide_NoCapture)
{
    setBoard(game, { 8, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 0, 0 }, 0);
    game.makeMove(Player::USER, 0);
    EXPECT_EQ(game.getPosition()[Player::JINN].getKalah(), 0);
}

// JINN captures from USER side (mirror of Capture_Basic)
TEST_F(KalahTest, Capture_JinnCaptures)
{
    setBoard(game, { 9, 0, 0, 0, 0, 0 }, 0, { 0, 0, 0, 0, 1, 0 }, 0, Player::JINN);
    MoveResult r = game.makeMove(Player::JINN, 4);
    EXPECT_EQ(r, MoveResult::SWITCH_TURN);
    const Position &pos = game.getPosition();
    EXPECT_EQ(pos[Player::JINN].getKalah(), 10);
    EXPECT_EQ(pos[Player::JINN][5], 0);
    EXPECT_EQ(pos[Player::USER][0], 0);
}

// Total stones invariant holds after capture
TEST_F(KalahTest, Capture_StoneInvariantPreserved)
{
    const int startTotal = 1 + 9;
    setBoard(game, { 0, 0, 0, 0, 1, 0 }, 0, { 9, 0, 0, 0, 0, 0 }, 0);
    game.makeMove(Player::USER, 4);
    EXPECT_EQ(totalStones(game.getPosition()), startTotal);
}

// ── E: Invalid Moves ──────────────────────────────────────────────────────

TEST_F(KalahTest, Invalid_NegativeIndex)
{
    EXPECT_EQ(game.makeMove(Player::USER, -1), MoveResult::INVALID);
}

TEST_F(KalahTest, Invalid_IndexAtOrAboveBoundary)
{
    EXPECT_EQ(game.makeMove(Player::USER, PITS_PER_SIDE), MoveResult::INVALID);
    EXPECT_EQ(game.makeMove(Player::USER, 100), MoveResult::INVALID);
}

TEST_F(KalahTest, Invalid_EmptyPit)
{
    setBoard(game, { 0, 6, 6, 6, 6, 6 }, 0, { 6, 6, 6, 6, 6, 6 }, 0);
    EXPECT_EQ(game.makeMove(Player::USER, 0), MoveResult::INVALID);
    EXPECT_EQ(game.getPosition()[Player::USER][0], 0);
}

TEST_F(KalahTest, Invalid_BoardUnchanged)
{
    // Record board state, attempt invalid, verify nothing moved
    const Position snapshot = game.getPosition();
    game.makeMove(Player::USER, -1);
    const Position &after = game.getPosition();
    for (int p = 0; p < 2; ++p)
        for (int i = 0; i < TOTAL_PITS; ++i)
            EXPECT_EQ(snapshot.sides[p].pits[i], after.sides[p].pits[i])
                << "side " << p << " pit " << i;
}

// ── F: Game Over & collectRemaining ──────────────────────────────────────

TEST_F(KalahTest, GameOver_FreshBoard_False)
{
    EXPECT_FALSE(game.getPosition().isGameOver());
}

TEST_F(KalahTest, GameOver_UserSideEmpty_True)
{
    setBoard(game, { 0, 0, 0, 0, 0, 0 }, 10, { 6, 6, 6, 6, 6, 6 }, 0);
    EXPECT_TRUE(game.getPosition().isGameOver());
}

TEST_F(KalahTest, GameOver_JinnSideEmpty_True)
{
    setBoard(game, { 6, 6, 6, 6, 6, 6 }, 0, { 0, 0, 0, 0, 0, 0 }, 10);
    EXPECT_TRUE(game.getPosition().isGameOver());
}

TEST_F(KalahTest, GameOver_BothRegularSidesEmpty_True)
{
    setBoard(game, { 0, 0, 0, 0, 0, 0 }, 36, { 0, 0, 0, 0, 0, 0 }, 36);
    EXPECT_TRUE(game.getPosition().isGameOver());
}

TEST_F(KalahTest, GameOver_OneStoneEachSide_False)
{
    setBoard(game, { 1, 0, 0, 0, 0, 0 }, 0, { 1, 0, 0, 0, 0, 0 }, 0);
    EXPECT_FALSE(game.getPosition().isGameOver());
}

TEST_F(KalahTest, CollectRemaining_SweepsToKalahs)
{
    setBoard(game, { 3, 2, 1, 4, 0, 5 }, 7, { 6, 0, 2, 0, 1, 0 }, 3);
    game.getPosition().collectRemaining();
    const Position &pos = game.getPosition();
    EXPECT_EQ(pos[Player::USER].getKalah(), 22); // 7 + (3+2+1+4+0+5)
    EXPECT_EQ(pos[Player::JINN].getKalah(), 12); // 3 + (6+0+2+0+1+0)
    for (int i = 0; i < PITS_PER_SIDE; ++i) {
        EXPECT_EQ(pos[Player::USER][i], 0) << "USER pit " << i;
        EXPECT_EQ(pos[Player::JINN][i], 0) << "JINN pit " << i;
    }
}

TEST_F(KalahTest, CollectRemaining_StoneInvariant)
{
    const int startTotal = 7 + (3 + 2 + 1 + 4 + 0 + 5) + 3 + (6 + 0 + 2 + 0 + 1 + 0);
    setBoard(game, { 3, 2, 1, 4, 0, 5 }, 7, { 6, 0, 2, 0, 1, 0 }, 3);
    game.getPosition().collectRemaining();
    EXPECT_EQ(totalStones(game.getPosition()), startTotal);
}

TEST_F(KalahTest, CollectRemaining_ThenGameOver)
{
    setBoard(game, { 3, 2, 1, 4, 0, 5 }, 7, { 6, 0, 2, 0, 1, 0 }, 3);
    game.getPosition().collectRemaining();
    EXPECT_TRUE(game.getPosition().isGameOver());
}

// ── G: Stone Count Invariant ──────────────────────────────────────────────

TEST_F(KalahTest, StoneInvariant_AfterInit)
{
    EXPECT_EQ(totalStones(game.getPosition()), 72);
}

TEST_F(KalahTest, StoneInvariant_AfterCapture)
{
    const int startTotal = 1 + 9;
    setBoard(game, { 0, 0, 0, 0, 1, 0 }, 0, { 9, 0, 0, 0, 0, 0 }, 0);
    game.makeMove(Player::USER, 4);
    EXPECT_EQ(totalStones(game.getPosition()), startTotal);
}

TEST_F(KalahTest, StoneInvariant_AfterCollectRemaining)
{
    const int startTotal = 7 + (3 + 2 + 1 + 4 + 0 + 5) + 3 + (6 + 0 + 2 + 0 + 1 + 0);
    setBoard(game, { 3, 2, 1, 4, 0, 5 }, 7, { 6, 0, 2, 0, 1, 0 }, 3);
    game.getPosition().collectRemaining();
    EXPECT_EQ(totalStones(game.getPosition()), startTotal);
}

// ── H: AI — selectAIMove() ────────────────────────────────────────────────

TEST_F(KalahTest, AI_FreshBoard_ValidIndex)
{
    int pit = game.selectAIMove();
    EXPECT_GE(pit, 0);
    EXPECT_LT(pit, PITS_PER_SIDE);
    EXPECT_GT(game.getPosition()[Player::JINN][pit], 0);
}

// Only pit 5 is non-empty for JINN — must pick it
TEST_F(KalahTest, AI_OneValidMove_ReturnsThatPit)
{
    setBoard(game, { 6, 6, 6, 6, 6, 6 }, 0, { 0, 0, 0, 0, 0, 6 }, 0, Player::JINN);
    EXPECT_EQ(game.selectAIMove(), 5);
}

// No JINN moves available → returns -1
TEST_F(KalahTest, AI_NoMoves_ReturnsMinus1)
{
    setBoard(game, { 0, 0, 0, 0, 0, 0 }, 36, { 0, 0, 0, 0, 0, 0 }, 36);
    EXPECT_EQ(game.selectAIMove(), -1);
}

// AI must not leave any permanent side-effects on the position
TEST_F(KalahTest, AI_DoesNotMutatePosition)
{
    const Position before = game.getPosition();
    game.selectAIMove();
    const Position &after = game.getPosition();
    for (int p = 0; p < 2; ++p)
        for (int i = 0; i < TOTAL_PITS; ++i)
            EXPECT_EQ(before.sides[p].pits[i], after.sides[p].pits[i])
                << "side " << p << " pit " << i;
}

// All difficulty levels must return a valid pit on a default board
TEST_F(KalahTest, AI_AllLevels_ValidIndex)
{
    for (auto lvl : { Level::NOVICE, Level::CANDIDATE, Level::PARTICIPANT, Level::MASTER }) {
        game.initializeBoard();
        game.setLevel(lvl);
        int pit = game.selectAIMove();
        EXPECT_GE(pit, 0) << "Level=" << static_cast<int>(lvl);
        EXPECT_LT(pit, PITS_PER_SIDE) << "Level=" << static_cast<int>(lvl);
    }
}

// Stone total must not change after AI evaluation
TEST_F(KalahTest, AI_StoneInvariantPreserved)
{
    game.selectAIMove();
    EXPECT_EQ(totalStones(game.getPosition()), 72);
}

// ── I: Settings ───────────────────────────────────────────────────────────

TEST_F(KalahTest, Settings_Level)
{
    game.setLevel(Level::MASTER);
    EXPECT_EQ(game.getLevel(), Level::MASTER);
}

TEST_F(KalahTest, Settings_UserName)
{
    game.setUserName("Alice");
    EXPECT_EQ(game.getUserName(), "Alice");
}

TEST_F(KalahTest, Settings_Gender)
{
    game.setGender(Gender::FEMALE);
    EXPECT_EQ(game.getGender(), Gender::FEMALE);
}

// ── J: Stats Tracking ────────────────────────────────────────────────────

TEST_F(KalahTest, Stats_JinnWin)
{
    game.updateStats(Player::JINN);
    const GameStats &s = game.getStats();
    EXPECT_EQ(s.gamesPlayed, 1);
    EXPECT_EQ(s.jinnWins, 1);
    EXPECT_EQ(s.userWins, 0);
    EXPECT_EQ(s.draws, 0);
}

TEST_F(KalahTest, Stats_UserWin)
{
    game.updateStats(Player::USER);
    const GameStats &s = game.getStats();
    EXPECT_EQ(s.gamesPlayed, 1);
    EXPECT_EQ(s.userWins, 1);
    EXPECT_EQ(s.jinnWins, 0);
}

// The else branch of updateStats increments draws (documents the quirk)
TEST_F(KalahTest, Stats_Draw)
{
    game.updateStats(static_cast<Player>(99));
    EXPECT_EQ(game.getStats().draws, 1);
}

TEST_F(KalahTest, Stats_Accumulates)
{
    game.updateStats(Player::JINN);
    game.updateStats(Player::JINN);
    game.updateStats(Player::JINN);
    const GameStats &s = game.getStats();
    EXPECT_EQ(s.gamesPlayed, 3);
    EXPECT_EQ(s.jinnWins, 3);
}

// Stats survive a board reset
TEST_F(KalahTest, Stats_PersistedAcrossInitBoard)
{
    game.updateStats(Player::USER);
    game.initializeBoard();
    EXPECT_EQ(game.getStats().userWins, 1);
}
