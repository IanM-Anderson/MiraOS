#include "Variables.h"
#include <iostream>
#include <stdexcept>

// ---------------------------------------------------------
// Allocate memory for the global variables and hardware exactly once
// ---------------------------------------------------------
System_variable variables[MAX_NUM_VARIABLES];
int num_variables = 0;

// Instantiate the display driver here so it only exists in memory once
GxEPD2_4G_4G<GxEPD2_750_T7, MAX_HEIGHT(GxEPD2_750_T7)> display(GxEPD2_750_T7(CS_PIN, DC_PIN, RST_PIN, BUSY_PIN));

const std::string event_names[NUM_EVENTS] = {
    "dial_right",
    "dial_left",
    "dial_click",
    "btn1",
    "btn2",
    "on_exit",
    "on_enter",
    "on_focus",
    "off_focus",
    "on_minute"};

// ---------------------------------------------------------
// Function Implementations
// ---------------------------------------------------------

std::string get_variable_value(const std::string &name)
{
    std::string value = "";

    for (int i = 0; i < num_variables && value == ""; ++i)
    {
        if (variables[i].name == name)
        {
            value = variables[i].value;
        }
    }
    return value;
}

void set_variable(const std::string &name, const std::string &value)
{
    bool variable_exists = false;
    for (int i = 0; i < num_variables && !variable_exists; ++i)
    {
        if (variables[i].name == name)
        {
            variables[i].value = value;
            variable_exists = true;
        }
    }

    if (!variable_exists && num_variables < MAX_NUM_VARIABLES)
    {
        System_variable new_variable = {name, value};
        variables[num_variables] = new_variable;
        num_variables++;
    }
}

Input_Event parse_event_type(const std::string &event_str)
{
    for (int i = 0; i < NUM_EVENTS; ++i)
    {
        if (event_str == event_names[i])
        {
            return static_cast<Input_Event>(i);
        }
    }
    return NUM_EVENTS;
}