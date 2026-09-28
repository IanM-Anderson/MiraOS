#include "System.h"
#include "Network.h"
#include "Sheet_loader.h"
#include "Storage.h"
#include "ArduinoJson-v7.4.3.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <LittleFS.h>
#include <cstdlib>
#include <time.h>

System::System()
{
    screen_manager = new Screen_manager();
    input_handler = new Input_Handler(this, screen_manager);

    time_t now = time(nullptr);
    struct tm tstruct;
    tstruct = *localtime(&now);
    
    last_minute_time = millis() - (tstruct.tm_sec * 1000);
}

void System::load_config() 
{
    File file = Storage::open_file_read("/config.json");
    if (!file) return;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    Storage::close_file(file);

    if (!error && doc.is<JsonObject>()) {
        JsonObject root = doc.as<JsonObject>();
        for (JsonPair kv : root) {
            set_variable(kv.key().c_str(), kv.value().as<std::string>());
        }
        Serial.println("[System] Successfully loaded configuration variables from LittleFS.");
    }
}

void System::init(std::string default_ssid, std::string default_pass, std::string default_url) 
{
    wifi_ssid = default_ssid;
    wifi_password = default_pass;
    web_app_url = default_url;

    set_variable("wifi_ssid", wifi_ssid);
    set_variable("wifi_pass", wifi_password);

    LittleFS.begin();
    display.init(115200);
    pinMode(D1, INPUT_PULLUP);

    // Overwrite the defaults with any saved variables from flash memory before proceeding
    load_config();

    perform_sync();
}

void System::perform_sync() 
{
    Serial.println("\n[System] Starting Sync...");

    // Check if the user updated the Wi-Fi credentials via the UI before connecting
    std::string current_ssid = get_variable_value("wifi_ssid");
    std::string current_pass = get_variable_value("wifi_pass");
    if (!current_ssid.empty()) wifi_ssid = current_ssid;
    if (!current_pass.empty()) wifi_password = current_pass;

    Network::connect(wifi_ssid.c_str(), wifi_password.c_str());

    // Check OTA Boot Trap
    if (digitalRead(D1) == LOW) {
        handle_ota_mode();
    }

    send_telemetry();
    Sheet_loader::load_json(web_app_url);
    Network::disconnect();

    switch_to_screen("Home");
}

void System::handle_ota_mode() 
{
    Serial.println("\n[SYSTEM] OTA Update Mode Active.");
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(COLOR_WHITE);
        display.setTextColor(COLOR_BLACK);
        display.setTextSize(3);
        display.setCursor(50, 50);
        display.print("OTA MODE ACTIVE");
        display.setCursor(50, 100);
        display.print(WiFi.localIP().toString().c_str());
    } while (display.nextPage());
    display.hibernate();

    ArduinoOTA.begin();
    while (true) {
        ArduinoOTA.handle();
        delay(10); // Infinite trap loop to allow OTA flashing
    }
}

void System::send_telemetry() 
{
    Serial.println("Sending Telemetry...");
    String telemetry_payload = "{\"vcc\":" + String(ESP.getVcc()) + 
                               ",\"rssi\":" + String(WiFi.RSSI()) + 
                               ",\"heap\":" + String(ESP.getFreeHeap()) + "}";
    Network::POST(web_app_url, telemetry_payload.c_str());
}

void System::run_loop() 
{
    update(millis());

    if (is_idle()) {
        unsigned long sleep_time = time_until_next_event();
        unsigned long start_sleep = millis();
        while (millis() - start_sleep < sleep_time) {
            if (!is_idle()) break;
            delay(20); 
        }
    }

    // Daily 3 AM Sync Scheduler
    time_t now = time(nullptr);
    struct tm tstruct;
    tstruct = *localtime(&now);
    
    if (tstruct.tm_hour == 3 && tstruct.tm_min == 0 && tstruct.tm_mday != last_sync_day) {
        last_sync_day = tstruct.tm_mday;
        perform_sync();
    }
}

// *****************************************************************************
//                          SYSTEM SCRIPTING FUNCTIONS
// *****************************************************************************

