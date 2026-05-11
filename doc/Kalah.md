# Kalah — C++ Implementation Details

## Origins

This C++ program is a modernization of a Kalah game originally written in a BESM-6 Pascal dialect, deployed on a Soviet mainframe ca. 1975–1985. The original (`kalah.pas`, 2,755 lines) ran in a multi-user shared environment, used Cyrillic text throughout, and had a rich AI opponent named Джин ("Djinn"). The C++ recreation preserves the core rules and AI algorithms while eliminating platform dependencies. The result is ~1,615 lines of C++17 across three translation units.

---

## Game Rules

Kalah is a two-player Mancala variant. The board has:

- 6 pits per player (initially 6 stones each, 72 stones total)
- 1 Kalah (store) per player, starting empty
- Both players compete to accumulate the most stones in their Kalah

**A turn:**
1. Pick any non-empty pit on your side (1–6).
2. Lift all stones from that pit and distribute them one at a time counter-clockwise, dropping into each pit and your own Kalah.
3. **Skip the opponent's Kalah** when passing over it.
4. **Extra turn:** If the last stone lands in your own Kalah, you move again.
5. **Capture:** If the last stone lands in an empty pit on your own side and the directly opposite pit on the opponent's side is non-empty, capture all stones from both pits into your Kalah.

**Game end:** When all pits on either side are empty, the game is over. The player whose side still has stones collects them all into their own Kalah. Most stones in Kalah wins. A draw is possible.

**Winning threshold constant:** `WINNING_SCORE = 37` (majority of 72 total stones).

---

## Session Flow

The application is a Qt/QML GUI. `GameController : QObject` owns the `KalahGame` and drives both the setup screens and the game loop via signals and slots.

### App State Machine

```
Welcome(0) → EnterName(1) → SelectGender(2) → SelectDifficulty(3) → Playing(4)
```

Exposed as `Q_PROPERTY(int appState)` with the `AppState` enum.

### Setup Slot Chain

```
proceedFromWelcome()   → appState = EnterName
submitName(QString)    → KalahGame::setUserName(); appState = SelectGender
selectGender(int)      → KalahGame::setGender(); appState = SelectDifficulty
selectLevel(int)       → KalahGame::setLevel(); initializeBoard(); appState = Playing
                          emits boardChanged, currentPlayerChanged, gameOverChanged
```

### User Move Path

```
sow(pitIndex)
  └─ makeMove(USER, pit)
       └─ afterMove(result)
            ├─ emit boardChanged
            ├─ isGameOver? → collectRemaining(); emit boardChanged, gameOverChanged
            ├─ EXTRA_TURN  → return  (same player moves again)
            └─ SWITCH_TURN → emit currentPlayerChanged
                              JINN's turn? → scheduleAIMove()
```

### AI Move Path

```
scheduleAIMove()
  └─ m_aiThinking = true; emit aiThinkingChanged
     QTimer::start(700 ms)
          └─ doAIMove()
               ├─ selectAIMove() → pit index
               ├─ makeMove(JINN, pit)
               ├─ EXTRA_TURN  → QTimer::start(700 ms)  (JINN moves again)
               └─ SWITCH_TURN → m_aiThinking = false; emit aiThinkingChanged, currentPlayerChanged
```

### New Game

`newGame()` stops the AI timer, resets `m_aiThinking`, calls `initializeBoard()`, and emits `boardChanged`, `currentPlayerChanged`, `gameOverChanged`. It transitions directly to the `Playing` state without going through setup screens again.

---

## Data Structures

### Enumerations

```cpp
enum class Player   { JINN = 0, USER = 1 };
enum class MoveResult { INVALID = 0, SWITCH_TURN = 1, EXTRA_TURN = 2 };
enum class Level    { NOVICE = 1, CANDIDATE = 2, PARTICIPANT = 3, MASTER = 4 };
enum class Gender   { UNKNOWN = 0, MALE = 1, FEMALE = 2 };
```

Level names map to the original Russian: Юноша (Youth), Кандидат, Участник, Эфенди.

### Constants

```cpp
constexpr int PITS_PER_SIDE  = 6;
constexpr int KALAH_INDEX    = 6;   // index into OneSide::pits[]
constexpr int TOTAL_PITS     = 7;   // 6 pits + 1 Kalah per side
constexpr int INITIAL_STONES = 6;
constexpr int WINNING_SCORE  = 37;
```

### OneSide

```cpp
struct OneSide {
    std::array<int, 7> pits;   // pits[0..5] = regular pits; pits[6] = Kalah
    int getKalah() const;
    int getTotalInPits() const; // sum of pits[0..5] only
};
```

