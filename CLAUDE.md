# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and Run

A top-level `Makefile` wraps CMake. Use it instead of invoking CMake directly:

| Target | What it does |
| ------ | ------------ |
| `make` / `make all` | Create `build/` if needed, configure with `RelWithDebInfo`, then build everything |
| `make debug` | Create `build/` and configure with `Debug` (configure only — run `make` afterward to build) |
| `make test` | Build `unit_tests` and `controller_tests` targets, then run them via `ctest --output-on-failure` |
| `make install` | Build then install to `/usr/local` |
| `make clean` | Delete `build/` entirely |

The binary is written to `build/bin/kalah`. Requires Qt 6.2+ and CMake 3.16+. CMake handles MOC, RCC, and QML compilation automatically via `qt_add_executable` / `qt_add_qml_module`.

## Architecture

The project is a C++/QML Qt application implementing Kalah-style mancala, structured in two clean layers:

```text
QML screens  ──events──▶  GameController : QObject  ──calls──▶  KalahGame
             ◀─signals──  (src/gamecontroller.h/cpp)             (src/kalah.h/cpp)
```

### Game engine (`src/kalah.h`, `src/kalah.cpp`)

Pure C++17, zero Qt dependencies. Key types:

- `KalahGame` — owns board state and AI; main entry points:
  - `initializeBoard()` — reset to 6 stones per pit
  - `makeMove(Player, pitIndex)` → `MoveResult` (`INVALID` / `SWITCH_TURN` / `EXTRA_TURN`)
  - `selectAIMove()` → pit index chosen by minimax with alpha-beta pruning
  - `setLevel(Level)` — `NOVICE` / `CANDIDATE` / `PARTICIPANT` / `MASTER` (controls search depth and eval weights)
  - `setUserName()`, `setGender()`
- `Position` — board state: `sides[2]` (indexed by `Player::JINN=0`, `Player::USER=1`), each an `OneSide` of 7 slots (pits 0–5 + kalah at index 6)
- `isGameOver()` — true when either side has no stones in regular pits
- `collectRemaining()` — sweeps remaining pit stones into each player's kalah at end of game

Board constants: `PITS_PER_SIDE=6`, `KALAH_INDEX=6`, `INITIAL_STONES=6`.

### Qt bridge (`src/gamecontroller.h`, `src/gamecontroller.cpp`)

`GameController : QObject` — the only Qt/QML-facing object. Owns a `KalahGame` and manages:

- **App state machine** exposed as `Q_PROPERTY(int appState)` with enum `AppState`:
  `Welcome(0)` → `EnterName(1)` → `SelectGender(2)` → `SelectDifficulty(3)` → `Playing(4)`
- **Board state** as a flat 14-element `QVariantList pits`:
  - `pits[0..5]` = USER pits, `pits[6]` = USER kalah
  - `pits[7..12]` = JINN pits, `pits[13]` = JINN kalah
- **AI turn timing**: after user moves, a 700 ms `QTimer` fires before the AI plays; `aiThinking` property reflects this
- Public slots: `proceedFromWelcome()`, `submitName(QString)`, `selectGender(int)`, `selectLevel(int)`, `sow(int pitIndex)`, `newGame()`
- Signals: `appStateChanged`, `boardChanged`, `currentPlayerChanged`, `gameOverChanged`, `aiThinkingChanged`, `illegalMove(int)`

Player mapping: `currentPlayer==0` → USER (bottom row), `currentPlayer==1` → JINN (top row).

### Entry point (`src/main.cpp`)

Instantiates `GameController`, injects it as the `game` context property, loads `qrc:/qt/qml/Kalah/qml/Main.qml`.

### QML frontend

Window is 800×480 (landscape palmtop). `Main.qml` is a thin `StackView` shell; screens are pushed/replaced in response to `game.appStateChanged`:

| File | Purpose |
| ---- | ------- |
| `qml/Main.qml` | `ApplicationWindow` + `StackView`; routes `appState` changes to screen components |
| `qml/WelcomeScreen.qml` | Splash — title, subtitle, pulsing "tap to begin"; calls `game.proceedFromWelcome()` |
| `qml/NameScreen.qml` | `TextField` for player name; calls `game.submitName()` |
| `qml/GenderScreen.qml` | Three touch buttons (Male/Female/Prefer not to say); calls `game.selectGender()` |
| `qml/DifficultyScreen.qml` | Four touch buttons (Юноша/Кандидат/Участник/Эфенди); calls `game.selectLevel()` |
| `qml/GameScreen.qml` | Board + score bar + status label + New Game button |
| `qml/Pit.qml` | Circular pit component; highlights when playable |
| `qml/Store.qml` | Tall oval kalah/store component (display only) |

**Data flow:** QML tap → `game.sow(index)` → C++ updates `KalahGame` → emits `boardChanged` → QML re-reads `game.pits` and redraws. AI move follows the same path after the timer fires.

## Implementation Reference

[src/Kalah.md](src/Kalah.md) contains full documentation of the game engine: class design, AI algorithm, evaluation weights, and rule details.

## Rules Implemented

Standard Kalah rules: counter-clockwise sowing, skip opponent's kalah, capture of opposite pit when last stone lands in an empty own pit, free turn when last stone lands in own kalah, end-of-game sweep of remaining stones into respective kalahs.
