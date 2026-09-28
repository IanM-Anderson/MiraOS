#include "Graph_element.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

Graph_element::Graph_element(std::string name, std::string x, std::string y, std::string z,
                             std::string width, std::string height, std::string visible,
                             bool is_focusable)
    : UI_element(name, x, y, z, width, height, visible, is_focusable) {}

void Graph_element::set_titles(std::string main_title, std::string x_title, std::string y_title) {
    title = main_title;
    x_axis_title = x_title;
    y_axis_title = y_title;
}

void Graph_element::set_toggles(bool line, bool stems, bool grid, bool pt_labels, bool pts) {
    show_line = line;
    show_stems = stems;
    show_grid = grid;
    show_point_labels = pt_labels;
    show_points = pts;
}

void Graph_element::add_data_point(float px, float py) {
    data_points.push_back({px, py});
}

void Graph_element::add_x_tick(float val, std::string label) {
    x_ticks.push_back({val, label});
}

void Graph_element::add_y_tick(float val, std::string label) {
    y_ticks.push_back({val, label});
}

void Graph_element::calculate_bounds() {
    if (data_points.empty()) return;
    min_x = data_points[0].x; max_x = data_points[0].x;
    min_y = data_points[0].y; max_y = data_points[0].y;

    for (const auto& pt : data_points) {
        if (pt.x < min_x) min_x = pt.x;
        if (pt.x > max_x) max_x = pt.x;
        if (pt.y < min_y) min_y = pt.y;
        if (pt.y > max_y) max_y = pt.y;
    }
    if (max_x == min_x) { max_x += 1; min_x -= 1; }
    if (max_y == min_y) { max_y += 1; min_y -= 1; }
}

void Graph_element::draw_self() {
    uint16_t primary_col = parse_color(color);
    uint16_t border_col = parse_color(boarder_color);
    uint16_t grid_col = COLOR_LIGHT_GREY;

    int base_x = get_x(), base_y = get_y();
    int w = get_width(), h = get_height();

    std::string dec_fill = fill;
    decode_string(&dec_fill);
    if (dec_fill == "true" || dec_fill == "1") display.fillRect(base_x, base_y, w, h, primary_col);
    display.drawRect(base_x, base_y, w, h, border_col);

    int margin_left = 40, margin_bottom = 30, margin_top = 25, margin_right = 15;
    int plot_x = base_x + margin_left, plot_y = base_y + margin_top;
    int plot_w = w - margin_left - margin_right, plot_h = h - margin_top - margin_bottom;

    if (plot_w <= 0 || plot_h <= 0) return;

    calculate_bounds();

    auto map_x = [&](float val) -> int { return plot_x + (int)(((val - min_x) / (max_x - min_x)) * plot_w); };
    auto map_y = [&](float val) -> int { return plot_y + plot_h - (int)(((val - min_y) / (max_y - min_y)) * plot_h); };

    display.setTextSize(1);
    display.setTextColor(border_col);

    if (!title.empty()) {
        int16_t tx, ty; uint16_t tw, th;
        display.getTextBounds(title.c_str(), 0, 0, &tx, &ty, &tw, &th);
        display.setCursor(base_x + (w - tw) / 2, base_y + 5);
        display.print(title.c_str());
    }

    display.drawLine(plot_x, plot_y + plot_h, plot_x + plot_w, plot_y + plot_h, border_col); // X-Axis
    display.drawLine(plot_x, plot_y, plot_x, plot_y + plot_h, border_col); // Y-Axis

    for (const auto& tick : x_ticks) {
        int tx = map_x(tick.value);
        if (tx >= plot_x && tx <= plot_x + plot_w) {
            display.drawLine(tx, plot_y + plot_h, tx, plot_y + plot_h + 4, border_col);
            if (show_grid) display.drawLine(tx, plot_y, tx, plot_y + plot_h, grid_col);
            int16_t bx, by; uint16_t bw, bh;
            display.getTextBounds(tick.label.c_str(), 0, 0, &bx, &by, &bw, &bh);
            display.setCursor(tx - (bw / 2), plot_y + plot_h + 6);
            display.print(tick.label.c_str());
        }
    }

    for (const auto& tick : y_ticks) {
        int ty = map_y(tick.value);
        if (ty >= plot_y && ty <= plot_y + plot_h) {
            display.drawLine(plot_x - 4, ty, plot_x, ty, border_col);
            if (show_grid) display.drawLine(plot_x, ty, plot_x + plot_w, ty, grid_col);
            int16_t bx, by; uint16_t bw, bh;
            display.getTextBounds(tick.label.c_str(), 0, 0, &bx, &by, &bw, &bh);
            display.setCursor(plot_x - bw - 6, ty - (bh / 2));
            display.print(tick.label.c_str());
        }
    }

    if (!data_points.empty()) {
        int prev_x = -1, prev_y = -1;
        for (size_t i = 0; i < data_points.size(); ++i) {
            int cx = map_x(data_points[i].x);
            int cy = map_y(data_points[i].y);

            if (show_stems) display.drawLine(cx, cy, cx, plot_y + plot_h, border_col);
            if (show_line && i > 0) display.drawLine(prev_x, prev_y, cx, cy, border_col);
            if (show_points) display.fillCircle(cx, cy, 2, border_col);

            if (show_point_labels) {
                char lbl[10]; snprintf(lbl, sizeof(lbl), "%.1f", data_points[i].y);
                int16_t bx, by; uint16_t bw, bh;
                display.getTextBounds(lbl, 0, 0, &bx, &by, &bw, &bh);
                display.setCursor(cx - (bw / 2), cy - bh - 4);
                display.print(lbl);
            }
            prev_x = cx; prev_y = cy;
        }
    }
}