#ifndef SHEET_LOADER_H
#define SHEET_LOADER_H

#include "Variables.h"
#include "Screen_manager.h"
#include "Rect_element.h"
#include "Image_element.h"
#include "Circle_element.h"
#include "Text_element.h"
#include "Graph_element.h"
#include "List_element.h"
#include "Spinner_element.h"
#include "Storage.h"
#include "ArduinoJson-v7.4.3.h"

class Sheet_loader {
public:
    static void load_json(const std::string &url);
    static void load_json_from_string(const std::string &json_string); // Restored
    static void split_and_save_screens(); 
    static Screen *load_screen_from_storage(const std::string &screen_name, Screen *&current_screen);
};

#endif