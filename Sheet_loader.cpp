#include "Sheet_loader.h"
#include "Network.h"
#include <Arduino.h>
#include <cstdlib>

void Sheet_loader::load_json(const std::string &url) {
    if (Network::get_json(url)) {
        split_and_save_screens();
    }
}

void Sheet_loader::load_json_from_string(const std::string &json_string) {
    File cache = Storage::open_file_write("/cache.json");
    if (cache) {
        cache.print(json_string.c_str());
        Storage::close_file(cache);
    }
    split_and_save_screens();
}

void Sheet_loader::split_and_save_screens()
{
    Serial.println("[Sheet Loader] Parsing master JSON payload...");
    Serial.printf("[Memory] Free Heap before master parse: %d bytes\n", ESP.getFreeHeap());

    File file = Storage::open_file_read("/cache.json");
    if (!file) {
        Serial.println("[Sheet Loader ERROR] Could not open /cache.json");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    Storage::close_file(file);

    if (error)
    {
        Serial.printf("[Sheet Loader ERROR] Failed to parse JSON: %s\n", error.c_str());
        return;
    }

    // Updated: ArduinoJson v7 syntax for checking key existence and type
    if (doc["variables"].is<JsonArray>()) {
        JsonArray vars = doc["variables"];
        for (JsonObject v : vars) {
            set_variable(v["name"] | "", v["value"] | "");
        }
    }

    JsonArray screens = doc["screens"];
    if (screens.isNull())
    {
        Serial.println("[Sheet Loader ERROR] No 'screens' array found in the JSON.");
        return;
    }

    for (JsonObject screen_obj : screens)
    {
        std::string screen_name = screen_obj["screen_name"].as<std::string>();
        if (screen_name.empty())
        {
            screen_name = screen_obj["name"] | ""; 
            if (screen_name.empty()) {
                Serial.println("[Sheet Loader Warning] Found a screen with no name, skipping.");
                continue;
            }
        }

        JsonArray elements = screen_obj["elements"];
        for (JsonObject elem_json : elements)
        {
            std::string type = elem_json["type"] | "";
            if (type == "image" && !elem_json["bitmap_hex"].isNull())
            {
                std::string img_name = elem_json["name"].as<std::string>();
                std::string hex_data = elem_json["bitmap_hex"].as<std::string>();
                std::string bmp_path = "/bitmaps/" + img_name + ".hex";
                
                File bmp_file = Storage::open_file_write(bmp_path);
                if (bmp_file)
                {
                    bmp_file.print(hex_data.c_str());
                    Storage::close_file(bmp_file);
                    Serial.printf("[Sheet Loader] Successfully saved bitmap file: %s\n", bmp_path.c_str());
                }
                
                elem_json["bitmap_file"] = img_name + ".hex";
                elem_json.remove("bitmap_hex");
            }
        }

        std::string filepath = "/screens/" + screen_name + ".json";
        File screen_file = Storage::open_file_write(filepath);
        if (screen_file)
        {
            serializeJson(screen_obj, screen_file);
            Storage::close_file(screen_file);
            Serial.printf("[Sheet Loader] Successfully saved individual screen: %s\n", filepath.c_str());
        }
    }

    doc.clear();
    Serial.printf("[Memory] Free Heap after splitting & clearing doc: %d bytes\n", ESP.getFreeHeap());
}

Screen *Sheet_loader::load_screen_from_storage(const std::string &screen_name, Screen *&old_screen)
{
    std::string filepath = "/screens/" + screen_name + ".json";
    File file = Storage::open_file_read(filepath);
    if (!file)
    {
        Serial.printf("[Loader ERROR] Could not find or open screen file: %s\n", filepath.c_str());
        return old_screen; 
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    Storage::close_file(file);

    if (error)
    {
        Serial.printf("[Loader ERROR] JSON parsing failed for %s: %s\n", screen_name.c_str(), error.c_str());
        return old_screen;
    }
    
    if (old_screen != nullptr)
    {
        delete old_screen;
        old_screen = nullptr;
    }
    
    std::string s_name = doc["screen_name"] | screen_name;
    Screen *new_screen = new Screen(s_name);

    // Updated: ArduinoJson v7 syntax for checking key existence and type
    if (doc["events"].is<JsonObject>()) {
        JsonObject screen_events = doc["events"];
        for (JsonPair kv : screen_events)
        {
            std::string event_key = kv.key().c_str();
            std::string action_script = kv.value().as<std::string>();
            Input_Event ev = parse_event_type(event_key);
            if (ev != Input_Event::NUM_EVENTS)
            {
                new_screen->add_event_mapping(ev, action_script);
            }
        }
    }

    JsonArray elements = doc["elements"];
    for (JsonObject elem_json : elements)
    {
        std::string type = elem_json["type"] | "rect";
        std::string name = elem_json["name"] | "";
        std::string x = elem_json["x"] | "0";
        std::string y = elem_json["y"] | "0";
        std::string z = elem_json["z"] | "0";
        std::string w = elem_json["w"] | "0";
        std::string h = elem_json["h"] | "0";
        std::string visible = elem_json["visible"] | "1";
        bool focusable = elem_json["focusable"] | false;
        bool partial_pref = elem_json["partial"] | false;
        std::string col = elem_json["color"] | "1";
        std::string b_col = elem_json["boarder_color"] | "0";
        std::string b_thick = elem_json["boarder_thickness"] | "0";
        std::string fill_val = elem_json["fill"] | "false";
        std::string txt = elem_json["text"] | "";
        std::string t_size = elem_json["text_size"] | "20";
        std::string t_color = elem_json["text_color"] | "0";        
        
        UI_element *element = nullptr;

        if (type == "rect")
        {
            element = new Rect_element(name, x, y, z, w, h, visible, focusable);
        }
        else if (type == "image")
        {
            Image_element *img_el = new Image_element(name, x, y, z, w, h, visible, focusable);
            std::string bmp_file = elem_json["bitmap_file"] | "";
            int orig_w = elem_json["src_w"] | 16;
            int orig_h = elem_json["src_h"] | 16;
            
            if (!bmp_file.empty())
            {
                std::string bmp_path = "/bitmaps/" + bmp_file;
                File bmp_file_obj = Storage::open_file_read(bmp_path);
                
                if (bmp_file_obj)
                {
                    size_t byte_count = bmp_file_obj.size() / 2;
                    uint8_t *buffer = new uint8_t[byte_count];
                    
                    for (size_t i = 0; i < byte_count; ++i)
                    {
                        char hex_byte[3] = {0};
                        bmp_file_obj.readBytes(hex_byte, 2);
                        buffer[i] = (uint8_t)strtoul(hex_byte, nullptr, 16);
                    }
                    
                    Storage::close_file(bmp_file_obj);
                    
                    img_el->set_bitmap(buffer, true, orig_w, orig_h); 
                }
            }
            element = img_el;
        }
        else if (type == "circle")
        {
            element = new Circle_element(name, x, y, z, w, h, visible, focusable);
        }
        else if (type == "text")
        {
            Text_element *text_el = new Text_element(name, x, y, z, w, h, visible, focusable);
            std::string t_align = elem_json["text_align"] | "left";
            text_el->set_text_properties(txt, t_size, t_color, t_align);
            element = text_el;
        }
        else if (type == "graph")
        {
            Graph_element *graph_el = new Graph_element(name, x, y, z, w, h, visible, focusable);
            
            graph_el->set_titles(elem_json["title"] | "", elem_json["x_title"] | "", elem_json["y_title"] | "");
            graph_el->set_toggles(elem_json["show_line"] | true, elem_json["show_stems"] | false, 
                                  elem_json["show_grid"] | false, elem_json["show_point_labels"] | false, 
                                  elem_json["show_points"] | true);

            if (elem_json["data"].is<JsonArray>()) {
                for (JsonObject pt : elem_json["data"].as<JsonArray>()) {
                    graph_el->add_data_point(pt["x"] | 0.0f, pt["y"] | 0.0f);
                }
            }
            if (elem_json["x_ticks"].is<JsonArray>()) {
                for (JsonObject t : elem_json["x_ticks"].as<JsonArray>()) {
                    graph_el->add_x_tick(t["v"] | 0.0f, t["l"] | "");
                }
            }
            if (elem_json["y_ticks"].is<JsonArray>()) {
                for (JsonObject t : elem_json["y_ticks"].as<JsonArray>()) {
                    graph_el->add_y_tick(t["v"] | 0.0f, t["l"] | "");
                }
            }
            element = graph_el;
        }
        else if (type == "list")
        {
            List_element *list_el = new List_element(name, x, y, z, w, h, visible, focusable);
            list_el->set_list_config(elem_json["items_per_page"] | 5);

            if (elem_json["labels"].is<JsonArray>() && elem_json["actions"].is<JsonArray>()) {
                JsonArray lbl_arr = elem_json["labels"];
                JsonArray act_arr = elem_json["actions"];
                size_t item_count = std::min(lbl_arr.size(), act_arr.size());
                for (size_t i = 0; i < item_count; i++) {
                    list_el->add_item(lbl_arr[i] | "", act_arr[i] | "");
                }
            }
            element = list_el;
        }
        else if (type == "spinner")
        {
            Spinner_element *spin_el = new Spinner_element(name, x, y, z, w, h, visible, focusable);
            spin_el->set_spinner_config(elem_json["target_variable"] | "", 
                                        elem_json["submit_action"] | "", 
                                        elem_json["initial_text"] | "");
            element = spin_el;
        }

        if (element != nullptr)
        {
            element->set_styling(col, b_col, b_thick, fill_val);
            element->set_partial_refresh(partial_pref);
            element->set_fast_refresh(elem_json["fast_refresh"] | false);
            
            // Updated: ArduinoJson v7 syntax for checking key existence and type
            if (elem_json["events"].is<JsonObject>()) {
                JsonObject events_json = elem_json["events"];
                for (JsonPair kv : events_json)
                {
                    std::string event_key = kv.key().c_str();
                    std::string action_script = kv.value().as<std::string>();
                    Input_Event ev = parse_event_type(event_key);
                    if (ev != Input_Event::NUM_EVENTS)
                    {
                        element->add_event_mapping(ev, action_script);
                    }
                }
            }
            new_screen->add_element(element);
        }
    }
    return new_screen;
}