/**
 * This is our main system loop. It will be called repeatedly and will be
 * passed in the current system time. It is used to check all time related
 * events, include delays, timers, ect.
 *
 * @param system_time: the time of the internal system clock
 */
void System::update(unsigned long system_time)
{
    current_system_time = system_time;

    // -------------------- Update Clock ---------------------------------------
    if (current_system_time - last_minute_time >= 60000)
    {
        last_minute_time = current_system_time;
        input_handler->isr_enqueue_event(ON_MINUTE);
    }

    // -------------------- Delay Logic ----------------------------------------
    // Loop through all task slots to check on delayed scripts
    for (int i = 0; i < MAX_CONCURRENT_TASKS; ++i)
    {
        // if there is a delay in a task
        if (active_slots[i].is_active && active_slots[i].delay_end_time > 0)
        {
            // if the delay should be ended
            if (current_system_time >= active_slots[i].delay_end_time)
            {
                // Clear the delay timer flag for this slot before resuming
                active_slots[i].delay_end_time = 0;

                std::string script_to_resume = active_slots[i].remaining_actions;
                active_slots[i].remaining_actions = "";

                // Resume the script right inside its own isolated task slot
                execute_script(script_to_resume, i);
            }
        }
    }

    // -------------------- Input Logic ----------------------------------------
    input_handler->process_events();

    // -------------------- Render Logic ---------------------------------------
    // Only push pixels to the screen once per loop, after all scripts finish
    if (needs_refresh)
    {
        needs_refresh = false;
        // check if there is a specific element
        if (!target_element_for_partial.empty())
        {
            Serial.printf("Refreshing Element\n");
            screen_manager->refresh_element(target_element_for_partial);
            target_element_for_partial = "";
        }
        else 
        {
            screen_manager->refresh_screen();
        }
    }
}

/**
 * Goes through the list of possible commands and sends the action to the
 * appropriate handler.
 *
 * @param command: The command to be executed.
 * @param arguments: An array of arguments for the command.
 * @param num_arguments: The number of arguments in the arguments array.
 * @param task_slot_index: The index of the Task_slot we are using
 */
