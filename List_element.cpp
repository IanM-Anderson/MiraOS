#include "List_element.h"
#include <cstdlib>

List_element::List_element(std::string name, std::string x, std::string y, std::string z,
                           std::string width, std::string height, std::string visible,
                           bool is_focusable)
    : UI_element(name, x, y, z, width, height, visible, is_focusable) {}

void List_element::set_list_config(int per_page) {
    if (per_page > 0) items_per_page = per_page;
}

void List_element::add_item(std::string label, std::string action) {
    labels.push_back(label);
    actions.push_back(action);
}

bool List_element::scroll_down() {
    if (labels.empty()) return false;
    int max_absolute = labels.size() - 1;
    int current_absolute = (current_page * items_per_page) + selected_index;

    if (current_absolute < max_absolute) {
        selected_index++;
        if (selected_index >= items_per_page) {
            selected_index = 0;
            current_page++;
        }
        return true;
    }
    return false; // Escapes boundaries
}

bool List_element::scroll_up() {
    if (labels.empty()) return false;

    if (selected_index > 0) {
        selected_index--;
        return true;
    } 
    else if (current_page > 0) {
        current_page--;
        selected_index = items_per_page - 1;
        return true;
    }
    return false; // Escapes boundaries
}

std::string List_element::click() {
    if (labels.empty()) return "";
    int absolute_index = (current_page * items_per_page) + selected_index;
    if (absolute_index >= 0 && absolute_index < labels.size()) {
        return actions[absolute_index];
    }
    return "";
}

void List_element::draw_self() {
    uint16_t primary_col = parse_color(color);
    uint16_t border_col = parse_color(boarder_color);
    uint16_t highlight_col = border_col; 
    uint16_t highlight_text = primary_col; 

    int bx = get_x(), by = get_y();
    int w = get_width(), h = get_height();

    if (items_per_page <= 0) return;
    int item_h = h / items_per_page;

    display.setTextSize(2);
    for (int i = 0; i < items_per_page; i++) {
        int absolute_index = (current_page * items_per_page) + i;
        int item_y = by + (i * item_h);

        bool is_focused = (get_focused() && selected_index == i);
        uint16_t bg = is_focused ? highlight_col : primary_col;
        uint16_t fg = is_focused ? highlight_text : border_col;

        display.fillRect(bx, item_y, w, item_h, bg);
        display.drawRect(bx, item_y, w, item_h, border_col);

        if (absolute_index < labels.size()) {
            display.setTextColor(fg);
            display.setCursor(bx + 10, item_y + (item_h / 2) - 7);
            display.print(labels[absolute_index].c_str());
        }
    }
}