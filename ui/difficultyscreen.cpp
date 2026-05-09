#include "difficultyscreen.h"

DifficultyScreen::DifficultyScreen()
    : Gtk::Box(Gtk::Orientation::VERTICAL, 12)
{
    set_halign(Gtk::Align::CENTER);
    set_valign(Gtk::Align::CENTER);
    set_expand(true);
    set_margin_start(120);
    set_margin_end(120);
    add_css_class("screen");
    add_css_class("screen-difficulty");

    m_label.set_text("Select difficulty");
    m_label.add_css_class("subtitle");
    m_label.set_halign(Gtk::Align::CENTER);
    append(m_label);

    make_level(m_levels[0], "Юноша",    "Novice · great for beginners",          1);
    make_level(m_levels[1], "Кандидат", "Candidate · a fair challenge",           2);
    make_level(m_levels[2], "Участник", "Participant · for experienced players",  3);
    make_level(m_levels[3], "Эфенди",   "Master · maximum difficulty",            4);
}

void DifficultyScreen::make_level(DiffButton& db, const char* title,
                                   const char* subtitle, int value)
{
    db.title.set_text(title);
    db.title.add_css_class("difficulty-title");
    db.title.set_halign(Gtk::Align::CENTER);

    db.sub.set_text(subtitle);
    db.sub.add_css_class("difficulty-sub");
    db.sub.set_halign(Gtk::Align::CENTER);

    db.inner.append(db.title);
    db.inner.append(db.sub);

    db.btn.set_child(db.inner);
    db.btn.add_css_class("btn-choice");
    db.btn.set_hexpand(true);
    db.btn.signal_clicked().connect([this, value] {
        if (m_ctrl) m_ctrl->selectLevel(value);
    });

    append(db.btn);
}

void DifficultyScreen::set_controller(GameController& c)
{
    m_ctrl = &c;
}
