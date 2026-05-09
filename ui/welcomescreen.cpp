#include "welcomescreen.h"

WelcomeScreen::WelcomeScreen()
    : Gtk::Box(Gtk::Orientation::VERTICAL, 24)
{
    set_halign(Gtk::Align::CENTER);
    set_valign(Gtk::Align::CENTER);
    set_expand(true);
    add_css_class("screen");

    m_title.set_text("Kalah");
    m_title.add_css_class("title");
    m_title.set_halign(Gtk::Align::CENTER);

    m_subtitle.set_text("A classic strategy game of skill and planning");
    m_subtitle.add_css_class("subtitle");
    m_subtitle.set_halign(Gtk::Align::CENTER);

    m_tap_hint.set_text("Tap anywhere to begin");
    m_tap_hint.add_css_class("tap-hint");
    m_tap_hint.set_halign(Gtk::Align::CENTER);

    append(m_title);
    append(m_subtitle);
    append(m_tap_hint);

    auto click = Gtk::GestureClick::create();
    click->signal_pressed().connect([this](int, double, double) {
        if (m_ctrl) m_ctrl->proceedFromWelcome();
    });
    add_controller(click);
}

void WelcomeScreen::set_controller(GameController& c)
{
    m_ctrl = &c;
}
