#ifndef IMAGE_ELEMENT_H
#define IMAGE_ELEMENT_H

#include <cstdint>
#include "UI_element.h"

class Image_element : public UI_element
{
private:
    const uint8_t *bitmap_data = nullptr;
    bool owns_data = false;
    int src_width = 16;  // Original bitmap width
    int src_height = 16; // Original bitmap height

public:
    Image_element(std::string name, std::string x, std::string y, std::string z,
                  std::string width, std::string height, std::string visible,
                  bool is_focusable);
    ~Image_element() override;

    // Set bitmap data and optionally take memory ownership
    void set_bitmap(const uint8_t *data, bool take_ownership = false, int sw = 16, int sh = 16);

    void draw_self() override;
};

#endif