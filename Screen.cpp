#include "Screen.h"

Screen::Screen()
{
    Screen::name = "";
    num_elements = 0;
    num_events = 0;
    focus_index = -1;

    for (int i = 0; i < sizeof(elements) / sizeof(elements[0]); i++)
    {
        elements[i] = nullptr;
    }
}

Screen::Screen(std::string name)
{
    Screen::name = name;
    num_elements = 0;
    num_events = 0;
    focus_index = -1;

    for (int i = 0; i < sizeof(elements) / sizeof(elements[0]); i++)
    {
        elements[i] = nullptr;
    }
}

Screen::~Screen()
{
    int curr_num_elements = num_elements;
    for (int i = 0; i < curr_num_elements; i++)
    {
        delete elements[i]; 
        elements[i] = nullptr;
        num_elements--;
    }
}

std::string Screen::get_name()
{
    return Screen::name;
}

void Screen::pass_system_to_elements(System *sys_ptr)
{
    for (int i = 0; i < num_elements; i++)
    {
        if (elements[i] != nullptr)
        {
            elements[i]->set_system(sys_ptr);
        }
    }
}

void Screen::sort_elements()
{
    for (int i = 1; i < num_elements; ++i)
    {
        UI_element *key = elements[i];
        int key_z = key->get_z();
        int j = i - 1;

        while (j >= 0 && elements[j]->get_z() > key_z)
        {
            elements[j + 1] = elements[j];
            j = j - 1;
        }
        elements[j + 1] = key;
    }
}

void Screen::add_element(UI_element *new_element)
{
    if (num_elements < sizeof(elements) / sizeof(elements[0]))
    {
        elements[num_elements] = new_element;
        num_elements++;
        sort_elements();
    }
}

void Screen::clear()
{
    // Placeholder
}

void Screen::draw_screen()
{
    clear();

    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(COLOR_WHITE);

        for (int i = 0; i < num_elements; ++i)
        {
            if (elements[i] != nullptr)
            {
                elements[i]->draw();
            }
        }
    } while (display.nextPage());

    display.powerOff();
}

void Screen::draw_screen_partial(int x, int y, int w, int h)
{
    x = (x / 8) * 8;
    w = ((w + 7) / 8) * 8;

    display.setPartialWindow(x, y, w, h);
    
    display.firstPage();
    do {
        display.fillScreen(COLOR_WHITE);
        for (int i = 0; i < num_elements; ++i)
        {
            if (elements[i] != nullptr)
            {
                elements[i]->draw();
            }
        }
    } while (display.nextPage());

    display.powerOff();
}

UI_element *Screen::get_element_by_name(const std::string &element_name)
{
    for (int i = 0; i < num_elements; ++i)
    {
        if (elements[i]->get_name() == element_name)
        {
            return elements[i];
        }
    }
    return nullptr; 
}

void Screen::invert_screen_colors()
{
    for (int i = 0; i < num_elements; ++i)
    {
        elements[i]->invert_colors();
    }
    draw_screen();
}

UI_element *Screen::get_focused_element()
{
    if (focus_index >= 0 && focus_index < num_elements)
    {
        return elements[focus_index];
    }
    return nullptr;
}

UI_element *Screen::focus_next()
{
    if (num_elements == 0) return nullptr;

    int start_idx = (focus_index < 0) ? 0 : focus_index + 1;

    for (int count = 0; count < num_elements; count++)
    {
        int i = (start_idx + count) % num_elements;

        if (elements[i]->get_focusable())
        {
            if (focus_index >= 0 && focus_index < num_elements)
            {
                elements[focus_index]->set_focused(false);
                elements[focus_index]->off_focus();
            }

            focus_index = i;
            elements[focus_index]->set_focused(true);
            elements[focus_index]->on_focus();

            Serial.printf(" [Display Driver] -> Focused Element Name: %s\n", elements[focus_index]->get_name().c_str());

            return elements[focus_index];
        }
    }
    return nullptr;
}

UI_element *Screen::focus_previous()
{
    if (num_elements == 0) return nullptr;

    int start_idx = (focus_index <= 0) ? num_elements - 1 : focus_index - 1;

    for (int count = 0; count < num_elements; count++)
    {
        int i = start_idx - count;
        if (i < 0)
        {
            i += num_elements;
        }

        if (elements[i]->get_focusable())
        {
            if (focus_index >= 0 && focus_index < num_elements)
            {
                elements[focus_index]->set_focused(false);
                elements[focus_index]->off_focus();
            }

            focus_index = i;
            elements[focus_index]->set_focused(true);
            elements[focus_index]->on_focus();

            Serial.printf(" [Display Driver] -> Focused Element Name: %s\n", elements[focus_index]->get_name().c_str());

            return elements[focus_index];
        }
    }
    return nullptr;
}

void Screen::set_element_visibility(const std::string &element_name, std::string visible)
{
    bool found = false;
    for (int i = 0; i < num_elements && !found; ++i)
    {
        if (elements[i]->get_name() == element_name)
        {
            elements[i]->set_visible(visible);
            found = true;
        }
    }
}

void Screen::toggle_element_visibility(const std::string &element_name)
{
    bool found = false;
    for (int i = 0; i < num_elements && !found; ++i)
    {
        if (elements[i]->get_name() == element_name)
        {
            elements[i]->toggle_visible();
            found = true;
        }
    }
}

void Screen::set_element_text(const std::string &element_name, const std::string &text)
{
    bool found = false;
    for (int i = 0; i < num_elements && !found; ++i)
    {
        if (elements[i]->get_name() == element_name)
        {
            elements[i]->set_text(text);
            found = true;
        }
    }
}

void Screen::update_element_position(const std::string &element_name, std::string x, std::string y, std::string z)
{
    UI_element *el = get_element_by_name(element_name);
    if (el != nullptr)
    {
        el->set_position(x, y, z);
        sort_elements();
    }
}

void Screen::add_event_mapping(Input_Event event, const std::string &action)
{
    if (num_events < sizeof(events) / sizeof(events[0]))
    {
        events[num_events].event_type = event;
        events[num_events].action = action;
        num_events++;
    }
}

std::string Screen::get_action_for_event(Input_Event event)
{
    for (int i = 0; i < num_events; ++i)
    {
        if (events[i].event_type == event)
        {
            return events[i].action;
        }
    }
    return ""; 
}