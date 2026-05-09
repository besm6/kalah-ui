#include "mainwindow.h"

static constexpr const char* kCss = R"css(
* {
    font-family: Georgia, serif;
}
window, .screen {
    background-color: #1a0f05;
    color: #c8b89a;
}
.screen-name, .screen-gender, .screen-difficulty {
    background-color: #2a1e0e;
}
.screen-game {
    background-color: #3b2a14;
}
label.title {
    color: #d4a84b;
    font-size: 48pt;
    font-weight: bold;
}
label.subtitle {
    color: #c8b89a;
    font-size: 16pt;
}
label.tap-hint {
    color: #a09070;
    font-size: 14pt;
}
label.score {
    color: #d4a84b;
    font-size: 14pt;
    font-weight: bold;
}
label.score-center {
    color: #c8b89a;
    font-size: 14pt;
}
label.status {
    color: #c8b89a;
    font-size: 14pt;
}
label.status.gameover {
    color: #d4a84b;
}
label.status.thinking {
    color: #a08060;
}
label.player-label {
    color: #a08060;
    font-size: 14pt;
    font-weight: bold;
}
entry.name-entry {
    background-color: #1a0f05;
    color: #f0e6d0;
    border: 2px solid #5a4030;
    border-radius: 8px;
    font-size: 20pt;
    padding: 6px 12px;
    caret-color: #d4a84b;
}
entry.name-entry:focus {
    border-color: #d4a84b;
    outline: none;
}
button.btn-gold {
    background: #d4a84b;
    color: #1a0f05;
    border: 2px solid #f0c860;
    border-radius: 8px;
    font-size: 16pt;
    padding: 8px 24px;
}
button.btn-gold:disabled {
    background: #3a2a10;
    color: #806040;
    border-color: #5a4030;
}
button.btn-choice {
    background-color: #1a0f05;
    color: #d4c8b0;
    border: 2px solid #5a4030;
    border-radius: 10px;
    font-size: 18pt;
    padding: 12px 16px;
    min-height: 56px;
}
button.btn-choice:hover {
    border-color: #d4a84b;
}
button.btn-choice:active {
    background-color: #2a1e0e;
    border-color: #d4a84b;
}
button.btn-newgame {
    background: #d4a84b;
    color: #1a0f05;
    border: 2px solid #f0c860;
    border-radius: 8px;
    font-size: 15pt;
    padding: 6px 16px;
    min-width: 160px;
}
label.difficulty-title {
    color: #d4c8b0;
    font-size: 16pt;
    font-weight: bold;
}
label.difficulty-sub {
    color: #907060;
    font-size: 10pt;
}
)css";

MainWindow::MainWindow()
{
    set_title("Kalah");
    set_default_size(800, 480);
    set_resizable(false);

    load_css();

    m_welcome.set_controller(m_controller);
    m_name.set_controller(m_controller);
    m_gender.set_controller(m_controller);
    m_difficulty.set_controller(m_controller);
    m_game.set_controller(m_controller);

    m_stack.add(m_welcome,    "welcome");
    m_stack.add(m_name,       "enter_name");
    m_stack.add(m_gender,     "select_gender");
    m_stack.add(m_difficulty, "select_difficulty");
    m_stack.add(m_game,       "playing");
    m_stack.set_transition_type(Gtk::StackTransitionType::SLIDE_LEFT_RIGHT);
    m_stack.set_transition_duration(200);

    set_child(m_stack);

    m_controller.signal_app_state_changed.connect(
        sigc::mem_fun(*this, &MainWindow::on_app_state_changed));
}

void MainWindow::on_app_state_changed()
{
    static const char* names[] = {
        "welcome", "enter_name", "select_gender", "select_difficulty", "playing"
    };
    int s = m_controller.appState();
    if (s >= 0 && s <= 4)
        m_stack.set_visible_child(Glib::ustring(names[s]));
}

void MainWindow::load_css()
{
    auto provider = Gtk::CssProvider::create();
    provider->load_from_data(kCss);
    Gtk::StyleProvider::add_provider_for_display(
        Gdk::Display::get_default(), provider,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}
