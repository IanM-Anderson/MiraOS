#ifndef RECT_ELEMENT_H
#define RECT_ELEMENT_H

#include <cstdint>
#include "UI_element.h"

class Rect_element : public UI_element
{
public:
    Rect_element(std::string name, std::string x, std::string y, std::string z,
                 std::string width, std::string height, std::string visible,
                 bool is_focusable);
    void draw_self();
};

#endif