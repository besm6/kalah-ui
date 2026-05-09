#pragma once

#include <gtkmm.h>
#include "gamecontroller.h"

class NameScreen : public Gtk::Box {
public:
    NameScreen();
    void set_controller(GameController& c);

private:
    GameController* m_ctrl = nullptr;
    Gtk::Label  m_label;
    Gtk::Entry  m_entry;
    Gtk::Button m_continue_btn;

    void on_submit();
};
