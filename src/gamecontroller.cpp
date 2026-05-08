#include "gamecontroller.h"

// Creates the game engine and wires up the AI delay timer.
// The timer is configured to fire only once per activation; when it fires it calls doAIMove.
GameController::GameController(QObject *parent)
    : QObject(parent), m_game(std::make_unique<KalahGame>()), m_aiTimer(new QTimer(this))
{
    m_aiTimer->setSingleShot(true);
    connect(m_aiTimer, &QTimer::timeout, this, &GameController::doAIMove);
}

// Changes the current screen and notifies QML via a signal.
// Skips the signal if the state has not actually changed, avoiding unnecessary redraws.
void GameController::setAppState(AppState s)
{
    if (m_appState == s)
        return;
    m_appState = s;
    emit appStateChanged();
}

// Converts the player's name from a C++ std::string to a Qt QString
// so QML can display it directly.
QString GameController::userName() const
{
    return QString::fromStdString(m_game->getUserName());
}

// Packages all 14 pit counts into a flat list the QML board reads.
// Indices 0–6 hold the USER's six pits and kalah; indices 7–13 hold JINN's.
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

// Returns 0 when it is the human player's turn (bottom row)
// and 1 when the AI plays (top row). QML uses this to highlight playable pits.
int GameController::currentPlayer() const
{
    // USER=1 maps to player 0 (bottom), JINN=0 maps to player 1 (top)
    return m_game->getPosition().currentPlayer == Player::USER ? 0 : 1;
}

// Delegates to the game engine to check whether either side has run out of stones.
// Returns true when the game has ended and no more moves can be made.
bool GameController::gameOver() const
{
    return m_game->getPosition().isGameOver();
}

// Compares the two kalahs after the game ends.
// Returns 0 if the USER wins, 1 if JINN wins, or -1 for a tie.
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

// Called when the player taps the welcome screen.
// Advances the app to the name-entry screen.
void GameController::proceedFromWelcome()
{
    setAppState(EnterName);
}

// Saves the typed name into the game engine, notifies QML that the name changed,
// and advances to the gender-selection screen.
void GameController::submitName(const QString &name)
{
    m_game->setUserName(name.toStdString());
    emit userNameChanged();
    setAppState(SelectGender);
}

// Saves the gender choice into the game engine and moves to the difficulty-selection screen.
void GameController::selectGender(int g)
{
    m_game->setGender(static_cast<Gender>(g));
    setAppState(SelectDifficulty);
}

// Applies the chosen difficulty, resets the board to the starting position, and
// switches to the playing screen. Emits signals so QML refreshes the board display.
void GameController::selectLevel(int l)
{
    m_game->setLevel(static_cast<Level>(l));
    m_game->initializeBoard();
    setAppState(Playing);
    emit boardChanged();
    emit currentPlayerChanged();
    emit gameOverChanged();
}

// Called when the player taps a pit. Silently ignores taps while the AI is thinking,
// after the game ends, or when it is not the player's turn. If the move is valid,
// hands off to afterMove(); otherwise emits illegalMove so the UI can react.
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

// Cancels any pending AI move, resets the board to the starting position,
// and stays on the Playing screen so a fresh game begins immediately.
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

// Runs after any valid move (human or AI). Refreshes the board display, checks if the
// game just ended (sweeping remaining stones into kalahs if so), grants an extra turn
// if earned, or emits currentPlayerChanged and schedules the AI's response.
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

// Sets the "AI thinking" flag, notifies QML, and starts the 700 ms delay timer.
// The brief pause makes the AI feel more natural rather than responding instantly.
void GameController::scheduleAIMove()
{
    m_aiThinking = true;
    emit aiThinkingChanged();
    m_aiTimer->start(700);
}

// Fires when the AI timer expires. Asks the engine to choose and execute a move,
// updates the board display, and either grants the AI an extra turn or hands
// control back to the player by clearing the aiThinking flag.
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
