#include "genderscreen.h"

GenderScreen::GenderScreen()
    : Gtk::Box(Gtk::Orientation::VERTICAL, 16)
    , m_male("Male")
    , m_female("Female")
    , m_other("Prefer not to say")
{
    set_halign(Gtk::Align::CENTER);
    set_valign(Gtk::Align::CENTER);
    set_expand(true);
    set_margin_start(120);
    set_margin_end(120);
    add_css_class("screen");
    add_css_class("screen-gender");

    m_label.set_text("Select your gender");
    m_label.add_css_class("subtitle");
    m_label.set_halign(Gtk::Align::CENTER);

    for (auto* btn : {&m_male, &m_female, &m_other}) {
        btn->add_css_class("btn-choice");
        btn->set_hexpand(true);
    }

    m_male.signal_clicked().connect([this]   { if (m_ctrl) m_ctrl->selectGender(1); });
    m_female.signal_clicked().connect([this] { if (m_ctrl) m_ctrl->selectGender(2); });
    m_other.signal_clicked().connect([this]  { if (m_ctrl) m_ctrl->selectGender(0); });

    append(m_label);
    append(m_male);
    append(m_female);
    append(m_other);
}

void GenderScreen::set_controller(GameController& c)
{
    m_ctrl = &c;
}
