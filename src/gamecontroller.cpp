#include "gamecontroller.h"

GameController::GameController(QObject *parent)
    : QObject(parent), m_game(std::make_unique<KalahGame>()), m_aiTimer(new QTimer(this))
{
    m_aiTimer->setSingleShot(true);
    connect(m_aiTimer, &QTimer::timeout, this, &GameController::doAIMove);
}

void GameController::setAppState(AppState s)
{
    if (m_appState == s)
        return;
    m_appState = s;
    emit appStateChanged();
}

QString GameController::userName() const
{
    return QString::fromStdString(m_game->getUserName());
}

QVariantList GameController::pits() const
{
    const Position &pos = m_game->getPosition();
    QVariantList list;
    list.reserve(14);
    // indices 0..5: USER pits, index 6: USER kalah
    for (int i = 0; i < PITS_PER_SIDE; ++i)
        list.append(pos[Player::USER][i]);
    list.append(pos[Player::USER].getKalah());
    // indices 7..12: JINN pits, index 13: JINN kalah
    for (int i = 0; i < PITS_PER_SIDE; ++i)
        list.append(pos[Player::JINN][i]);
    list.append(pos[Player::JINN].getKalah());
    return list;
}

int GameController::currentPlayer() const
{
    // USER=1 maps to player 0 (bottom), JINN=0 maps to player 1 (top)
    return m_game->getPosition().currentPlayer == Player::USER ? 0 : 1;
}

bool GameController::gameOver() const
{
    return m_game->getPosition().isGameOver();
}

int GameController::winner() const
{
    const Position &pos = m_game->getPosition();
    int userKalah       = pos[Player::USER].getKalah();
    int jinnKalah       = pos[Player::JINN].getKalah();
    if (userKalah > jinnKalah)
        return 0; // USER wins
    if (jinnKalah > userKalah)
        return 1; // JINN wins
    return -1;    // tie
}

void GameController::proceedFromWelcome()
{
    setAppState(EnterName);
}

void GameController::submitName(const QString &name)
{
    m_game->setUserName(name.toStdString());
    emit userNameChanged();
    setAppState(SelectGender);
}

void GameController::selectGender(int g)
{
    m_game->setGender(static_cast<Gender>(g));
    setAppState(SelectDifficulty);
}

void GameController::selectLevel(int l)
{
    m_game->setLevel(static_cast<Level>(l));
    m_game->initializeBoard();
    setAppState(Playing);
    emit boardChanged();
    emit currentPlayerChanged();
    emit gameOverChanged();
}

void GameController::sow(int pitIndex)
{
    if (m_aiThinking || gameOver())
        return;
    if (m_game->getPosition().currentPlayer != Player::USER)
        return;

    MoveResult result = m_game->makeMove(Player::USER, pitIndex);
    if (result == MoveResult::INVALID) {
        emit illegalMove(pitIndex);
        return;
    }
    afterMove(result);
}

void GameController::newGame()
{
    m_aiTimer->stop();
    if (m_aiThinking) {
        m_aiThinking = false;
        emit aiThinkingChanged();
    }
    m_game->initializeBoard();
    emit boardChanged();
    emit currentPlayerChanged();
    emit gameOverChanged();
    setAppState(Playing);
}

void GameController::afterMove(MoveResult result)
{
    emit boardChanged();

    if (m_game->getPosition().isGameOver()) {
        m_game->getPosition().collectRemaining();
        emit boardChanged();
        emit gameOverChanged();
        return;
    }

    if (result == MoveResult::EXTRA_TURN) {
        // same player goes again; no turn switch signal needed
        return;
    }

    // SWITCH_TURN
    emit currentPlayerChanged();

    if (m_game->getPosition().currentPlayer == Player::JINN) {
        scheduleAIMove();
    }
}

void GameController::scheduleAIMove()
{
    m_aiThinking = true;
    emit aiThinkingChanged();
    m_aiTimer->start(700);
}

void GameController::doAIMove()
{
    int pit = m_game->selectAIMove();
    if (pit < 0) {
        // No valid AI move — game should already be over
        m_aiThinking = false;
        emit aiThinkingChanged();
        return;
    }

    MoveResult result = m_game->makeMove(Player::JINN, pit);
    emit boardChanged();

    if (m_game->getPosition().isGameOver()) {
        m_game->getPosition().collectRemaining();
        m_aiThinking = false;
        emit aiThinkingChanged();
        emit boardChanged();
        emit gameOverChanged();
        return;
    }

    if (result == MoveResult::EXTRA_TURN) {
        // JINN gets another move
        m_aiTimer->start(700);
        return;
    }

    // SWITCH_TURN — back to USER
    m_aiThinking = false;
    emit aiThinkingChanged();
    emit currentPlayerChanged();
}
