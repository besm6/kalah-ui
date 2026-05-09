#pragma once

#include <gtkmm.h>
#include <array>
#include "gamecontroller.h"
#include "pitwidget.h"
#include "storewidget.h"

class GameScreen : public Gtk::Box {
public:
    GameScreen();
    void set_controller(GameController& c);

private:
    GameController* m_ctrl = nullptr;

    // Score bar
    Gtk::Box   m_score_bar{Gtk::Orientation::HORIZONTAL, 0};
    Gtk::Label m_player_label;
    Gtk::Label m_score_label;
    Gtk::Label m_jinn_label;

    // Status
    Gtk::Label m_status_label;

    // Board row
    Gtk::Box       m_board_row{Gtk::Orientation::HORIZONTAL, 8};
    StoreWidget    m_jinn_store{"Jinn"};
    Gtk::Grid      m_pit_grid;
    StoreWidget    m_user_store;   // label set from userName in refresh
    std::string    m_user_store_label;

    // Pits: indices 0-5 USER (bottom row), 7-12 JINN (top row, stored as 0-5)
    std::array<PitWidget*, 6> m_user_pits{};
    std::array<PitWidget*, 6> m_jinn_pits{};

    // New Game button
    Gtk::Button m_new_game_btn{"New Game"};

    void refresh_board();
    void refresh_status();

    // Pit objects are heap-allocated and owned by the grid
    void build_board();
};
