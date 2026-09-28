#include "Rect_element.h"

Rect_element::Rect_element(std::string name, std::string x, std::string y, std::string z,
                           std::string width, std::string height, std::string visible,
                           bool is_focusable) : UI_element(name, x, y, z, width, height, visible, is_focusable)
{
    // do Nothing else
}

void Rect_element::draw_self()
{
    std::string dec_color = color;
    std::string dec_bcolor = boarder_color;
    std::string dec_thick = boarder_thickness;
    std::string dec_fill = fill;

    decode_string(&dec_color);
    decode_string(&dec_bcolor);
    decode_string(&dec_thick);
    decode_string(&dec_fill);

    bool is_filled = (dec_fill == "true" || dec_fill == "1");
    int border_w = 0;
    if (!dec_thick.empty())
        border_w = atoi(dec_thick.c_str());

    // Cleaned up: Using the inherited master parser
    uint16_t fill_col = parse_color(dec_color);
    uint16_t border_col = parse_color(dec_bcolor);

    if (is_filled)
    {
        display.fillRect(get_x(), get_y(), get_width(), get_height(), fill_col);
    }

    if (border_w > 0)
    {
        for (int i = 0; i < border_w; i++)
        {
            display.drawRect(get_x() + i, get_y() + i, get_width() - (i * 2), get_height() - (i * 2), border_col);
        }
    }
}