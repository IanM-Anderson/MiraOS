#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "Variables.h"
#include "Screen.h"
#include "Sheet_loader.h"

class Screen_manager
{
private:
    Screen *current_screen = nullptr;
    int partial_update_counter = 0;

    void draw_screen(Screen *screen);

public:
    Screen_manager();
    Screen *get_current_screen();
    UI_element *get_focused_element();

    void add_element_to_screen(UI_element *new_element);

    bool switch_screen(const std::string &screen_name);
    void refresh_screen();
    void refresh_element(const std::string &element_name);

    void invert_screen_colors();

    void set_element_visibility(const std::string &element_name, std::string visible);
    void toggle_element_visibility(const std::string &element_name);
    void set_element_text(const std::string &element_name, const std::string &text);
    void update_element_position(const std::string &element_name, std::string x, std::string y, std::string z);
};

#endif