void System::dispatch_command(const std::string &command, const std::string arguments[], int num_arguments, int task_slot_index)
{
    if (command == "goto")
    {
        if (num_arguments == 1)
        {
            std::string target_screen_name = arguments[0];
            if (screen_manager != nullptr)
            {
                switch_to_screen(target_screen_name);
            }
        }
    }
    else if (command == "show")
    {
        if (num_arguments == 1)
        {
            screen_manager->set_element_visibility(arguments[0], "1");
            needs_refresh = true;
            target_element_for_partial = arguments[0];
        }
    }
    else if (command == "hide")
    {
        if (num_arguments == 1)
        {
            screen_manager->set_element_visibility(arguments[0], "0");
            needs_refresh = true;
            target_element_for_partial = arguments[0];
        }
    }
    else if (command == "toggle")
    {
        if (num_arguments == 1)
        {
            screen_manager->toggle_element_visibility(arguments[0]);
            needs_refresh = true;
            target_element_for_partial = arguments[0];
        }
    }
    else if (command == "settext")
    {
        if (num_arguments == 2)
        {
            screen_manager->set_element_text(arguments[0], arguments[1]);
            needs_refresh = true;
            target_element_for_partial = arguments[0];
        }
    }
    else if (command == "set_position")
    {
        // 4 arguments -> set_position:element_name:x:y:z
        if (num_arguments == 4)
        {
            screen_manager->update_element_position(arguments[0], arguments[1], arguments[2], arguments[3]);
            needs_refresh = true;
            target_element_for_partial = arguments[0];
        }
    }
    else if (command == "refresh")
    {
        if (num_arguments == 0)
        {
            needs_refresh = true;
            target_element_for_partial = "";
        }
    }
    else if (command == "invert")
    {
        if (num_arguments == 0)
        {
            screen_manager->invert_screen_colors();
            needs_refresh = true;
            target_element_for_partial = "";
        }
    }
    else if (command == "setvar")
    {
        if (num_arguments == 2)
        {
            set_variable(arguments[0], arguments[1]);
            needs_refresh = true;
            target_element_for_partial = ""; // NEED BETTER WAY
        }
    }
    else if (command == "ifvar")
    {
        if (num_arguments == 4)
        {
            std::string current_val = get_variable_value(arguments[0]);
            if (current_val == arguments[1])
            {
                execute_script(arguments[2], task_slot_index);
            }
            else
            {
                execute_script(arguments[3], task_slot_index);
            }
        }
        else if (num_arguments == 3)
        {
            std::string current_val = get_variable_value(arguments[0]);
            if (current_val == arguments[1])
            {
                execute_script(arguments[2], task_slot_index);
            }
        }
    }
    else if (command == "delay")
    {
        if (num_arguments == 1)
        {
            unsigned long delay_time = 0;
            if (!arguments[0].empty())
            {
                // Use base-10 for the delay time, pulling from arguments[0]
                delay_time = strtoul(arguments[0].c_str(), nullptr, 10); 
            }
            active_slots[task_slot_index].delay_end_time = current_system_time + delay_time;
        }
    }
    else if (command == "focus_next")
    {
        if (Screen *current = screen_manager->get_current_screen())
        {
            current->focus_next();
            needs_refresh = true;
            target_element_for_partial = ""; // NEED BETTER WAY
        }
    }
    else if (command == "focus_prev")
    {
        if (Screen *current = screen_manager->get_current_screen())
        {
            current->focus_previous();
            needs_refresh = true;
            target_element_for_partial = ""; // NEED BETTER WAY
        }
    }
    else if (command == "addvar")
    {
        if (num_arguments == 2)
        {
            std::string current_val_str = get_variable_value(arguments[0]);
            int current_val = 0;

            if (!current_val_str.empty())
            {
                current_val = atoi(current_val_str.c_str());
            }

            int amount_to_add = 0;
            amount_to_add = atoi(arguments[1].c_str());

            int new_val = current_val + amount_to_add;
            set_variable(arguments[0], std::to_string(new_val));

            needs_refresh = true;
            target_element_for_partial = ""; // NEED BETTER WAY
        }
    }
    else if (command == "updatetime")
    {
        if (num_arguments == 1)
        {
            time_t now = time(0);
            struct tm tstruct;
            char buf[10];

            tstruct = *localtime(&now);

            strftime(buf, sizeof(buf), "%H:%M", &tstruct);
            Serial.printf("Time: %s\n", buf);
            screen_manager->set_element_text(arguments[0], buf);
            needs_refresh = true;
            target_element_for_partial = arguments[0];
        }
    }
    else if (command == "list_down")
    {
        if (num_arguments >= 1) {
            UI_element* el = screen_manager->get_current_screen()->get_element_by_name(arguments[0]);
            if (el) {
                if (!static_cast<List_element*>(el)->scroll_down()) {
                    if (num_arguments == 2 && !arguments[1].empty()) execute_script(arguments[1], task_slot_index);
                    target_element_for_partial = ""; 
                } else {
                    target_element_for_partial = arguments[0];
                }
                needs_refresh = true;
            }
        }
    }
    else if (command == "list_up")
    {
        if (num_arguments >= 1) {
            UI_element* el = screen_manager->get_current_screen()->get_element_by_name(arguments[0]);
            if (el) {
                if (!static_cast<List_element*>(el)->scroll_up()) {
                    if (num_arguments == 2 && !arguments[1].empty()) execute_script(arguments[1], task_slot_index);
                    target_element_for_partial = ""; 
                } else {
                    target_element_for_partial = arguments[0];
                }
                needs_refresh = true;
            }
        }
    }
    else if (command == "list_click")
    {
        if (num_arguments >= 1) {
            UI_element* el = screen_manager->get_current_screen()->get_element_by_name(arguments[0]);
            if (el) {
                std::string act = static_cast<List_element*>(el)->click();
                target_element_for_partial = arguments[0];
                needs_refresh = true;
                if (!act.empty()) execute_script(act, task_slot_index);
            }
        }
    }
    else if (command == "spin_next")
    {
        if (num_arguments >= 1) {
            UI_element* el = screen_manager->get_current_screen()->get_element_by_name(arguments[0]);
            if (el) {
                static_cast<Spinner_element*>(el)->scroll_next();
                target_element_for_partial = arguments[0];
                needs_refresh = true;
            }
        }
    }
    else if (command == "spin_prev")
    {
        if (num_arguments >= 1) {
            UI_element* el = screen_manager->get_current_screen()->get_element_by_name(arguments[0]);
            if (el) {
                static_cast<Spinner_element*>(el)->scroll_prev();
                target_element_for_partial = arguments[0];
                needs_refresh = true;
            }
        }
    }
    else if (command == "spin_click")
    {
        if (num_arguments >= 1) {
            UI_element* el = screen_manager->get_current_screen()->get_element_by_name(arguments[0]);
            if (el) {
                std::string act = static_cast<Spinner_element*>(el)->click();
                target_element_for_partial = arguments[0];
                needs_refresh = true;
                if (!act.empty()) {
                    execute_script(act, task_slot_index);
                    target_element_for_partial = "";
                }
            }
        }
    }
    else if (command == "save_config")
    {
        // Loop through all passed parameters and write them to a JSON file
        if (num_arguments > 0)
        {
            JsonDocument doc;
            for (int i = 0; i < num_arguments; i++) {
                std::string var_name = arguments[i];
                doc[var_name] = get_variable_value(var_name);
            }

            File file = Storage::open_file_write("/config.json");
            if (file) {
                serializeJson(doc, file);
                Storage::close_file(file);
                Serial.println("[System] Config saved to LittleFS.");
            }
            
            // This is a background action; no screen refresh is necessary unless explicitly requested by a subsequent command
            target_element_for_partial = "";
        }
    }
}

