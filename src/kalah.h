#ifndef KALAH_H
#define KALAH_H

#include <array>
#include <memory>
#include <random>
#include <string>
#include <vector>

constexpr int PITS_PER_SIDE  = 6;
constexpr int KALAH_INDEX    = 6;
constexpr int TOTAL_PITS     = 7;
constexpr int INITIAL_STONES = 6;
constexpr int WINNING_SCORE  = 37;

enum class Player { JINN = 0, USER = 1 };

enum class MoveResult { INVALID = 0, SWITCH_TURN = 1, EXTRA_TURN = 2 };

enum class Level {
    NOVICE      = 1, // Юноша (Youth)
    CANDIDATE   = 2, // Кандидат
    PARTICIPANT = 3, // Участник
    MASTER      = 4  // Эфенди
};

enum class Gender { UNKNOWN = 0, MALE = 1, FEMALE = 2 };

struct OneSide {
    std::array<int, TOTAL_PITS> pits;

    // Represents one player's side of the board: six regular pits plus one kalah (scoring pit).
    // All pits start at zero; initializeBoard() fills them with stones before play begins.
    OneSide() { pits.fill(0); }

    // Returns a reference to a pit so its stone count can be read or changed.
    // Index 0–5 are regular pits; index 6 (KALAH_INDEX) is the scoring pit.
    int &operator[](size_t idx) { return pits[idx]; }
    // Read-only version of pit access; used when the board state must not be modified.
    const int &operator[](size_t idx) const { return pits[idx]; }

    // Returns the number of stones currently in this player's kalah (the big scoring pit).
    int getKalah() const { return pits[KALAH_INDEX]; }
    // Directly sets the stone count in the kalah.
    // Used during end-of-game score collection when remaining pit stones are swept in.
    void setKalah(int value) { pits[KALAH_INDEX] = value; }

    // Adds up all stones in the six regular pits, not counting the kalah.
    // Returns zero when the side is completely empty, which signals that the game is over.
    int getTotalInPits() const
    {
        int sum = 0;
        for (int i = 0; i < PITS_PER_SIDE; ++i) {
            sum += pits[i];
        }
        return sum;
    }
};

struct Position {
    std::array<OneSide, 2> sides;
    Player currentPlayer;

    // Stores the complete board: both players' pit counts and whose turn it is.
    // The human player (USER) always moves first at the start of a new game.
    Position() : currentPlayer(Player::USER) {}

    // Looks up one player's OneSide using a Player enum value instead of a raw array index.
    OneSide &operator[](Player p) { return sides[static_cast<int>(p)]; }
    // Read-only version; safe to call when the position must not change.
    const OneSide &operator[](Player p) const { return sides[static_cast<int>(p)]; }

    // Returns the other player. Passing JINN gives USER; passing USER gives JINN.
    Player opponent(Player p) const { return (p == Player::JINN) ? Player::USER : Player::JINN; }

    // Returns true when at least one player has no stones left in their regular pits.
    // Kalah rules say play ends immediately when either side is emptied.
    bool isGameOver() const
    {
        return sides[0].getTotalInPits() == 0 || sides[1].getTotalInPits() == 0;
    }

    // Sweeps every stone still in regular pits into its owner's kalah.
    // Called once at game end so each player scores everything remaining on their side.
    void collectRemaining()
    {
        for (int i = 0; i < PITS_PER_SIDE; ++i) {
            sides[0].pits[KALAH_INDEX] += sides[0].pits[i];
            sides[0].pits[i] = 0;
            sides[1].pits[KALAH_INDEX] += sides[1].pits[i];
            sides[1].pits[i] = 0;
        }
    }
};

struct EvaluationWeights {
    int kalahWeight;    // Weight for stones in kalah
    int extraTurnBonus; // Bonus for landing in kalah
    int emptyPitValue;  // Value of empty pits (capture setup)
    int multiLapBonus;  // Bonus for stones that go multiple laps
    int captureValue;   // Value of capture opportunities
    int mobilityWeight; // Weight for mobile stones
    int distanceWeight; // Weight for distance to kalah
};

struct GameStats {
    int jinnScore   = 0;
    int userScore   = 0;
    int gamesPlayed = 0;
    int jinnWins    = 0;
    int userWins    = 0;
    int draws       = 0;
};

class KalahGame {
public:
    KalahGame(Level level = Level::NOVICE, const std::string &userName = "Player");

    void initializeBoard();
    MoveResult makeMove(Player player, int pitIndex, bool animate = true);
    int selectAIMove();

    // Exposes the current board state so callers can read pit counts,
    // detect whose turn it is, or check whether the game has ended.
    Position &getPosition() { return position; }
    // Read-only version for code that only needs to inspect the board.
    const Position &getPosition() const { return position; }

    // Returns the current AI difficulty setting: NOVICE, CANDIDATE, PARTICIPANT, or MASTER.
    Level getLevel() const { return level; }
    // Changes the AI difficulty. Affects how many moves ahead the AI searches
    // and how it weights strategic factors like captures and extra turns.
    void setLevel(Level l) { level = l; }

    // Returns a read-only snapshot of cumulative results:
    // wins, losses, draws, and total games played.
    const GameStats &getStats() const { return stats; }
    void updateStats(Player winner);

    // Returns the human player's name exactly as they entered it.
    std::string getUserName() const { return userName; }
    // Stores the player's name; shown in the UI and used when composing AI commentary.
    void setUserName(const std::string &name) { userName = name; }

    // Returns the player's chosen gender (UNKNOWN, MALE, or FEMALE).
    Gender getGender() const { return gender; }
    // Records the player's gender selection, which the AI may use
    // when picking commentary phrases.
    void setGender(Gender g) { gender = g; }

    std::string getPhrase(int moveNumber);

private:
    Position position;
    Level level;
    std::string userName;
    Gender gender;
    GameStats stats;
    std::mt19937 rng;

    int movesThisGame;
    int lastJinnKalah;
    int lastUserKalah;
    bool aiSilent;

    EvaluationWeights getWeights() const;
    int evaluatePosition(const Position &pos);
    int minimax(Position &pos, int depth, int alpha, int beta, bool maximizing);
    int diffScore(const Position &pos);

    std::vector<std::string> phrases;
    std::vector<bool> phrasesUsed;
    void initializePhrases();
    int randomInt(int max);
};

#endif // KALAH_H
