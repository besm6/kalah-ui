#include "gamescreen.h"

GameScreen::GameScreen()
    : Gtk::Box(Gtk::Orientation::VERTICAL, 8)
    , m_user_store("Player")
{
    set_expand(true);
    set_margin_start(16);
    set_margin_end(16);
    set_margin_top(12);
    set_margin_bottom(12);
    add_css_class("screen");
    add_css_class("screen-game");

    // Score bar
    m_player_label.add_css_class("score");
    m_player_label.set_text("Player");
    m_player_label.set_halign(Gtk::Align::START);
    m_player_label.set_hexpand(true);

    m_score_label.add_css_class("score-center");
    m_score_label.set_text("0 — 0");
    m_score_label.set_halign(Gtk::Align::CENTER);
    m_score_label.set_hexpand(true);

    m_jinn_label.add_css_class("player-label");
    m_jinn_label.set_text("Jinn");
    m_jinn_label.set_halign(Gtk::Align::END);
    m_jinn_label.set_hexpand(true);

    m_score_bar.append(m_player_label);
    m_score_bar.append(m_score_label);
    m_score_bar.append(m_jinn_label);

    // Status label
    m_status_label.add_css_class("status");
    m_status_label.set_text("Your turn");
    m_status_label.set_halign(Gtk::Align::CENTER);

    // Board
    build_board();

    m_board_row.set_halign(Gtk::Align::CENTER);
    m_board_row.set_valign(Gtk::Align::CENTER);
    m_board_row.set_vexpand(true);
    m_board_row.append(m_jinn_store);
    m_board_row.append(m_pit_grid);
    m_board_row.append(m_user_store);

    // New Game button
    m_new_game_btn.add_css_class("btn-newgame");
    m_new_game_btn.set_halign(Gtk::Align::CENTER);

    append(m_score_bar);
    append(m_status_label);
    append(m_board_row);
    append(m_new_game_btn);
}

void GameScreen::build_board()
{
    m_pit_grid.set_row_spacing(8);
    m_pit_grid.set_column_spacing(8);
    m_pit_grid.set_halign(Gtk::Align::CENTER);
    m_pit_grid.set_valign(Gtk::Align::CENTER);

    // JINN pits: top row, displayed right-to-left (pit 12 at col 0 ... pit 7 at col 5)
    for (int col = 0; col < 6; ++col) {
        int board_index = 12 - col; // board indices 12, 11, 10, 9, 8, 7
        auto* pit = new PitWidget(board_index, false);
        m_jinn_pits[col] = pit;
        m_pit_grid.attach(*pit, col, 0);
    }

    // USER pits: bottom row, left-to-right (pit 0 at col 0 ... pit 5 at col 5)
    for (int col = 0; col < 6; ++col) {
        int board_index = col; // board indices 0..5
        auto* pit = new PitWidget(board_index, true);
        m_user_pits[col] = pit;
        m_pit_grid.attach(*pit, col, 1);
    }
}

void GameScreen::set_controller(GameController& c)
{
    m_ctrl = &c;

    c.signal_board_changed.connect(sigc::mem_fun(*this, &GameScreen::refresh_board));
    c.signal_current_player_changed.connect(sigc::mem_fun(*this, &GameScreen::refresh_board));
    c.signal_game_over_changed.connect(sigc::mem_fun(*this, &GameScreen::refresh_status));
    c.signal_ai_thinking_changed.connect(sigc::mem_fun(*this, &GameScreen::refresh_status));

    m_new_game_btn.signal_clicked().connect([this] { m_ctrl->newGame(); });

    for (int col = 0; col < 6; ++col) {
        m_user_pits[col]->signal_tapped.connect([this](int idx) { m_ctrl->sow(idx); });
    }
}

void GameScreen::refresh_board()
{
    if (!m_ctrl) return;
    auto p = m_ctrl->pits();

    // Update score bar
    m_player_label.set_text(m_ctrl->userName());

    m_score_label.set_text(std::to_string(p[6]) + " \xe2\x80\x94 " + std::to_string(p[13]));

    // Update store labels (user store label tracks user name)
    m_user_store.update(p[6]);
    m_jinn_store.update(p[13]);

    bool user_can_play = !m_ctrl->gameOver()
                      && !m_ctrl->aiThinking()
                      && m_ctrl->currentPlayer() == 0;

    // USER pits (bottom row): board indices 0..5
    for (int col = 0; col < 6; ++col) {
        bool playable = user_can_play && p[col] > 0;
        m_user_pits[col]->update(p[col], playable);
    }

    // JINN pits (top row): col 0 → board index 12, col 5 → board index 7
    for (int col = 0; col < 6; ++col) {
        int board_idx = 12 - col;
        m_jinn_pits[col]->update(p[board_idx], false);
    }

    refresh_status();
}

void GameScreen::refresh_status()
{
    if (!m_ctrl) return;

    m_status_label.remove_css_class("gameover");
    m_status_label.remove_css_class("thinking");

    if (m_ctrl->gameOver()) {
        int w = m_ctrl->winner();
        if (w == -1) {
            m_status_label.set_text("Tie game!");
        } else if (w == 0) {
            m_status_label.set_text(m_ctrl->userName() + " wins!");
        } else {
            m_status_label.set_text("Jinn wins!");
        }
        m_status_label.add_css_class("gameover");
    } else if (m_ctrl->aiThinking()) {
        m_status_label.set_text("Jinn is thinking\xe2\x80\xa6");
        m_status_label.add_css_class("thinking");
    } else {
        m_status_label.set_text("Your turn");
    }
}
