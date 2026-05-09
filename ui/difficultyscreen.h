#pragma once

#include <gtkmm.h>
#include "gamecontroller.h"

class DifficultyScreen : public Gtk::Box {
public:
    DifficultyScreen();
    void set_controller(GameController& c);

private:
    GameController* m_ctrl = nullptr;
    Gtk::Label m_label;

    struct DiffButton {
        Gtk::Button btn;
        Gtk::Box    inner{Gtk::Orientation::VERTICAL, 2};
        Gtk::Label  title;
        Gtk::Label  sub;
    };
    DiffButton m_levels[4];

    void make_level(DiffButton& db, const char* title, const char* subtitle, int value);
};
