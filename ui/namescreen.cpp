#include "namescreen.h"

NameScreen::NameScreen()
    : Gtk::Box(Gtk::Orientation::VERTICAL, 24)
    , m_continue_btn("Continue")
{
    set_halign(Gtk::Align::CENTER);
    set_valign(Gtk::Align::CENTER);
    set_expand(true);
    set_margin_start(72);
    set_margin_end(72);
    add_css_class("screen");
    add_css_class("screen-name");

    m_label.set_text("Enter your name");
    m_label.add_css_class("subtitle");
    m_label.set_halign(Gtk::Align::CENTER);

    m_entry.set_placeholder_text("Your name");
    m_entry.set_max_length(24);
    m_entry.add_css_class("name-entry");
    m_entry.set_hexpand(true);

    m_continue_btn.add_css_class("btn-gold");
    m_continue_btn.set_sensitive(false);
    m_continue_btn.set_halign(Gtk::Align::CENTER);

    m_entry.signal_changed().connect([this] {
        m_continue_btn.set_sensitive(!m_entry.get_text().empty());
    });
    m_entry.signal_activate().connect(sigc::mem_fun(*this, &NameScreen::on_submit));
    m_continue_btn.signal_clicked().connect(sigc::mem_fun(*this, &NameScreen::on_submit));

    append(m_label);
    append(m_entry);
    append(m_continue_btn);
}

void NameScreen::set_controller(GameController& c)
{
    m_ctrl = &c;
}

void NameScreen::on_submit()
{
    if (!m_ctrl || m_entry.get_text().empty()) return;
    m_ctrl->submitName(std::string(m_entry.get_text()));
}
