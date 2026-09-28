#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H

#include "Variables.h"

// Tell the compiler this display object exists in your main.cpp file
extern GxEPD2_4G_4G<GxEPD2_750_T7, MAX_HEIGHT(GxEPD2_750_T7)> display;

class System;

class UI_element
{
private:
    std::string name = "";
    std::string x = "0", y = "0", z = "0";
    std::string width = "0";
    std::string height = "0";
    std::string visible = "true";
    bool is_focusable = false;
    bool is_focused = false;

    System *system = nullptr; // pointer to the main system to deal with variables

    Event_mapping events[MAX_NUM_MAPPINGS]; // Array to hold event mappings
    int num_events = 0;

    // A virtual function where the children must be able to draw themselves
    virtual void draw_self() = 0;

protected:
    std::string color = "1";         // Default to Binary White
    std::string boarder_color = "0"; // Default to Binary Black
    std::string text_color = "0";    // Default to Binary Black
    std::string boarder_thickness = "0";
    std::string fill = "false";
    bool partial_refresh_preferred = false;
    bool fast_refresh_preferred = false;

    // HELPER FUNCTIONS
    uint16_t parse_color(std::string color_str);
    void decode_string(std::string *str);

public:
    UI_element();
    UI_element(std::string name, std::string x, std::string y, std::string z,
               std::string width, std::string height, std::string visible,
               bool is_focusable);

    virtual ~UI_element() = default;

    // GETTERS
    std::string get_name();
    int get_x();
    int get_y();
    int get_z();
    int get_width();
    int get_height();
    bool is_visible();
    bool get_focusable();
    bool get_focused();
    bool get_partial_refresh() const { return partial_refresh_preferred; }
    bool get_fast_refresh() const { return fast_refresh_preferred; }

    // SETTERS
    void set_name(std::string new_name) { name = new_name; };
    void set_visible(std::string visible);
    void toggle_visible();
    void set_position(std::string x, std::string y, std::string z);
    void set_dimentions(std::string width, std::string height);
    void set_focusable(bool focusable);
    void set_focused(bool focused);
    void set_system(System *sys_ptr) { system = sys_ptr; }

    void set_styling(std::string c, std::string bc, std::string bt, std::string f);
    virtual void set_text(std::string new_text) {} // Does nothing by default
    void set_partial_refresh(bool val) { partial_refresh_preferred = val; }
    void set_fast_refresh(bool val) { fast_refresh_preferred = val; }

    void draw();
    void invert_colors();

    // ACTION/FOCUS EVENTS
    void add_event_mapping(Input_Event event, const std::string &action);
    std::string get_action_for_event(Input_Event event);
    void on_focus();
    void off_focus();
};

#endif