### Position

```cpp
struct Position {
    std::array<OneSide, 2> sides;  // [0] = JINN, [1] = USER
    Player currentPlayer;          // initialized to USER

    Player opponent(Player p) const;
    bool isGameOver() const;       // either side's getTotalInPits() == 0
    void collectRemaining();       // sweeps all pit stones into each Kalah
};
```

`operator[]` is overloaded for `Player`, so `pos[Player::JINN][3]` accesses Djinn's pit at index 3.

### EvaluationWeights

```cpp
struct EvaluationWeights {
    int kalahWeight;      // multiplier for Kalah stone count
    int extraTurnBonus;   // bonus for pits whose stone count == distance to Kalah
    int emptyPitValue;    // value of an empty pit with opponent stones opposite
    int multiLapBonus;    // stones that would traverse the whole board
    int captureValue;     // opponent can capture from this pit
    int mobilityWeight;   // per-stone mobility bonus
    int distanceWeight;   // per-pit proximity to Kalah
};
```

Values by level:

| Level       | kalah | extraTurn | emptyPit | multiLap | capture | mobility | distance |
|-------------|-------|-----------|----------|----------|---------|----------|----------|
| NOVICE      | 10    | 5         | 2        | 1        | 3       | 1        | 1        |
| CANDIDATE   | 15    | 8         | 3        | 2        | 5       | 2        | 2        |
| PARTICIPANT | 20    | 10        | 5        | 3        | 8       | 3        | 3        |
| MASTER      | 25    | 12        | 6        | 4        | 10      | 4        | 4        |

### GameStats

```cpp
struct GameStats {
    int jinnScore = 0;    // wins for Jinn across this session
    int userScore = 0;
    int gamesPlayed = 0;
    int jinnWins = 0;
    int userWins = 0;
    int draws = 0;
};
```

---

## Move Execution (makeMove)

```cpp
MoveResult KalahGame::makeMove(Player player, int pitIndex, bool animate);
```

1. Validate `pitIndex` ∈ [0,5] and `currentSide[pitIndex] > 0`; return `INVALID` otherwise.
2. Lift all stones: set the chosen pit to 0, `stones = original count`.
3. Walk pits one at a time, counter-clockwise:
   - Advance `currentPit++`; if `currentPit >= TOTAL_PITS` wrap to the next side (`currentP = opponent(currentP)`, `currentPit = 0`).
   - If `currentPit == KALAH_INDEX` and `currentP == opponent`: **skip** (do not drop, do not decrement `stones`).
   - Otherwise drop one stone: `position[currentP][currentPit]++; stones--`.
4. When `stones == 0` (last stone placed):
   - **Extra turn:** if `currentPit == KALAH_INDEX && currentP == player` → return `EXTRA_TURN`.
   - **Capture:** if `currentP == player && currentPit < 6 && position[player][currentPit] == 1`:
     - Compute `oppositePit = PITS_PER_SIDE - 1 - currentPit`.
     - If `position[opponent][oppositePit] > 0`: move captured + landing stone into `position[player][KALAH_INDEX]`; zero both pits.
5. Set `position.currentPlayer = opponent`; increment `movesThisGame`; return `SWITCH_TURN`.

The `animate` parameter is accepted but unused inside `makeMove`; all move timing is handled by the 700 ms `QTimer` in `GameController`.

---

## AI: selectAIMove

```cpp
int KalahGame::selectAIMove();   // returns 0-based pit index, or -1 if no moves
```

**Search depth** by level:
- NOVICE, CANDIDATE → depth 2
- PARTICIPANT, MASTER → depth 4

**Algorithm:**

1. For each valid pit of JINN:
   - Save `position`, call `makeMove(JINN, pit, false)`, capture `newPos`, restore `position`.
   - If result is `EXTRA_TURN`: call `minimax(newPos, depth, ...)` keeping `maximizing = true` (same depth, not depth-1).
   - Otherwise: call `minimax(newPos, depth-1, ..., maximizing = false)`.
   - Store `{pit, score}`.

2. Find `maxScore` over all moves.

3. Build `bestMoves`: all moves with `score >= maxScore - 50` (50-point tolerance).

4. **Randomization at PARTICIPANT/MASTER:** 20% chance to pick a random move from `bestMoves` instead of the first.

5. Return `bestMoves[0].pit` (or random pick).

---

## AI: minimax

```cpp
int KalahGame::minimax(Position& pos, int depth, int alpha, int beta, bool maximizing);
```

Standard alpha-beta minimax. JINN maximizes, USER minimizes.

