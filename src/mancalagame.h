#pragma once

#include <QObject>
#include <QVariantList>
#include <array>

// Standard Kalah-style mancala.
// Board layout (indices, counterclockwise from player 0's first pit):
//
//        12  11  10   9   8   7
//   13                              6     <- 13 = P1 store, 6 = P0 store
//         0   1   2   3   4   5
//
// P0 owns pits 0..5 and store 6.
// P1 owns pits 7..12 and store 13.

class MancalaGame : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList pits READ pits NOTIFY boardChanged)
    Q_PROPERTY(int currentPlayer READ currentPlayer NOTIFY currentPlayerChanged)
    Q_PROPERTY(bool gameOver READ gameOver NOTIFY gameOverChanged)
    Q_PROPERTY(int winner READ winner NOTIFY gameOverChanged)

public:
    explicit MancalaGame(QObject *parent = nullptr);

    QVariantList pits() const;
    int currentPlayer() const { return m_currentPlayer; }
    bool gameOver() const { return m_gameOver; }
    int winner() const { return m_winner; }  // 0, 1, or -1 for tie

    static constexpr int PitsPerSide = 6;
    static constexpr int BoardSize = 14;
    static constexpr int P0Store = 6;
    static constexpr int P1Store = 13;

public slots:
    // Returns true if the move was legal and applied.
    bool sow(int pitIndex);
    void reset();

signals:
    void boardChanged();
    void currentPlayerChanged();
    void gameOverChanged();
    void illegalMove(int pitIndex);
    void moveCompleted(int lastIndex, bool extraTurn, bool capture);

private:
    bool isOwnPit(int player, int index) const;
    int  ownStore(int player) const { return player == 0 ? P0Store : P1Store; }
    int  opponentStore(int player) const { return player == 0 ? P1Store : P0Store; }
    bool sideEmpty(int player) const;
    void checkEndgame();

    std::array<int, BoardSize> m_board{};
    int  m_currentPlayer = 0;
    bool m_gameOver = false;
    int  m_winner   = -1;
};
