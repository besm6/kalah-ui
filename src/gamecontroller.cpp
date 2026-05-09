#include "gamecontroller.h"

GameController::GameController()
    : m_game(std::make_unique<KalahGame>())
{}

GameController::~GameController()
{
    m_ai_timer_conn.disconnect();
}

void GameController::setAppState(AppState s)
{
    if (m_appState == s)
        return;
    m_appState = s;
    signal_app_state_changed.emit();
}

std::array<int, 14> GameController::pits() const
{
    const Position& pos = m_game->getPosition();
    std::array<int, 14> arr;
    for (int i = 0; i < PITS_PER_SIDE; ++i)
        arr[i] = pos[Player::USER][i];
    arr[6] = pos[Player::USER].getKalah();
    for (int i = 0; i < PITS_PER_SIDE; ++i)
        arr[7 + i] = pos[Player::JINN][i];
    arr[13] = pos[Player::JINN].getKalah();
    return arr;
}

int GameController::currentPlayer() const
{
    return m_game->getPosition().currentPlayer == Player::USER ? 0 : 1;
}

int GameController::winner() const
{
    const Position& pos = m_game->getPosition();
    int userKalah = pos[Player::USER].getKalah();
    int jinnKalah = pos[Player::JINN].getKalah();
    if (userKalah > jinnKalah) return 0;
    if (jinnKalah > userKalah) return 1;
    return -1;
}

void GameController::proceedFromWelcome()
{
    setAppState(AppState::EnterName);
}

void GameController::submitName(const std::string& name)
{
    m_game->setUserName(name);
    signal_user_name_changed.emit();
    setAppState(AppState::SelectGender);
}

void GameController::selectGender(int g)
{
    m_game->setGender(static_cast<Gender>(g));
    setAppState(AppState::SelectDifficulty);
}

void GameController::selectLevel(int l)
{
    m_game->setLevel(static_cast<Level>(l));
    m_game->initializeBoard();
    setAppState(AppState::Playing);
    signal_board_changed.emit();
    signal_current_player_changed.emit();
    signal_game_over_changed.emit();
}

void GameController::sow(int pitIndex)
{
    if (m_aiThinking || gameOver())
        return;
    if (m_game->getPosition().currentPlayer != Player::USER)
        return;

    MoveResult result = m_game->makeMove(Player::USER, pitIndex);
    if (result == MoveResult::INVALID) {
        signal_illegal_move.emit(pitIndex);
        return;
    }
    afterMove(result);
}

void GameController::newGame()
{
    m_ai_timer_conn.disconnect();
    if (m_aiThinking) {
        m_aiThinking = false;
        signal_ai_thinking_changed.emit();
    }
    m_game->initializeBoard();
    signal_board_changed.emit();
    signal_current_player_changed.emit();
    signal_game_over_changed.emit();
    setAppState(AppState::Playing);
}

void GameController::afterMove(MoveResult result)
{
    signal_board_changed.emit();

    if (m_game->getPosition().isGameOver()) {
        m_game->getPosition().collectRemaining();
        signal_board_changed.emit();
        signal_game_over_changed.emit();
        return;
    }

    if (result == MoveResult::EXTRA_TURN)
        return;

    signal_current_player_changed.emit();

    if (m_game->getPosition().currentPlayer == Player::JINN)
        scheduleAIMove();
}

void GameController::scheduleAIMove()
{
    m_aiThinking = true;
    signal_ai_thinking_changed.emit();
    m_ai_timer_conn = Glib::signal_timeout().connect([this]() -> bool {
        doAIMove();
        return false;
    }, 700);
}

void GameController::doAIMove()
{
    int pit = m_game->selectAIMove();
    if (pit < 0) {
        m_aiThinking = false;
        signal_ai_thinking_changed.emit();
        return;
    }

    MoveResult result = m_game->makeMove(Player::JINN, pit);
    signal_board_changed.emit();

    if (m_game->getPosition().isGameOver()) {
        m_game->getPosition().collectRemaining();
        m_aiThinking = false;
        signal_ai_thinking_changed.emit();
        signal_board_changed.emit();
        signal_game_over_changed.emit();
        return;
    }

    if (result == MoveResult::EXTRA_TURN) {
        m_ai_timer_conn = Glib::signal_timeout().connect([this]() -> bool {
            doAIMove();
            return false;
        }, 700);
        return;
    }

    m_aiThinking = false;
    signal_ai_thinking_changed.emit();
    signal_current_player_changed.emit();
}
