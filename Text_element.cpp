#include "Text_element.h"

Text_element::Text_element(std::string name, std::string x, std::string y, std::string z,
                           std::string width, std::string height, std::string visible,
                           bool is_focusable)
    : UI_element(name, x, y, z, width, height, visible, is_focusable) {}

void Text_element::set_text_properties(std::string t, std::string ts, std::string tc, std::string ta)
{
    text = t;
    text_size = ts;
    text_color = tc;
    text_align = ta;
}

void Text_element::set_text(std::string new_text)
{
    text = new_text;
}

void Text_element::draw_self()
{
    std::string dec_text = text;
    decode_string(&dec_text);

    // Replace 2-byte UTF-8 degree symbol with the Adafruit GFX 1-byte degree char (\xF7)
    size_t pos = 0;
    while ((pos = dec_text.find("\xC2\xB0", pos)) != std::string::npos) 
    {
        dec_text.replace(pos, 2, "\xF7");
        pos += 1;
    }

    // Cleaned up: Using the inherited master parser for text
    display.setTextColor(parse_color(text_color));

    int size = 2;
    if (atoi(text_size.c_str()) != 0) {
      size = atoi(text_size.c_str());
    }
    display.setTextSize(size);

    int cursor_x = get_x();
    int cursor_y = get_y();

    if (text_align == "left")
    {
        display.setCursor(cursor_x, cursor_y);
        display.print(dec_text.c_str());
    }
    else if (text_align == "center")
    {
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(dec_text.c_str(), cursor_x, cursor_y, &x1, &y1, &w, &h);
        int centered_x = cursor_x + (get_width() - w) / 2;
        display.setCursor(centered_x, cursor_y);
        display.print(dec_text.c_str());
    }
    else if (text_align == "right")
    {
        int16_t x1, y1;
        uint16_t w, h;
        display.getTextBounds(dec_text.c_str(), cursor_x, cursor_y, &x1, &y1, &w, &h);
        int right_x = cursor_x + get_width() - w;
        display.setCursor(right_x, cursor_y);
        display.print(dec_text.c_str());
    }
}