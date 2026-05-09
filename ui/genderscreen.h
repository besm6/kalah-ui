#pragma once

#include <gtkmm.h>
#include "gamecontroller.h"

class GenderScreen : public Gtk::Box {
public:
    GenderScreen();
    void set_controller(GameController& c);

private:
    GameController* m_ctrl = nullptr;
    Gtk::Label  m_label;
    Gtk::Button m_male;
    Gtk::Button m_female;
    Gtk::Button m_other;
};
