#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "Variables.h"
#include "Screen_manager.h"

class System; 

class Input_Handler {
private:
    System *sys = nullptr;
    Screen_manager *screen_manager = nullptr;
    Default_Action default_actions[NUM_EVENTS];

    static const int QUEUE_SIZE = 16;
    volatile Input_Event event_queue[QUEUE_SIZE];
    volatile int queue_head = 0;
    volatile int queue_tail = 0;

    void handle_single_event(Input_Event event);

    unsigned long btn1_press_time = 0;
    bool btn1_is_down = false;
    bool btn1_long_fired = false;

    unsigned long btn2_press_time = 0;
    bool btn2_is_down = false;
    bool btn2_long_fired = false;

    unsigned long dial_sw_press_time = 0;
    bool dial_sw_is_down = false;
    bool dial_sw_long_fired = false;

public:
    Input_Handler(System *system_ptr, Screen_manager *screen_mgr_ptr);
    void set_default_action(Input_Event event, const std::string &script);
    void isr_enqueue_event(Input_Event event);
    void process_events();
    void init_hardware_inputs();

    // --- Sleep Management Helpers ---
    bool has_events() const { return queue_head != queue_tail; }
    bool is_any_input_active() const { return btn1_is_down || btn2_is_down || dial_sw_is_down; }
    
    volatile bool hardware_wakeup_triggered = false;
    void clear_wakeup_flag() { hardware_wakeup_triggered = false; }
    bool check_wakeup_flag() const { return hardware_wakeup_triggered; }
};

#endif