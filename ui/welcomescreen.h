#pragma once

#include <gtkmm.h>
#include "gamecontroller.h"

class WelcomeScreen : public Gtk::Box {
public:
    WelcomeScreen();
    void set_controller(GameController& c);

private:
    GameController* m_ctrl = nullptr;
    Gtk::Label m_title;
    Gtk::Label m_subtitle;
    Gtk::Label m_tap_hint;
};
