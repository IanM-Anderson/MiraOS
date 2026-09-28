#ifndef LIST_ELEMENT_H
#define LIST_ELEMENT_H

#include <cstdint>
#include <string>
#include <vector>
#include "UI_element.h"

class List_element : public UI_element
{
private:
    std::vector<std::string> labels;
    std::vector<std::string> actions;
    
    int items_per_page = 5;
    int current_page = 0;
    int selected_index = 0;

public:
    List_element(std::string name, std::string x, std::string y, std::string z,
                 std::string width, std::string height, std::string visible,
                 bool is_focusable);

    void set_list_config(int per_page);
    void add_item(std::string label, std::string action);

    bool scroll_down();
    bool scroll_up();
    std::string click(); 

    void draw_self() override;
};

#endif