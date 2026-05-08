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

## Board Layout

The ncurses display (70 × 15 character window) arranges the board as:

```
        DJINN
     6   5   4   3   2   1
   ( n) ( n) ( n) ( n) ( n) ( n)
[nn]                          [nn]
   ( n) ( n) ( n) ( n) ( n) ( n)
     1   2   3   4   5   6
        <player name>
```

- Djinn's pits are numbered 6–1 left-to-right (pit 6 is at index 0 internally, pit 1 at index 5).
- The user's pits are numbered 1–6 left-to-right matching their 0-based indices.
- Pits display as `( n)` (green highlight when selected); Kalahs display as `[nn]` in magenta.
- Djinn's Kalah is at screen column 3; user's Kalah is at screen column 60.

---

## Build

```
make          # compile to ./kalah  (g++ -std=c++17 -Wall -Wextra -O2 -g -lncurses)
make run      # compile and run
make clean    # remove .o and binary
```

Manual compilation:
```bash
g++ -std=c++17 -Wall -Wextra -O2 -c kalah.cpp -o kalah.o
g++ -std=c++17 -Wall -Wextra -O2 -c ui.cpp -o ui.o
g++ -std=c++17 -Wall -Wextra -O2 -c main.cpp -o main.o
g++ -std=c++17 -Wall -Wextra -O2 -o kalah kalah.o ui.o main.o -lncurses
```

Requires: GCC 7+ or Clang 5+; ncurses development headers (`libncurses5-dev` on Debian/Ubuntu, `ncurses-devel` on Fedora, `brew install ncurses` on macOS).

---

## Session Flow

```
main()
  └─ GameController::run()
       ├─ ui.initialize()       — ncurses init, create 3 windows
       ├─ ui.showWelcome()
       ├─ ui.getUserName()      — stored in KalahGame::userName
       ├─ ui.selectGender()     — stored as Gender enum (MALE/FEMALE/UNKNOWN)
       ├─ ui.selectLevel()      — stored as Level enum
       ├─ ui.initialize()       — reinitialize windows after setup screens
       └─ loop: playGame()  →  confirm("Play another game?")
            └─ showFinalStats()
```

### playGame()

```
game.initializeBoard()
draw board + stats

loop while !position.isGameOver():
    USER turn  → handleUserTurn()
    JINN turn  → handleAITurn()
    flip currentPlayer unless extraTurn

finishGame()   ← collectRemaining(), display score, updateStats()
```

### handleUserTurn()

Loops on input until a valid move is made:
- `'q'` / `'Q'` → confirm quit (sets `gameRunning = false`)
- `'h'` / `'H'` → show help overlay
- `'1'`–`'6'` → convert to 0-based index, validate non-empty pit, call `game.makeMove()`
- Invalid input or empty pit → `displayError()` + `waitForKey()`

### handleAITurn()

1. Display "Djinn is thinking..." and sleep 500 ms.
2. Call `game.selectAIMove()` to get pit index.
3. Optionally fetch a phrase (30% chance) and display it.
4. Call `game.makeMove(Player::JINN, move, true)`.
5. Announce "Djinn plays pit N" (converting 0-based to display number: `PITS_PER_SIDE - move`).
6. Sleep 800 ms.

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

`operator[]` is overloaded for both `Player` and `size_t`, so `pos[Player::JINN][3]` accesses Djinn's pit at index 3.

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
   - Advance `currentPit++`; if `currentPit >= 7` wrap to the next side (`currentP = opponent(currentP)`, `currentPit = 0`).
   - If `currentPit == KALAH_INDEX` and `currentP == opponent`: **skip** (do not drop, do not decrement `stones`).
   - Otherwise drop one stone: `position[currentP][currentPit]++; stones--`.
4. When `stones == 0` (last stone placed):
   - **Extra turn:** if `currentPit == KALAH_INDEX && currentP == player` → return `EXTRA_TURN`.
   - **Capture:** if `currentP == player && currentPit < 6 && position[player][currentPit] == 1`:
     - Compute `oppositePit = 5 - currentPit`.
     - If `position[opponent][oppositePit] > 0`: move captured + landing stone into `position[player][KALAH_INDEX]`; zero both pits.
5. Set `position.currentPlayer = opponent`; increment `movesThisGame`; return `SWITCH_TURN`.

The `animate` parameter is accepted but currently only delays are used in the controller layer; `makeMove` itself doesn't call ncurses.

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
- `jinnScore += (6 - distance) * distanceWeight`  where `distance = 6 - i`
- If `jinnStones == distance`: `jinnScore += extraTurnBonus` (this pit would land exactly in Kalah)
- If `jinnStones == 7 + i`: `userScore += captureValue` (Jinn's stones would wrap around and threaten user)

For Jinn's pit `i` empty:
- `oppositePit = 5 - i`; if `pos[USER][oppositePit] > 0`: `userScore += emptyPitValue` (user can capture)

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

## UI (KalahUI)

### Windows

Three `WINDOW*` objects, stacked vertically, centered on the terminal:

| Window      | Height | Width | Content |
|-------------|--------|-------|---------|
| `statsWin`  | 3      | 70    | Level and session score |
| `boardWin`  | 15     | 70    | Board with pits and Kalahs |
| `messageWin`| 5      | 70    | Messages, prompts, errors |

### Color Pairs

| Pair | Foreground | Background | Used for |
|------|------------|------------|---------|
| 1    | YELLOW     | BLACK      | Djinn labels and messages |
| 2    | CYAN       | BLACK      | User name |
| 3    | GREEN      | BLACK      | Highlights, extra turn, win messages |
| 4    | RED        | BLACK      | Errors |
| 5    | WHITE      | BLUE       | Title bar |
| 6    | MAGENTA    | BLACK      | Kalah displays |

### Input

- `getUserMove()`: reads one character from `messageWin`. Returns 0–5 for `'1'`–`'6'`, -2 for `'q'`/'Q'`, -3 for `'h'`/'H'`, -1 for anything else.
- `getUserInput()`: calls `mvwgetnstr` for free-text entry (name input).
- `confirm()`: blocks on `'y'`/`'Y'`/`'n'`/`'N'`.

### Animation

`animateMove()` currently just calls `napms(300)`, redraws the board, then `napms(200)`. No per-stone animation.

---

## Multi-Game Sessions

The session persists throughout `GameController::run()`. `KalahGame` (and its embedded `GameStats`) is constructed once and reused across games. `initializeBoard()` resets the board and `movesThisGame` counter but not the stats. At session end, `showFinalStats()` prints totals: games played, Djinn wins, user wins, draws.

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

- The `draw` case in `finishGame()` calls `updateStats()` only when `winner != static_cast<Player>(-1)`, so draws do not increment `draws` in `GameStats`. The `draws` field is incremented only if the caller explicitly passes a draw sentinel — which never happens in the current code. Effectively, draws are untracked.
- `ui.initialize()` is called twice in `GameController::run()`: once at startup (correct) and once after level selection (redundant but harmless — it reinitializes windows after the full-screen setup screens).
- `selectAIMove()` saves and restores the game's `position` by value assignment (`Position savedPos = position; ... position = savedPos`). This is correct but means the AI search mutates and restores `KalahGame::position` repeatedly during the search tree traversal.
- Djinn's display pit number is `PITS_PER_SIDE - move` (i.e., `6 - 0-based-index`), because Djinn's pits are drawn right-to-left: display pit 6 is internal index 0, display pit 1 is internal index 5.
