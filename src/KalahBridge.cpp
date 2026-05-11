#include "KalahBridge.h"
#include "kalah.h"

#include <algorithm>
#include <string>

struct KalahHandle {
    KalahGame game;
    int appState = 0;
    mutable std::string cachedName;
};

#define H(ptr)  (reinterpret_cast<KalahHandle *>(ptr))
#define CH(ptr) (reinterpret_cast<const KalahHandle *>(ptr))

void *kalah_create()         { return new KalahHandle(); }
void  kalah_destroy(void *h) { delete H(h); }

int  kalah_get_app_state(const void *h) { return CH(h)->appState; }

void kalah_proceed_from_welcome(void *h) { H(h)->appState = 1; }

void kalah_submit_name(void *h, const char *name)
{
    std::string n(name ? name : "");
    if (n.size() > 24) n.resize(24);
    H(h)->game.setUserName(n);
    H(h)->appState = 2;
}

void kalah_select_gender(void *h, int g)
{
    H(h)->game.setGender(static_cast<Gender>(g));
    H(h)->appState = 3;
}

void kalah_select_level(void *h, int l)
{
    H(h)->game.setLevel(static_cast<Level>(l));
    H(h)->game.initializeBoard();
    H(h)->appState = 4;
}

int kalah_sow(void *h, int pit)
{
    return static_cast<int>(H(h)->game.makeMove(Player::USER, pit));
}

int kalah_do_ai_move(void *h, int pit)
{
    return static_cast<int>(H(h)->game.makeMove(Player::JINN, pit));
}

int kalah_select_ai_move(void *h)
{
    return H(h)->game.selectAIMove();
}

void kalah_new_game(void *h)
{
    H(h)->game.initializeBoard();
    // Stay in Playing state (appState remains 4)
}

void kalah_get_pits(const void *h, int *out_pits)
{
    const Position &pos = CH(h)->game.getPosition();
    for (int i = 0; i < PITS_PER_SIDE; ++i)
        out_pits[i] = pos[Player::USER][i];
    out_pits[6] = pos[Player::USER].getKalah();
    for (int i = 0; i < PITS_PER_SIDE; ++i)
        out_pits[7 + i] = pos[Player::JINN][i];
    out_pits[13] = pos[Player::JINN].getKalah();
}

int kalah_current_player(const void *h)
{
    return CH(h)->game.getPosition().currentPlayer == Player::USER ? 0 : 1;
}

int kalah_is_game_over(const void *h)
{
    return CH(h)->game.getPosition().isGameOver() ? 1 : 0;
}

int kalah_winner(const void *h)
{
    const Position &pos = CH(h)->game.getPosition();
    int u = pos[Player::USER].getKalah();
    int j = pos[Player::JINN].getKalah();
    if (u > j) return 0;
    if (j > u) return 1;
    return -1;
}

void kalah_collect_remaining(void *h)
{
    H(h)->game.getPosition().collectRemaining();
}

const char *kalah_get_user_name(const void *h)
{
    CH(h)->cachedName = CH(h)->game.getUserName();
    return CH(h)->cachedName.c_str();
}
