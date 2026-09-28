#include "Input_handler.h"
#include "System.h"
#include <Arduino.h>

// Safe ESP8266 Pins
#define PIN_BTN1 D1
#define PIN_BTN2 D6

Input_Handler *global_input_handler = nullptr;

// ISR to instantly wake the CPU from light sleep when a button changes state
void IRAM_ATTR isr_wake_up() {
    if (global_input_handler) global_input_handler->hardware_wakeup_triggered = true;
}

Input_Handler::Input_Handler(System *system_ptr, Screen_manager *screen_mgr_ptr)
{
    sys = system_ptr;
    screen_manager = screen_mgr_ptr;
    queue_head = 0;
    queue_tail = 0;

    for (int i = 0; i < NUM_EVENTS; i++) {
        default_actions[i].event_id = (Input_Event)i;
        default_actions[i].event_name = event_names[i];
        default_actions[i].script = "";
    }

    global_input_handler = this;
    init_hardware_inputs();
}

void Input_Handler::init_hardware_inputs()
{
    pinMode(PIN_BTN1, INPUT_PULLUP);
    pinMode(PIN_BTN2, INPUT_PULLUP);

    // Attach hardware interrupts to wake the processor from sleep instantly
    attachInterrupt(digitalPinToInterrupt(PIN_BTN1), isr_wake_up, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN2), isr_wake_up, CHANGE);
}

void Input_Handler::set_default_action(Input_Event event, const std::string &script)
{
    if (event >= 0 && event < NUM_EVENTS) {
        default_actions[event].script = script;
    }
}

void Input_Handler::isr_enqueue_event(Input_Event event)
{
    int next_head = (queue_head + 1) % QUEUE_SIZE;
    if (next_head != queue_tail) {
        event_queue[queue_head] = event;
        queue_head = next_head;
    }
}

void check_button_state(uint8_t pin, unsigned long &press_time, bool &is_down, bool &long_fired, 
                        Input_Event short_ev, Input_Event long_ev, Input_Event hold_ev, Input_Handler *handler)
{
    int reading = digitalRead(pin);
    unsigned long now = millis();

    if (reading == LOW) 
    {
        if (!is_down) {
            is_down = true;
            press_time = now;
            long_fired = false;
        } else {
            if (!long_fired && (now - press_time >= 1500)) {
                handler->isr_enqueue_event(long_ev);
                long_fired = true;
            } else if (long_fired && (now - press_time >= 2500)) {
                if ((now - press_time) % 500 < 20) {
                    handler->isr_enqueue_event(hold_ev);
                }
            }
        }
    } 
    else 
    {
        if (is_down) {
            unsigned long duration = now - press_time;
            if (duration > 50 && !long_fired) {
                handler->isr_enqueue_event(short_ev);
            }
            is_down = false;
            long_fired = false;
        }
    }
}

void Input_Handler::process_events()
{
    check_button_state(PIN_BTN1, btn1_press_time, btn1_is_down, btn1_long_fired, BTN1, BTN1, BTN1, this);
    check_button_state(PIN_BTN2, btn2_press_time, btn2_is_down, btn2_long_fired, BTN2, BTN2, BTN2, this);

    while (queue_tail != queue_head) {
        Input_Event ev = event_queue[queue_tail];
        queue_tail = (queue_tail + 1) % QUEUE_SIZE;
        handle_single_event(ev);
    }
}

void Input_Handler::handle_single_event(Input_Event event)
{
    Screen *active_screen = screen_manager->get_current_screen();
    UI_element *focused_item = screen_manager->get_focused_element();
    std::string element_script = "";
    std::string screen_script = "";
    std::string default_script = default_actions[event].script;

    if (focused_item) element_script = focused_item->get_action_for_event(event);
    if (active_screen) screen_script = active_screen->get_action_for_event(event);

    if (element_script != "") sys->execute_script(element_script, -1);
    else if (screen_script != "") sys->execute_script(screen_script, -1);
    else if (default_script != "") sys->execute_script(default_script, -1);
}