/**
 * This function help clean arguments by dealing with the random operator | and
 * by getting rid of the [] charcters in group commands
 *
 * @param argument: The argument to clean
 * @return: The cleaned up argument
 */
std::string System::clean_arguments(std::string arguments)
{
    // array to store the options
    std::string options[10];
    int num_options = 0;

    // index of the start of the current option
    int start_index = 0;

    // ------------------------- General Cleanup -------------------------------
    // Remove leading/trailing whitespace if present
    size_t first = arguments.find_first_not_of(" \t\n\r\f\v");
    size_t last = arguments.find_last_not_of(" \t\n\r\f\v");
    if (first != std::string::npos && last != std::string::npos)
    {
        arguments = arguments.substr(first, (last - first + 1));
    }

    // ------------------- Clean up Group Argument -----------------------------
    // Clean up bracket boundaries
    if (!arguments.empty() && arguments.front() == '[')
    {
        if (arguments.back() == ']')
        {
            arguments = arguments.substr(1, arguments.length() - 2);
        }
        else
        {
            // Auto-close missing trailing bracket support
            arguments = arguments.substr(1);
        }
    }

    // ------------------------ Clean Up Options -------------------------------
    // split the argument into the options based on the | character and add them
    // to the options array
    while (start_index < arguments.length() && num_options < sizeof(options) / sizeof(options[0]))
    {
        // index of the end of the current option
        int end_index = arguments.find('|', start_index);

        if (end_index == std::string::npos)
        {
            // add the option to the list of options
            options[num_options] = arguments.substr(start_index);
            num_options++;
            start_index = arguments.length();
        }
        else
        {
            // add the option to the list of options
            options[num_options] = arguments.substr(start_index, end_index - start_index);

            // get the next option
            num_options++;
            start_index = end_index + 1;
        }
    }

    // pick a random option from the list of options
    if (num_options > 0)
    {
        int random_index = rand() % num_options;
        return options[random_index];
    }
    else
    {
        return ""; // Return an empty string if there are no options
    }
}

/**
 * This function is passed a single action and parses it in the main command and
 * all its argument. It uses the clean_arguments function to help deal with
 * random arguments and to clean up the [] characters for group arguments
 *
 * @param action: The action to parse
 * @param task_slot_index: The index of the Task_slot we want to use (gets
 * passed to the dispatch_command function for handeling delays)
 */
