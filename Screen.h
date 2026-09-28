#ifndef SCREEN_H
#define SCREEN_H

#include "Variables.h"
#include "UI_element.h"

#define MAX_NUM_ELEMENTS 20

class Screen
{
private:
    std::string name = "";
    UI_element *elements[MAX_NUM_ELEMENTS];
    Event_mapping events[MAX_NUM_MAPPINGS]; // Array to hold event mappings
    int num_events = 0;

    int num_elements = 0;
    int focus_index = -1;

public:
    Screen();
    Screen(std::string name);
    ~Screen();

    void pass_system_to_elements(System *sys_ptr);

    std::string get_name() const { return name; }
    void set_name(const std::string &new_name) { name = new_name; }

    void add_element(UI_element *new_element);
    void add_event_mapping(Input_Event event, const std::string &action);
    std::string get_action_for_event(Input_Event event);

    void clear(); // TODO
    void draw_screen();
    void draw_screen_partial(int x, int y, int w, int h);
    UI_element *get_element_by_name(const std::string &element_name);
    std::string get_name();

    UI_element *get_focused_element();
    UI_element *focus_next();
    UI_element *focus_previous();

    void set_element_visibility(const std::string &element_name, std::string visible);
    void toggle_element_visibility(const std::string &element_name);
    void set_element_text(const std::string &element_name, const std::string &text); // TODO
    void invert_screen_colors();

    void sort_elements();
    void update_element_position(const std::string &element_name, std::string x, std::string y, std::string z);
};

#endif