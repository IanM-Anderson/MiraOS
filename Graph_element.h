#ifndef GRAPH_ELEMENT_H
#define GRAPH_ELEMENT_H

#include <cstdint>
#include <string>
#include <vector>
#include "UI_element.h"

struct Graph_Tick {
    float value;
    std::string label;
};

struct Graph_Point {
    float x;
    float y;
};

class Graph_element : public UI_element
{
private:
    std::vector<Graph_Point> data_points;
    std::vector<Graph_Tick> x_ticks;
    std::vector<Graph_Tick> y_ticks;

    std::string title = "";
    std::string x_axis_title = "";
    std::string y_axis_title = "";

    bool show_line = true;
    bool show_stems = false;
    bool show_grid = false;
    bool show_point_labels = false;
    bool show_points = true;

    float min_x = 0, max_x = 100;
    float min_y = 0, max_y = 100;

    void calculate_bounds();

public:
    Graph_element(std::string name, std::string x, std::string y, std::string z,
                  std::string width, std::string height, std::string visible,
                  bool is_focusable);

    void set_titles(std::string main_title, std::string x_title, std::string y_title);
    void set_toggles(bool line, bool stems, bool grid, bool pt_labels, bool pts);
    
    void add_data_point(float px, float py);
    void add_x_tick(float val, std::string label);
    void add_y_tick(float val, std::string label);

    void draw_self() override;
};

#endif