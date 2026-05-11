#pragma once
#if __has_include(<swift/bridging>)
#include <swift/bridging>
#else
#define SWIFT_RETURNS_INDEPENDENT_VALUE
#endif
#include "kalah.h"

// Thin C++ wrapper around KalahGame designed for clean Swift/C++ interop.
// Owns both game state and app-flow state (the 5-step onboarding sequence).
// All methods use primitive types (int, bool, const char*) so Swift needs no
// knowledge of std::string, enum class, or C++ reference semantics.
class KalahSwift {
    KalahGame game;
    int appState_ = 0;          // 0=Welcome 1=EnterName 2=SelectGender 3=SelectDifficulty 4=Playing
    mutable std::string nameCache_;

public:
    KalahSwift() {}

    // App state ---------------------------------------------------------------

    int appState() const { return appState_; }

    void proceedFromWelcome() { appState_ = 1; }

    void submitName(const char *name) {
        std::string n(name ? name : "");
        if (n.size() > 24) n.resize(24);
        game.setUserName(n);
        appState_ = 2;
    }

    void selectGender(int g) {
        game.setGender(static_cast<Gender>(g));
        appState_ = 3;
    }

    void selectLevel(int l) {
        game.setLevel(static_cast<Level>(l));
        game.initializeBoard();
        appState_ = 4;
    }

    // Move actions ------------------------------------------------------------

    // Returns: 0=INVALID 1=SWITCH_TURN 2=EXTRA_TURN
    int sow(int pit)      { return static_cast<int>(game.makeMove(Player::USER, pit)); }
    int doAIMove(int pit) { return static_cast<int>(game.makeMove(Player::JINN, pit)); }
    int selectAIMove()    { return game.selectAIMove(); }

    void newGame() { game.initializeBoard(); }

    // Board read-back ---------------------------------------------------------

    // player: 0=JINN, 1=USER;  index: 0-5 regular pits, 6=kalah
    int getPitAt(int player, int index) const {
        return game.getPosition().sides[static_cast<size_t>(player)].pits[static_cast<size_t>(index)];
    }

    // Returns 0=USER (bottom), 1=JINN (top) — matches KalahViewModel convention
    int currentPlayer() const {
        return game.getPosition().currentPlayer == Player::USER ? 0 : 1;
    }

    bool isGameOver() const { return game.getPosition().isGameOver(); }

    void collectRemaining() { game.getPosition().collectRemaining(); }

    // Returns 0=user wins, 1=jinn wins, -1=tie
    int winner() const {
        const Position &pos = game.getPosition();
        int u = pos[Player::USER].getKalah();
        int j = pos[Player::JINN].getKalah();
        if (u > j) return 0;
        if (j > u) return 1;
        return -1;
    }

    // Pointer is valid until the next call to submitName or setUserName
    SWIFT_RETURNS_INDEPENDENT_VALUE const char *getUserName() const {
        nameCache_ = game.getUserName();
        return nameCache_.c_str();
    }
};
