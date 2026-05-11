# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and Run

The project is built with Swift Package Manager. A `Makefile` wraps the common commands:

| Target | What it does |
| ------ | ------------ |
| `make` / `make all` | `swift build` (debug) |
| `make release` | `swift build -c release` |
| `make run` | `swift run Kalah` — build and launch the app |
| `make test` | Build and run C++ engine unit tests via CMake/ctest |
| `swift test` | Build and run Swift UI unit tests (KalahTests) |
| `make install` | Copy release binary to `/usr/local/bin/Kalah` |
| `make clean` | Delete `.build/` and `cmake-build/` |

The debug binary is written to `.build/debug/Kalah`. Requires macOS 14+ and Swift 5.10+.

## Architecture

The project has two layers: a pure C++17 game engine and a SwiftUI front-end that calls it via Swift/C++ interop.

```text
SwiftUI views  ──calls──▶  KalahViewModel  ──protocol──▶  KalahBridge  ──C++ interop──▶  KalahSwift  ──▶  KalahGame
               ◀─@Observable─  (ui/)            (ui/)         (ui/)           (src/)       (src/)           (src/)
```

SPM targets: `KalahEngine` (C/C++, `src/`) → `KalahCore` (Swift library, `ui/`) → `Kalah` (executable, `app/`). Tests live in `tests/` as the `KalahTests` target.

### Game engine (`src/kalah.h`, `src/kalah.cpp`)

Pure C++17, zero framework dependencies. Key types:

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

### C++ bridge (`src/KalahSwift.h`)

A header-only C++ wrapper class that Swift imports directly via Swift/C++ interop (`.interoperabilityMode(.Cxx)`). All methods use primitive types (`int`, `bool`, `const char*`) so no C++ types leak into Swift. Also owns the app state machine.

- **App state machine** (int 0–4): `Welcome` → `EnterName` → `SelectGender` → `SelectDifficulty` → `Playing`
  - `proceedFromWelcome()`, `submitName(const char*)`, `selectGender(int)`, `selectLevel(int)`
- **Board access**: `getPitAt(player, index)` — player 0=JINN / 1=USER, index 0–6 (6=kalah)
- **Move actions**: `sow(pit)`, `doAIMove(pit)`, `selectAIMove()` — return int (0=INVALID, 1=SWITCH_TURN, 2=EXTRA_TURN)
- `src/module.modulemap` exposes `KalahSwift.h` with `requires cplusplus`

### SwiftUI front-end (`app/`, `ui/`)

Window is ~720×400. `KalahViewModel` is a `@MainActor @Observable` class that drives all state. Views read from it and call its methods. All Swift source is in the `KalahCore` library target (`ui/`) except the entry point.

| File | Purpose |
| ---- | ------- |
| `app/KalahApp.swift` | `@main` SwiftUI `App`; creates `KalahViewModel` and injects it as an environment object |
| `ui/KalahViewModel.swift` | `@Observable` state; delegates to `KalahProtocol`; schedules AI moves with `Task.sleep(700 ms)` |
| `ui/KalahProtocol.swift` | Protocol abstracting all engine calls; enables mock injection for tests |
| `ui/KalahBridge.swift` | Concrete `KalahProtocol` implementation; owns the `KalahSwift` C++ object |
| `ui/ContentView.swift` | Root view; switches on `vm.appState` with slide transition |
| `ui/WelcomeView.swift` | Title + subtitle + "Tap anywhere to begin"; full-screen tap gesture |
| `ui/NameView.swift` | `TextField` (max 24 chars) + Continue button |
| `ui/GenderView.swift` | Three buttons: Male / Female / Prefer not to say |
| `ui/DifficultyView.swift` | Four buttons: Юноша / Кандидат / Участник / Эфенди |
| `ui/GameView.swift` | Score bar + status label + board grid + New Game button |
| `ui/PitView.swift` | 80×80 circle; highlights when playable; tap calls `vm.sow(index)` |
| `ui/StoreView.swift` | 80×200 pill; display-only kalah score |
| `ui/Styles.swift` | `Color(hex:)` extension, `GoldButtonStyle`, `ChoiceButtonStyle`, shared colour constants |

**Data flow:** SwiftUI tap → `vm.sow(index)` → `engine.sow(pit:)` → `KalahBridge` → `KalahSwift::sow()` updates `KalahGame` → `syncBoard()` reads `engine.getPits()` → `@Observable` triggers view refresh. After a SWITCH_TURN result, `KalahViewModel` waits 700 ms then calls `engine.selectAIMove()` / `engine.doAIMove(pit:)` and syncs again.

Player mapping: `currentPlayer == 0` → USER (bottom row), `currentPlayer == 1` → JINN (top row).

### Swift unit tests (`tests/`)

`KalahTests` target exercises `KalahViewModel` in isolation using a `MockKalahEngine` that implements `KalahProtocol`. The mock records call counts and lets each test configure return values.

- `tests/KalahViewModelTests.swift` — 15 test cases covering state machine transitions, board sync, extra-turn logic, AI scheduling, game-over handling, and new-game reset
- `tests/MockKalahEngine.swift` — configurable mock with per-call result queues

Run with `swift test`. The `aiDelay` parameter on `KalahViewModel.init(engine:aiDelay:)` is set to `.zero` in tests so async AI-turn cases complete in milliseconds.

## Implementation Reference

[src/Kalah.md](src/Kalah.md) contains full documentation of the game engine: class design, AI algorithm, evaluation weights, and rule details.

## Rules Implemented

Standard Kalah rules: counter-clockwise sowing, skip opponent's kalah, capture of opposite pit when last stone lands in an empty own pit, free turn when last stone lands in own kalah, end-of-game sweep of remaining stones into respective kalahs.
