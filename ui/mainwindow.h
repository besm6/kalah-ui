#pragma once

#include <gtkmm.h>
#include "gamecontroller.h"
#include "welcomescreen.h"
#include "namescreen.h"
#include "genderscreen.h"
#include "difficultyscreen.h"
#include "gamescreen.h"

class MainWindow : public Gtk::ApplicationWindow {
public:
    MainWindow();

private:
    GameController   m_controller;
    Gtk::Stack       m_stack;
    WelcomeScreen    m_welcome;
    NameScreen       m_name;
    GenderScreen     m_gender;
    DifficultyScreen m_difficulty;
    GameScreen       m_game;

    void on_app_state_changed();
    void load_css();
};