void System::parse_command(const std::string &action, int task_slot_index)
{
    // Required output variables (will send to the next stage)
    std::string command;
    std::string arguments[10]; // Assuming a maximum of 10 arguments
    int num_arguments = 0;

    // ----------------------- Extract the Command -----------------------------
    // index at the start of the action
    int start_index = 0;
    int bracket_depth = 0;

    int end_index = action.find(':', start_index);
    // check if there is a colon (or if it is a singel command action)
    if (end_index != std::string::npos)
    {
        command = action.substr(start_index, (end_index - start_index));
        start_index = end_index + 1;
    }
    else
    {
        command = action;
        start_index = action.length() + 1;
    }

    // -------------------- Extract the Arguments ------------------------------
    // loop through all the charcters in the action
    for (int i = start_index; i <= action.length(); i++)
    {
        // ------------------------ Update Bracket Depth -----------------------
        if (i < action.length())
        {
            if (action[i] == '[')
                bracket_depth++;
            if (action[i] == ']')
                bracket_depth--;
        }

        // --------------------- Split Argument --------------------------------
        // only creat an argument if we hit a colon or end and we are not inside
        // any brackets
        if ((i == action.length() || action[i] == ':') && bracket_depth == 0)
        {
            // if there is more room for arguments
            if (num_arguments < sizeof(arguments) / sizeof(arguments[0]))
            {
                // add the argument to the list
                arguments[num_arguments] = clean_arguments(action.substr(start_index, i - start_index));

                // get ready for the next argument
                num_arguments++;
                start_index = i + 1;
            }
        }
    }

    // --------------- Send Action Parameters to Next Function -----------------
    // if the command is not empty, send the action to the appropriate handler
    if (!command.empty())
    {
        System::dispatch_command(command, arguments, num_arguments, task_slot_index);
    }
}

/**
 * This function is passed a string of all actions that need to be executed. It
 * first creates a new task in the task slots (if there are no slot availible it
 * throws out the task to ensure no memory overflows). Then it extracts one
 * action at a time from the string of actions and sends it to the parse_action
 * function.
 * It also pauses all actions in the current task slot if there is a delay on
 * that task slot. The actions are saved in the Task_slot struct and are resumed
 * when the delay ends.
 *
 * @param actions: A string containing all the actions to run.
 * @param task_slot_index: The index of the Task_slot we want to use (-1 for a
 * new stask slot)
 */
void System::execute_script(const std::string &actions, int task_slot_index)
{
    // --------------------- Setup a task slot ---------------------------------
    // check to see if it is a new task
    if (task_slot_index == -1)
    {
        int free_index = -1;

        // loop through all the task slots
        for (int i = 0; i < MAX_CONCURRENT_TASKS && free_index == -1; i++)
        {
            // check if the task slot is non active
            if (active_slots[i].is_active == false)
            {
                free_index = i;
            }
        }

        // if there are no slots available
        if (free_index == -1)
        {
            return; // exit now
        }

        task_slot_index = free_index;
        active_slots[task_slot_index].start();
    }

    // -------------------Split up the actions----------------------------------
    // index of the start of the current action
    int start_index = 0;
    int bracket_depth = 0;
    // string to hold the current action
    std::string action;
    std::string remainder;

    // loop through all the charcters in the actions
    for (int i = 0; i <= actions.length(); i++)
    {
        // ------------------------ Update Bracket Depth -----------------------
        if (i < actions.length())
        {
            if (actions[i] == '[')
                bracket_depth++;
            if (actions[i] == ']')
                bracket_depth--;
        }

        // --------------------- Split Actions ---------------------------------
        // only creat an action if we hit a semicolon or end and we are not
        // inside any brackets
        if ((i == actions.length() || actions[i] == ';') && bracket_depth == 0)
        {
            // --------------- Get the Action Statement ------------------------
            action = actions.substr(start_index, i - start_index);
            // remove leading and trailing whitespace from the action
            size_t first = action.find_first_not_of(" \t\n\r\f\v");
            if (first == std::string::npos)
            {
                action = ""; // It's completely empty or pure whitespace
            }
            else
            {
                size_t last = action.find_last_not_of(" \t\n\r\f\v");
                action = action.substr(first, (last - first + 1));
            }

            // If the action is not empty then send it to the decode section
            if (!action.empty())
            {
                // send to the decode command function
                parse_command(action, task_slot_index);

                // ------------------- Deal with Delays ------------------------
                // check if the most recent action was a delay command
                if (active_slots[task_slot_index].delay_end_time > 0)
                {
                    // if there are more actions left in this execution layer
                    if (i < actions.length())
                    {
                        // get the remainder of the actions left
                        remainder = actions.substr(i + 1);

                        // If an inner recursive block (nested executions) already claimed a remainder, safely append to it
                        if (!active_slots[task_slot_index].remaining_actions.empty())
                        {
                            active_slots[task_slot_index].remaining_actions = active_slots[task_slot_index].remaining_actions + "; " + remainder;
                        }
                        else
                        {
                            active_slots[task_slot_index].remaining_actions = remainder;
                        }
                    }
                    return; // stop executing the actions until the delay is over
                }
            }
            start_index = i + 1; // update start index for next action
        }
    }

    // end the task
    active_slots[task_slot_index].stop();
}

