#include "UI_element.h"
#include "System.h"

UI_element::UI_element()
{
    UI_element::name = "";
    UI_element::x = "";
    UI_element::y = "";
    UI_element::z = "";
    UI_element::width = "";
    UI_element::height = "";
    UI_element::visible = false;
    UI_element::is_focusable = false;
    UI_element::is_focused = false;
    UI_element::system = nullptr;
    num_events = 0;
}

UI_element::UI_element(std::string name, std::string x, std::string y,
                       std::string z, std::string width, std::string height,
                       std::string visible, bool is_focusable)
{
    UI_element::name = name;
    UI_element::x = x;
    UI_element::y = y;
    UI_element::z = z;
    UI_element::width = width;
    UI_element::height = height;
    UI_element::visible = visible;
    UI_element::is_focusable = is_focusable;
    UI_element::is_focused = false;
    UI_element::system = nullptr;
    num_events = 0;
}

void UI_element::decode_string(std::string *str)
{
    int start_index = 0;
    int variable_start_index = 0;
    int variable_end_index = str->length();
    std::string variable;

    while (start_index < str->length())
    {
        variable_start_index = str->find('{', start_index);
        variable_end_index = str->find('}', start_index);

        if (variable_start_index != std::string::npos && variable_end_index != std::string::npos)
        {
            variable = str->substr(variable_start_index + 1, variable_end_index - variable_start_index - 1);

            if (system != nullptr) {
                variable = get_variable_value(variable);
            } else {
                variable = "";
            }

            int old_length = (variable_end_index - variable_start_index) + 1;
            str->replace(variable_start_index, old_length, variable);

            start_index = variable_end_index + 1;
        }
        else
        {
            start_index = str->length();
        }
    }
}

std::string UI_element::get_name() { return UI_element::name; }

int UI_element::get_x() {
    int value = 0;
    std::string new_x = x;
    decode_string(&new_x);
    if (atoi(new_x.c_str()) != 0) value = atoi(new_x.c_str());
    return value;
}

int UI_element::get_y() {
    int value = 0;
    std::string new_y = y;
    decode_string(&new_y);
    if (atoi(new_y.c_str()) != 0) value = atoi(new_y.c_str());
    return value;
}

int UI_element::get_z() {
    int value = 0;
    std::string new_z = z;
    decode_string(&new_z);
    if (atoi(new_z.c_str()) != 0) value = atoi(new_z.c_str());
    return value;
}

int UI_element::get_width() {
    int value = 0;
    std::string new_w = width;
    decode_string(&new_w);
    if (atoi(new_w.c_str()) != 0) value = atoi(new_w.c_str());
    return value;
}

int UI_element::get_height() {
    int value = 0;
    std::string new_h = height;
    decode_string(&new_h);
    if (atoi(new_h.c_str()) != 0) value = atoi(new_h.c_str());
    return value;
}

bool UI_element::is_visible() {
    bool value_bool = 0;
    std::string new_v = visible;
    decode_string(&new_v);
    if (atoi(new_v.c_str()) != 0) {
        if (atoi(new_v.c_str()) == 1) value_bool = 1;
    }
    return value_bool;
}

bool UI_element::get_focusable() { return UI_element::is_focusable; }
bool UI_element::get_focused() { return UI_element::is_focused; }

void UI_element::set_visible(std::string visible) { UI_element::visible = visible; }
void UI_element::toggle_visible() { UI_element::visible = is_visible() ? "0" : "1"; }
void UI_element::set_position(std::string x, std::string y, std::string z) {
    UI_element::x = x; UI_element::y = y; UI_element::z = z;
}
void UI_element::set_dimentions(std::string width, std::string height) {
    UI_element::width = width; UI_element::height = height;
}
void UI_element::set_focusable(bool focusable) { UI_element::is_focusable = focusable; }
void UI_element::set_focused(bool focused) { UI_element::is_focused = focused; }
void UI_element::set_styling(std::string c, std::string bc, std::string bt, std::string f) {
    color = c; boarder_color = bc; boarder_thickness = bt; fill = f;
}

void UI_element::draw() {
    if (is_visible()) draw_self();
}

uint16_t UI_element::parse_color(std::string color_str)
{
    // Grayscale mapping for GxEPD2_4G
    if (color_str == "1" || color_str == "true" || color_str == "white") return COLOR_WHITE;
    if (color_str == "2" || color_str == "light_grey") return COLOR_LIGHT_GREY;
    if (color_str == "3" || color_str == "dark_grey") return COLOR_DARK_GREY;
    return COLOR_BLACK; 
}

void UI_element::invert_colors()
{
    if (color == "0" || color == "false" || color.empty()) color = "1";
    else if (color == "1" || color == "true") color = "0";
    else if (color == "2") color = "3"; // Swap light to dark
    else if (color == "3") color = "2"; // Swap dark to light

    if (boarder_color == "0" || boarder_color == "false" || boarder_color.empty()) boarder_color = "1";
    else if (boarder_color == "1" || boarder_color == "true") boarder_color = "0";
    else if (boarder_color == "2") boarder_color = "3";
    else if (boarder_color == "3") boarder_color = "2";

    if (text_color == "0" || text_color == "false" || text_color.empty()) text_color = "1";
    else if (text_color == "1" || text_color == "true") text_color = "0";
    else if (text_color == "2") text_color = "3";
    else if (text_color == "3") text_color = "2";
}

void UI_element::add_event_mapping(Input_Event event, const std::string &action)
{
    if (num_events < sizeof(events) / sizeof(events[0])) {
        events[num_events].event_type = event;
        events[num_events].action = action;
        num_events++;
    }
}

std::string UI_element::get_action_for_event(Input_Event event)
{
    for (int i = 0; i < num_events; ++i) {
        if (events[i].event_type == event) return events[i].action;
    }
    return "";
}

void UI_element::on_focus()
{
    std::string action = get_action_for_event(ON_FOCUS);
    if (action != "" && system != nullptr) system->execute_script(action, -1);
    else invert_colors();
}

void UI_element::off_focus()
{
    std::string action = get_action_for_event(OFF_FOCUS);
    if (action != "" && system != nullptr) system->execute_script(action, -1);
    else invert_colors();
}