For each move at the current node:
- Save `position`, assign `position = pos`, call `makeMove`, capture `newPos`, restore `position`.
- If `result == EXTRA_TURN`: recurse with **same depth** and **same maximizing** side (the extra turn continues for the same player).
- Otherwise: recurse with `depth - 1` and flipped `maximizing`.

Base cases: `depth == 0` or `pos.isGameOver()` → call `evaluatePosition(pos)`.

---

## AI: evaluatePosition

```cpp
int KalahGame::evaluatePosition(const Position& pos);
```

Returns `jinnScore - userScore` (positive = good for Jinn).

**Terminal node** (`isGameOver()`): calls `diffScore()`:
- Sum all stones (pits + Kalah) for each player.
- Djinn ahead → +100,000; behind → -100,000; equal → 0.

**Non-terminal**, for each pit `i` (0–5):

For Jinn's pit `i` with `jinnStones > 0`:

- `jinnScore += jinnStones * mobilityWeight`
- `jinnScore += (KALAH_INDEX - distance) * distanceWeight`  where `distance = KALAH_INDEX - i`
- If `jinnStones == distance`: `jinnScore += extraTurnBonus` (this pit would land exactly in Kalah)
- If `jinnStones == PITS_PER_SIDE + 1 + i`: `userScore += captureValue` (Jinn's stones wrap around and threaten user)

For Jinn's pit `i` empty:

- `oppositePit = PITS_PER_SIDE - 1 - i`; if `pos[USER][oppositePit] > 0`: `userScore += emptyPitValue` (user can capture)

Symmetrically for User's pit `i`.

Both Kalahs contribute: `kalah * kalahWeight` to each side's score.

---

## AI: Phrase System

`getPhrase(int moveNumber)` returns a random commentary string, or `""`.

- 70% chance returns `""` (silence).
- Picks from 30 hard-coded English phrases, each used at most once per game (`phrasesUsed` vector reset when all exhausted).
- `aiSilent` flag suppresses all phrases (never set to true in current code).
- RNG: `std::mt19937` seeded with `std::time(nullptr)` at construction.

Phrases include: "Thinking carefully...", "Interesting move!", "The tide is turning.", "Fortune favors the bold.", "May the best player win!" etc.

---

## Multi-Game Sessions

`KalahGame` is constructed once inside `GameController`'s constructor and reused for the entire application session. `initializeBoard()` resets the board and `movesThisGame` counter but not `GameStats`.

`newGame()` in `GameController` stops the AI timer, resets `m_aiThinking`, calls `initializeBoard()`, and emits board/player/gameover signals. It transitions directly to `Playing` without repeating the setup screens.

`updateStats()` exists in `KalahGame` but is not called anywhere in `GameController`, so all session statistics (`gamesPlayed`, `jinnWins`, `userWins`, `draws`) remain at zero throughout a session.

---

## What Was Removed from the Original

The following BESM-6 features were intentionally not ported:

| Feature | Original | C++ |
|---------|----------|-----|
| Opening book database | 8,415-position 6-ary tree, 5 KB zone file, used at levels 3–4 | Absent |
| Corrections/learning system | 48-bit/entry binary database, 15-bit position hash, adjusts moves 1–16 | Absent |
| Multi-user zone persistence | Zones at fixed octal addresses, player records with 210-game history | Absent |
| Game save/resume | Saved position state in player zone record | Absent |
| Time-of-day scheduling | Opening hours enforcement, night mode, CPU quota tracking | Absent |
| Level auto-promotion | Win at К → У → Э; score resets on promotion | Absent |
| Russian/Cyrillic dialogue | 100+ contextual phrases, gender-aware, cultural references | 30 English phrases |
| Administrative commands | 'B' for random test position, special tournament access | Absent |
| Detailed statistics zones | Per-level: node count, entropy, wall-clock/think time | Absent |

### Code Simplification vs. Original

| Metric | Original Pascal | C++ |
|--------|----------------|-----|
| Lines of code | 2,755 | ~1,615 |
| `goto` statements | 89 | 0 |
| Global variables | 50+ | 0 |
| Magic numbers | Many | Few (constants/enums) |

---

## Known Quirks and Notes

- `updateStats()` is never called from `GameController`, so all fields in `GameStats` (`gamesPlayed`, `jinnWins`, `userWins`, `draws`) remain zero for the entire session. Stats tracking is not wired up in the current Qt version.
- `selectAIMove()` saves and restores the game's `position` by value assignment (`Position savedPos = position; ... position = savedPos`). This is correct but means the AI search mutates and restores `KalahGame::position` repeatedly during the search tree traversal.
