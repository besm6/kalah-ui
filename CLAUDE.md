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

The binary is written to `build/bin/kalah`. Requires gtkmm-4.0, sigc++-3.0, glibmm-2.68, and CMake 3.16+.

## Architecture

The project is a C++17/gtkmm 4 application implementing Kalah-style mancala, structured in two clean layers:

```text
gtkmm screens  ──events──▶  GameController  ──calls──▶  KalahGame
               ◀─signals──  (src/gamecontroller.h/cpp)   (src/kalah.h/cpp)
```

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

### Controller bridge (`src/gamecontroller.h`, `src/gamecontroller.cpp`)

`GameController` — the only framework-facing logic object. Owns a `KalahGame` and manages:

- **App state machine** as enum `AppState`:
  `Welcome(0)` → `EnterName(1)` → `SelectGender(2)` → `SelectDifficulty(3)` → `Playing(4)`
- **Board state** as a flat `std::array<int,14> pits()`:
  - `pits[0..5]` = USER pits, `pits[6]` = USER kalah
  - `pits[7..12]` = JINN pits, `pits[13]` = JINN kalah
- **AI turn timing**: after user moves, a 700 ms `Glib::signal_timeout()` fires before the AI plays; `aiThinking()` reflects this
- Public methods: `proceedFromWelcome()`, `submitName(string)`, `selectGender(int)`, `selectLevel(int)`, `sow(int pitIndex)`, `newGame()`
- sigc++ signals: `signal_app_state_changed`, `signal_board_changed`, `signal_current_player_changed`, `signal_game_over_changed`, `signal_ai_thinking_changed`, `signal_illegal_move`

Player mapping: `currentPlayer()==0` → USER (bottom row), `currentPlayer()==1` → JINN (top row).

### Entry point (`src/main.cpp`)

Creates `Gtk::Application` and uses `make_window_and_run<MainWindow>()` to construct and show `MainWindow`.

### gtkmm frontend (`ui/`)

Window is 800×480 (landscape, non-resizable). `MainWindow` holds a `Gtk::Stack` that switches between 5 screens in response to `signal_app_state_changed`. CSS is embedded as a string in `mainwindow.cpp`.

| File | Purpose |
| ---- | ------- |
| `ui/mainwindow.h/cpp` | `Gtk::ApplicationWindow` + `Gtk::Stack`; owns `GameController` and all 5 screen objects; routes state changes |
| `ui/welcomescreen.h/cpp` | Title + subtitle + "Click anywhere"; `Gtk::GestureClick` → `game.proceedFromWelcome()` |
| `ui/namescreen.h/cpp` | `Gtk::Entry` (max 24) + Continue button; calls `game.submitName()` |
| `ui/genderscreen.h/cpp` | Three `Gtk::Button` (Male/Female/Prefer not to say); calls `game.selectGender()` |
| `ui/difficultyscreen.h/cpp` | Four `Gtk::Button` (Юноша/Кандидат/Участник/Эфенди) with subtitles; calls `game.selectLevel()` |
| `ui/gamescreen.h/cpp` | Score bar + status label + board row + New Game button |
| `ui/pitwidget.h/cpp` | `Gtk::DrawingArea` 90×90; Cairo circle; highlights when playable; emits `signal_tapped` |
| `ui/storewidget.h/cpp` | `Gtk::DrawingArea` 80×220; Cairo pill shape; display-only kalah |

**Data flow:** gtkmm click → `game.sow(index)` → C++ updates `KalahGame` → emits `signal_board_changed` → `GameScreen::refresh_board()` reads `game.pits()` and redraws widgets. AI move follows the same path after the GLib timer fires.

## Implementation Reference

[src/Kalah.md](src/Kalah.md) contains full documentation of the game engine: class design, AI algorithm, evaluation weights, and rule details.

## Rules Implemented

Standard Kalah rules: counter-clockwise sowing, skip opponent's kalah, capture of opposite pit when last stone lands in an empty own pit, free turn when last stone lands in own kalah, end-of-game sweep of remaining stones into respective kalahs.
