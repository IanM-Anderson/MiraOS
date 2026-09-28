#ifndef VARIABLES_H
#define VARIABLES_H

#include <string>
#include <iostream>
#include <stdexcept>
#include <cstdlib>

// Screen
#include <GxEPD2_4G_4G.h>
#include <Adafruit_GFX.h>
#include <Arduino.h>

// 16-bit Adafruit GFX Color Palette
#define COLOR_WHITE      0xFFFF
#define COLOR_LIGHT_GREY 0xC618
#define COLOR_DARK_GREY  0x7BEF
#define COLOR_BLACK      0x0000

// *****************************************************************************
//                                  SYSTEM
// *****************************************************************************
#define MAX_CONCURRENT_TASKS 10
#define MAX_NUM_VARIABLES 30

struct Task_slot {
    bool is_active = false;
    unsigned long delay_end_time = 0;
    std::string remaining_actions = "";

    void start() {
        is_active = true;
        delay_end_time = 0;
        remaining_actions = "";
    }
    void stop() {
        is_active = false;
        delay_end_time = 0;
        remaining_actions = "";
    }
};

struct System_variable {
    std::string name = "";
    std::string value = "";
};

extern System_variable variables[MAX_NUM_VARIABLES];
extern int num_variables;

std::string get_variable_value(const std::string &name);
void set_variable(const std::string &name, const std::string &value);

// *****************************************************************************
//                                 INPUT
// *****************************************************************************
enum Input_Event {
    DIAL_RIGHT = 0,
    DIAL_LEFT,
    DIAL_CLICK,
    BTN1,
    BTN2,
    ON_EXIT,
    ON_ENTER,
    ON_FOCUS,
    OFF_FOCUS,
    ON_MINUTE,
    NUM_EVENTS
};

extern const std::string event_names[NUM_EVENTS];

struct Default_Action {
    Input_Event event_id;
    std::string event_name;
    std::string script;
};

Input_Event parse_event_type(const std::string &event_str);

// *****************************************************************************
//                             EVENT MAPPING
// *****************************************************************************
#define MAX_NUM_MAPPINGS 5

struct Event_mapping {
    Input_Event event_type;
    std::string action;
};

// *****************************************************************************
//                             SCREEN
// *****************************************************************************
// ESP8266 Hardware SPI Pins
#define CS_PIN D8
#define DC_PIN D3
#define RST_PIN D4
#define BUSY_PIN D2
#define MAX_DISPLAY_BUFFER_SIZE 800
#define MAX_PARTIAL_UPDATES_BEFORE_FULL 5
#define MAX_FAST_UPDATES 15

#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

// MUST BE GxEPD2_4G_4G
extern GxEPD2_4G_4G<GxEPD2_750_T7, MAX_HEIGHT(GxEPD2_750_T7)> display;

#endif