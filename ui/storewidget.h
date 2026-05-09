#pragma once

#include <gtkmm.h>
#include <string>

class StoreWidget : public Gtk::DrawingArea {
public:
    explicit StoreWidget(const std::string& label);

    void update(int stones);

private:
    std::string m_label;
    int         m_stones = 0;

    void on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int w, int h);
};
