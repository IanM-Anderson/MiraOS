#ifndef SYSTEM_H
#define SYSTEM_H

#include "Variables.h"
#include "Screen_manager.h"
#include "Input_handler.h"
#include <string>

class System
{
private:
    Task_slot active_slots[MAX_CONCURRENT_TASKS];

    Screen_manager *screen_manager = nullptr;
    Input_Handler *input_handler = nullptr;

    unsigned long current_system_time = 0;
    unsigned long last_minute_time = 0;
    bool needs_refresh = false;
    std::string target_element_for_partial = ""; 

    std::string wifi_ssid;
    std::string wifi_password;
    std::string web_app_url;
    int last_sync_day = -1;

    std::string clean_arguments(const std::string arguments);
    void dispatch_command(const std::string &command, const std::string arguments[], int num_arguments, int task_index);
    void parse_command(const std::string &action, int task_index);

    void send_telemetry();
    void handle_ota_mode();
    void perform_sync();
    
    // New helper to read the saved variables from Flash
    void load_config();

public:
    System();

    void init(std::string default_ssid, std::string default_pass, std::string default_url);
    void run_loop();

    void update(unsigned long system_time);
    void execute_script(const std::string &actions, int task_index);

    Input_Handler *get_input_handler() { return input_handler; }

    void switch_to_screen(const std::string &screen_name);
    void change_focused_element(const int direction);

    bool is_idle();
    unsigned long time_until_next_event();
};

#endif