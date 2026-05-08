#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <memory>

#include "kalah.h"

class GameController : public QObject {
    Q_OBJECT

    Q_PROPERTY(int appState READ appState NOTIFY appStateChanged)
    Q_PROPERTY(QString userName READ userName NOTIFY userNameChanged)
    Q_PROPERTY(QVariantList pits READ pits NOTIFY boardChanged)
    Q_PROPERTY(int currentPlayer READ currentPlayer NOTIFY currentPlayerChanged)
    Q_PROPERTY(bool gameOver READ gameOver NOTIFY gameOverChanged)
    Q_PROPERTY(int winner READ winner NOTIFY gameOverChanged)
    Q_PROPERTY(bool aiThinking READ aiThinking NOTIFY aiThinkingChanged)

public:
    enum AppState { Welcome, EnterName, SelectGender, SelectDifficulty, Playing };
    Q_ENUM(AppState)

    explicit GameController(QObject *parent = nullptr);

    // Returns the current screen as an integer: Welcome(0), EnterName(1), SelectGender(2),
    // SelectDifficulty(3), or Playing(4). QML screens read this property to know what to show.
    int appState() const { return static_cast<int>(m_appState); }
    QString userName() const;
    QVariantList pits() const;
    int currentPlayer() const;
    bool gameOver() const;
    int winner() const;
    // Returns true while the AI timer is counting down before making its move.
    // The UI uses this flag to disable player taps and show a thinking indicator.
    bool aiThinking() const { return m_aiThinking; }

public slots:
    void proceedFromWelcome();
    void submitName(const QString &name);
    void selectGender(int g);
    void selectLevel(int l);
    void sow(int pitIndex);
    void newGame();

signals:
    void appStateChanged();
    void userNameChanged();
    void boardChanged();
    void currentPlayerChanged();
    void gameOverChanged();
    void aiThinkingChanged();
    void illegalMove(int pitIndex);

private:
    std::unique_ptr<KalahGame> m_game;
    AppState m_appState = Welcome;
    bool m_aiThinking   = false;
    QTimer *m_aiTimer;

    void setAppState(AppState s);
    void afterMove(MoveResult result);
    void scheduleAIMove();
    void doAIMove();
};

#endif // GAMECONTROLLER_H
