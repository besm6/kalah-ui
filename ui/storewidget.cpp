#include "storewidget.h"
#include <cmath>

StoreWidget::StoreWidget(const std::string& label)
    : m_label(label)
{
    set_size_request(70, 200);
    set_draw_func(sigc::mem_fun(*this, &StoreWidget::on_draw));
}

void StoreWidget::update(int stones)
{
    m_stones = stones;
    queue_draw();
}

void StoreWidget::on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int w, int h)
{
    double r = w / 2.0 - 2.0;  // half-width as corner radius → pill shape

    // Pill / tall oval shape via rounded rectangle
    cr->move_to(r + 2.0, 2.0);
    cr->arc(w - 2.0 - r, 2.0 + r, r, -M_PI / 2.0, 0);
    cr->arc(w - 2.0 - r, h - 2.0 - r, r, 0, M_PI / 2.0);
    cr->arc(2.0 + r, h - 2.0 - r, r, M_PI / 2.0, M_PI);
    cr->arc(2.0 + r, 2.0 + r, r, M_PI, 3 * M_PI / 2.0);
    cr->close_path();

    cr->set_source_rgb(0.627, 0.478, 0.298); // #a07a4c
    cr->fill_preserve();
    cr->set_source_rgb(0.353, 0.227, 0.102); // #5a3a1a
    cr->set_line_width(2.0);
    cr->stroke();

    double cx = w / 2.0;
    double cy = h / 2.0;

    cr->set_source_rgb(0.165, 0.102, 0.031); // #2a1a08
    cr->select_font_face("Sans", Cairo::ToyFontFace::Slant::NORMAL,
                          Cairo::ToyFontFace::Weight::NORMAL);
    cr->set_font_size(12.0);
    Cairo::TextExtents te;
    cr->get_text_extents(m_label, te);
    cr->move_to(cx - te.width / 2.0 - te.x_bearing, cy - 19.0);
    cr->show_text(m_label);

    cr->select_font_face("Sans", Cairo::ToyFontFace::Slant::NORMAL,
                          Cairo::ToyFontFace::Weight::BOLD);
    cr->set_font_size(29.0);
    auto num = std::to_string(m_stones);
    cr->get_text_extents(num, te);
    cr->move_to(cx - te.width / 2.0 - te.x_bearing,
                cy + te.height / 2.0 - te.y_bearing - te.height + 10.0);
    cr->show_text(num);
}
