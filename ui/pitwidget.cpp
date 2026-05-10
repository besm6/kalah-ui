#include "pitwidget.h"
#include <cmath>

PitWidget::PitWidget(int board_index, bool is_user_pit)
    : m_board_index(board_index), m_is_user_pit(is_user_pit)
{
    set_size_request(80, 80);
    set_draw_func(sigc::mem_fun(*this, &PitWidget::on_draw));

    if (is_user_pit) {
        auto click = Gtk::GestureClick::create();
        click->signal_pressed().connect([this](int, double, double) {
            if (m_playable && m_stones > 0) {
                m_pressed = true;
                queue_draw();
                signal_tapped.emit(m_board_index);
            }
        });
        click->signal_released().connect([this](int, double, double) {
            m_pressed = false;
            queue_draw();
        });
        add_controller(click);
    }
}

void PitWidget::update(int stones, bool playable)
{
    m_stones   = stones;
    m_playable = playable;
    queue_draw();
}

void PitWidget::on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int w, int h)
{
    double cx = w / 2.0;
    double cy = h / 2.0;
    double r  = std::min(cx, cy) - 2.0;

    // Fill circle
    cr->arc(cx, cy, r, 0, 2 * M_PI);
    if (m_pressed)
        cr->set_source_rgb(1.0, 0.878, 0.627);   // #ffe0a0
    else if (m_playable)
        cr->set_source_rgb(0.847, 0.706, 0.541); // #d8b48a
    else
        cr->set_source_rgb(0.722, 0.580, 0.416); // #b8946a
    cr->fill_preserve();

    // Border
    cr->set_source_rgb(0.353, 0.227, 0.102); // #5a3a1a
    cr->set_line_width(2.0);
    cr->stroke();

    // Stone count text
    cr->set_source_rgb(0.165, 0.102, 0.031); // #2a1a08
    cr->select_font_face("Sans", Cairo::ToyFontFace::Slant::NORMAL,
                          Cairo::ToyFontFace::Weight::BOLD);
    cr->set_font_size(25.0);
    auto txt = std::to_string(m_stones);
    Cairo::TextExtents te;
    cr->get_text_extents(txt, te);
    cr->move_to(cx - te.width / 2.0 - te.x_bearing,
                cy - te.height / 2.0 - te.y_bearing);
    cr->show_text(txt);
}