// *****************************************************************************
//                             SYSTEM COMMANDS
// *****************************************************************************

/**
 * Switches to the screen with the given name if it exists in the screen manager.
 * It will also run the actions for exiting the old screen and the actions for
 * entering the new screen.
 * If the screen does not exist, it will do nothing.
 *
 * @param screen_name: The name of the screen to switch to.
 */
void System::switch_to_screen(const std::string &screen_name)
{
    // gets the current screen
    Screen *current_screen = screen_manager->get_current_screen();
    // if there is a current screen
    if (current_screen != nullptr)
    {
        // get the action for exit event
        std::string exit_action = current_screen->get_action_for_event(ON_EXIT);
        // if the action is not an epmty string
        if (!exit_action.empty())
        {
            // run the action
            execute_script(exit_action, -1);
        }
    }

    // switch to the new screen
    screen_manager->switch_screen(screen_name);

    // gets the new current screen
    current_screen = screen_manager->get_current_screen();
    // if there is a current screen
    if (current_screen != nullptr)
    {
        current_screen->pass_system_to_elements(this);

        // get the action for the load event
        std::string enter_action = current_screen->get_action_for_event(ON_ENTER);
        // if the action is not an epmty string
        if (!enter_action.empty())
        {
            // run the action
            execute_script(enter_action, -1);
        }
    }
}

bool System::is_idle() 
{
    // If a hardware interrupt fired, we are not idle
    if (input_handler->check_wakeup_flag()) {
        input_handler->clear_wakeup_flag();
        return false;
    }
    
    // If the queue has items, the screen needs drawing, or a button is actively being held down
    if (input_handler->has_events()) return false;
    if (input_handler->is_any_input_active()) return false;
    if (needs_refresh) return false;
    
    return true;
}

unsigned long System::time_until_next_event() 
{
    unsigned long now = millis();
    unsigned long time_to_minute = 0;
    
    // Calculate time until the next ON_MINUTE hardware tick
    if (now - last_minute_time < 60000) {
        time_to_minute = 60000 - (now - last_minute_time);
    }

    // Calculate time until the nearest executing script delay finishes
    unsigned long time_to_task = 0xFFFFFFFF; // Max unsigned long value
    for (int i = 0; i < MAX_CONCURRENT_TASKS; ++i) {
        if (active_slots[i].is_active && active_slots[i].delay_end_time > 0) {
            if (active_slots[i].delay_end_time > now) {
                unsigned long delay_left = active_slots[i].delay_end_time - now;
                if (delay_left < time_to_task) {
                    time_to_task = delay_left;
                }
            } else {
                return 0; // A task delay has already expired, wake up immediately!
            }
        }
    }

    // Return whichever event happens first
    return (time_to_minute < time_to_task) ? time_to_minute : time_to_task;
}
