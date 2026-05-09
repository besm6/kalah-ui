#pragma once

#include <sigc++/sigc++.h>
#include <glibmm/main.h>
#include <array>
#include <memory>
#include <string>

#include "kalah.h"

class GameController {
public:
    enum class AppState { Welcome=0, EnterName=1, SelectGender=2, SelectDifficulty=3, Playing=4 };

    GameController();
    ~GameController();

    // Accessors
    int         appState()     const { return static_cast<int>(m_appState); }
    std::string userName()     const { return m_game->getUserName(); }
    std::array<int, 14> pits() const;
    int  currentPlayer() const;  // 0=USER(bottom), 1=JINN(top)
    bool gameOver()      const { return m_game->getPosition().isGameOver(); }
    int  winner()        const;  // 0=USER wins, 1=JINN wins, -1=tie
    bool aiThinking()    const { return m_aiThinking; }

    // Actions (formerly Qt slots)
    void proceedFromWelcome();
    void submitName(const std::string& name);
    void selectGender(int g);
    void selectLevel(int l);
    void sow(int pitIndex);
    void newGame();

    // sigc++ signals (formerly Qt signals)
    sigc::signal<void()>  signal_app_state_changed;
    sigc::signal<void()>  signal_user_name_changed;
    sigc::signal<void()>  signal_board_changed;
    sigc::signal<void()>  signal_current_player_changed;
    sigc::signal<void()>  signal_game_over_changed;
    sigc::signal<void()>  signal_ai_thinking_changed;
    sigc::signal<void(int)> signal_illegal_move;

private:
    std::unique_ptr<KalahGame> m_game;
    AppState         m_appState   = AppState::Welcome;
    bool             m_aiThinking = false;
    sigc::connection m_ai_timer_conn;

    void setAppState(AppState s);
    void afterMove(MoveResult result);
    void scheduleAIMove();
    void doAIMove();
};
