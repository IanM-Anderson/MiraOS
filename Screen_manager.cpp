#include <string>
#include "Screen_manager.h"

Screen_manager::Screen_manager()
{
    current_screen = nullptr;
}

Screen *Screen_manager::get_current_screen()
{
    return current_screen;
}

void Screen_manager::add_element_to_screen(UI_element *new_element)
{
    Screen *found_screen = get_current_screen();
    if (found_screen != nullptr)
    {
        found_screen->add_element(new_element);
    }
}

bool Screen_manager::switch_screen(const std::string &screen_name)
{
    Screen *new_screen = Sheet_loader::load_screen_from_storage(screen_name, current_screen);

    if (new_screen != nullptr && new_screen != current_screen)
    {
        // FATAL MEMORY LEAK PATCH: Delete old screen from the heap before replacing it
        if (current_screen != nullptr) {
            delete current_screen; 
        }
        current_screen = new_screen;
        refresh_screen(); 
    }
    return new_screen != nullptr;
}

UI_element *Screen_manager::get_focused_element()
{
    if (current_screen != nullptr)
    {
        return current_screen->get_focused_element();
    }
    return nullptr;
}

void Screen_manager::draw_screen(Screen *screen)
{
    if (screen != nullptr)
    {
        screen->draw_screen();
    }
}

void Screen_manager::refresh_screen()
{
    if (current_screen != nullptr)
    {
        Serial.printf("\tFull Refresh\n");
        draw_screen(current_screen);
    }
}

void Screen_manager::refresh_element(const std::string &element_name) 
{
    if (current_screen == nullptr) return;
    UI_element *el = current_screen->get_element_by_name(element_name);
    
    if (el != nullptr && el->get_partial_refresh()) {
        // Allow up to 60 partials for spinners/fast elements to prevent ghosting flashes
        int limit = el->get_fast_refresh() ? MAX_FAST_UPDATES : MAX_PARTIAL_UPDATES_BEFORE_FULL;
        
        if (partial_update_counter < limit) {
            partial_update_counter++;
            current_screen->draw_screen_partial(el->get_x(), el->get_y(), el->get_width(), el->get_height());
            return;
        }
    }
    partial_update_counter = 0;
    current_screen->draw_screen();
}

void Screen_manager::invert_screen_colors()
{
    if (current_screen != nullptr)
    {
        current_screen->invert_screen_colors();
    }
}

void Screen_manager::set_element_visibility(const std::string &element_name, std::string visible)
{
    if (current_screen != nullptr)
    {
        current_screen->set_element_visibility(element_name, visible);
    }
}

void Screen_manager::toggle_element_visibility(const std::string &element_name)
{
    if (current_screen != nullptr)
    {
        current_screen->toggle_element_visibility(element_name);
    }
}

void Screen_manager::set_element_text(const std::string &element_name, const std::string &text)
{
    if (current_screen != nullptr)
    {
        current_screen->set_element_text(element_name, text);
    }
}

void Screen_manager::update_element_position(const std::string &element_name, std::string x, std::string y, std::string z)
{
    if (current_screen != nullptr)
    {
        current_screen->update_element_position(element_name, x, y, z);
    }
}