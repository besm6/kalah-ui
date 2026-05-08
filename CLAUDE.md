# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and Run

```bash
mkdir build && cd build
cmake ..
cmake --build .
./mancala
```

Requires Qt 6.2+ and CMake 3.16+. The CMake build handles MOC, RCC, and QML compilation automatically via `qt_add_executable` / `qt_add_qml_module`.

## Architecture

The project is a C++/QML Qt application implementing Kalah-style mancala.

**C++ backend** ([src/mancalagame.h](src/mancalagame.h), [src/mancalagame.cpp](src/mancalagame.cpp)):
- `MancalaGame : QObject` — all game logic; exposes state to QML via `Q_PROPERTY`
- Board is a flat 14-element array: indices 0–5 are Player 0's pits, index 6 is Player 0's store, indices 7–12 are Player 1's pits, index 13 is Player 1's store
- Public slots: `sow(int pitIndex)` (make a move), `reset()` (new game)
- Signals: `boardChanged`, `currentPlayerChanged`, `gameOverChanged`, `illegalMove`, `moveCompleted`

**QML frontend** ([qml/Main.qml](qml/Main.qml), [qml/Pit.qml](qml/Pit.qml), [qml/Store.qml](qml/Store.qml)):
- `Main.qml` — root window (1024×600); renders board and game status; calls `game.sow()` on tap
- `Pit.qml` — circular pit component; highlights when it belongs to the current player
- `Store.qml` — tall oval store/mancala component (display only)

**Wiring** ([src/main.cpp](src/main.cpp)):
- `MancalaGame` is instantiated in C++ and injected into the QML engine as the `game` context property

**Data flow:** QML tap → `game.sow(index)` → C++ updates `m_board` → emits `boardChanged` → QML re-reads `game.pits` and redraws.

## Rules Implemented

Standard Kalah rules: counter-clockwise sowing, capture of opposite pit when landing in an empty own pit, free turn when landing in own store, end-of-game sweep of remaining stones to the respective stores.
