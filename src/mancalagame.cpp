#include "mancalagame.h"

MancalaGame::MancalaGame(QObject *parent)
    : QObject(parent)
{
    reset();
}

void MancalaGame::reset()
{
    m_board.fill(0);
    for (int i = 0; i < PitsPerSide; ++i) {
        m_board[i] = 4;                       // P0 row
        m_board[i + PitsPerSide + 1] = 4;     // P1 row
    }
    m_currentPlayer = 0;
    m_gameOver = false;
    m_winner = -1;

    emit boardChanged();
    emit currentPlayerChanged();
    emit gameOverChanged();
}

QVariantList MancalaGame::pits() const
{
    QVariantList list;
    list.reserve(BoardSize);
    for (int v : m_board) list.push_back(v);
    return list;
}

bool MancalaGame::isOwnPit(int player, int index) const
{
    if (player == 0) return index >= 0 && index <= 5;
    return index >= 7 && index <= 12;
}

bool MancalaGame::sideEmpty(int player) const
{
    int begin = (player == 0) ? 0 : 7;
    for (int i = 0; i < PitsPerSide; ++i)
        if (m_board[begin + i] != 0) return false;
    return true;
}

bool MancalaGame::sow(int pitIndex)
{
    if (m_gameOver) return false;
    if (pitIndex < 0 || pitIndex >= BoardSize) {
        emit illegalMove(pitIndex); return false;
    }
    if (!isOwnPit(m_currentPlayer, pitIndex) || m_board[pitIndex] == 0) {
        emit illegalMove(pitIndex); return false;
    }

    int stones = m_board[pitIndex];
    m_board[pitIndex] = 0;

    int idx = pitIndex;
    const int skipStore = opponentStore(m_currentPlayer);

    while (stones > 0) {
        idx = (idx + 1) % BoardSize;
        if (idx == skipStore) continue;       // never sow into opponent's store
        m_board[idx] += 1;
        --stones;
    }

    bool extraTurn = (idx == ownStore(m_currentPlayer));
    bool captured  = false;

    // Capture rule: last stone in own empty pit -> take that + opposite pit.
    if (!extraTurn && isOwnPit(m_currentPlayer, idx) && m_board[idx] == 1) {
        int opposite = 12 - idx;              // mirror across board
        if (m_board[opposite] > 0) {
            m_board[ownStore(m_currentPlayer)] += m_board[opposite] + 1;
            m_board[opposite] = 0;
            m_board[idx] = 0;
            captured = true;
        }
    }

    emit boardChanged();
    emit moveCompleted(idx, extraTurn, captured);

    if (!extraTurn) {
        m_currentPlayer = 1 - m_currentPlayer;
        emit currentPlayerChanged();
    }

    checkEndgame();
    return true;
}

void MancalaGame::checkEndgame()
{
    if (!sideEmpty(0) && !sideEmpty(1)) return;

    // Sweep remaining stones into each player's store.
    for (int i = 0; i < PitsPerSide; ++i) {
        m_board[P0Store] += m_board[i];
        m_board[i] = 0;
        m_board[P1Store] += m_board[i + PitsPerSide + 1];
        m_board[i + PitsPerSide + 1] = 0;
    }
    m_gameOver = true;
    if      (m_board[P0Store] >  m_board[P1Store]) m_winner = 0;
    else if (m_board[P0Store] <  m_board[P1Store]) m_winner = 1;
    else                                           m_winner = -1;

    emit boardChanged();
    emit gameOverChanged();
}
