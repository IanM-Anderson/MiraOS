#include "Spinner_element.h"
#include "Variables.h"

Spinner_element::Spinner_element(std::string name, std::string x, std::string y, std::string z,
                                 std::string width, std::string height, std::string visible,
                                 bool is_focusable)
    : UI_element(name, x, y, z, width, height, visible, is_focusable) 
{
    init_charset();
}

void Spinner_element::init_charset() {
    std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 !@#$%&*-_+.";
    for (char c : chars) {
        charset.push_back(std::string(1, c));
    }
    charset.push_back("DEL");
    charset.push_back("OK");
}

void Spinner_element::set_spinner_config(std::string target_var, std::string action, std::string initial_text) {
    target_variable = target_var;
    submit_action = action;
    current_text = initial_text;
    decode_string(&current_text); 
}

void Spinner_element::scroll_next() {
    char_index = (char_index + 1) % charset.size();
}

void Spinner_element::scroll_prev() {
    if (char_index == 0) {
        char_index = charset.size() - 1;
    } else {
        char_index--;
    }
}

std::string Spinner_element::click() {
    std::string selected = charset[char_index];
    if (selected == "DEL") {
        if (!current_text.empty()) current_text.pop_back();
    } 
    else if (selected == "OK") {
        if (!target_variable.empty()) set_variable(target_variable, current_text);
        return submit_action;
    } 
    else {
        current_text += selected;
    }
    return "";
}

void Spinner_element::draw_self() {
    uint16_t primary_col = parse_color(color);
    uint16_t border_col = parse_color(boarder_color);
    uint16_t highlight_col = border_col;
    uint16_t highlight_text = primary_col;

    int bx = get_x(), by = get_y();
    int w = get_width(), h = get_height();

    display.fillRect(bx, by, w, h, primary_col);
    display.drawRect(bx, by, w, h, border_col);
    display.setTextSize(3);

    std::string display_text = current_text;
    if (display_text.length() > 14) {
        display_text = "> " + display_text.substr(display_text.length() - 12);
    }
    
    display.setTextColor(border_col);
    display.setCursor(bx + 15, by + (h / 2) - 10);
    display.print(display_text.c_str());

    int16_t tx, ty; uint16_t tw, th;
    display.getTextBounds(display_text.c_str(), bx + 15, by + (h / 2) - 10, &tx, &ty, &tw, &th);

    int spin_x = bx + 15 + tw + 10;
    int spin_w = (charset[char_index] == "DEL" || charset[char_index] == "OK") ? 65 : 30;
    
    bool is_focused = get_focused();
    display.fillRect(spin_x, by + 10, spin_w, h - 20, is_focused ? highlight_col : primary_col);
    display.drawRect(spin_x, by + 10, spin_w, h - 20, border_col);

    display.setTextColor(is_focused ? highlight_text : border_col);
    display.setCursor(spin_x + 6, by + (h / 2) - 10);
    display.print(charset[char_index].c_str());
}