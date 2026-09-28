#ifndef TEXT_ELEMENT_H
#define TEXT_ELEMENT_H

#include <cstdint>
#include "UI_element.h"

class Text_element : public UI_element
{
private:
    std::string text = "";
    std::string text_size = "12";
    std::string text_align = "left";

public:
    Text_element(std::string name, std::string x, std::string y, std::string z,
                 std::string width, std::string height, std::string visible,
                 bool is_focusable);

    // Initial setup from JSON
    void set_text_properties(std::string t, std::string ts, std::string tc, std::string ta);

    // Dynamic update from scripts
    void set_text(std::string new_text) override;

    void draw_self() override;
};

#endif