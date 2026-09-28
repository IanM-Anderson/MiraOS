#include "Circle_element.h"

Circle_element::Circle_element(std::string name, std::string x, std::string y, std::string z,
                               std::string width, std::string height, std::string visible,
                               bool is_focusable)
    : UI_element(name, x, y, z, width, height, visible, is_focusable) {}

void Circle_element::draw_self()
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

    int radius = get_width() / 2;
    int center_x = get_x() + radius;
    int center_y = get_y() + (get_height() / 2);

    if (is_filled)
    {
        display.fillCircle(center_x, center_y, radius, fill_col);
    }

    if (border_w > 0)
    {
        for (int i = 0; i < border_w; i++)
        {
            display.drawCircle(center_x, center_y, radius - i, border_col);
        }
    }
}