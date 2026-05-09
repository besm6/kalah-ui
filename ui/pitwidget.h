#pragma once

#include <gtkmm.h>
#include <sigc++/sigc++.h>

class PitWidget : public Gtk::DrawingArea {
public:
    PitWidget(int board_index, bool is_user_pit);

    void update(int stones, bool playable);

    sigc::signal<void(int)> signal_tapped;

private:
    int  m_board_index;
    bool m_is_user_pit;
    int  m_stones   = 0;
    bool m_playable = false;

    void on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int w, int h);
};
