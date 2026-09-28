#ifndef SPINNER_ELEMENT_H
#define SPINNER_ELEMENT_H

#include <cstdint>
#include <string>
#include <vector>
#include "UI_element.h"

class Spinner_element : public UI_element
{
private:
    std::string current_text = "";
    std::string target_variable = "";
    std::string submit_action = "";

    int char_index = 0;
    std::vector<std::string> charset;

    void init_charset();

public:
    Spinner_element(std::string name, std::string x, std::string y, std::string z,
                    std::string width, std::string height, std::string visible,
                    bool is_focusable);

    void set_spinner_config(std::string target_var, std::string action, std::string initial_text);

    void scroll_next();
    void scroll_prev();
    std::string click(); 

    void draw_self() override;
};

#endif