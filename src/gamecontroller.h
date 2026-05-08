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

    int appState() const { return static_cast<int>(m_appState); }
    QString userName() const;
    QVariantList pits() const;
    int currentPlayer() const;
    bool gameOver() const;
    int winner() const;
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
