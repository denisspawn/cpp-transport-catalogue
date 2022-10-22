#pragma once

#include "domain.h"
#include "svg.h"
#include "geo.h"
#include "request_handler.h"

#include <vector>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <set>
#include <string_view>
#include <sstream>

using namespace std::literals::string_literals;

struct RenderSettings {
    double width = 600.0;
    double height = 400.0;
    double padding = 50.0;
    double line_width = 14.0;
    double stop_radius = 5.0;
    int bus_label_font_size = 20;
    svg::Point bus_label_offset;
    int stop_label_font_size = 20;
    svg::Point stop_label_offset;
    svg::Color underlayer_color = svg::Rgba{ 255, 255, 255, 0.85 };
    double underlayer_width = 3.0;
    std::vector<svg::Color> color_palette;
};

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }

    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRenderer {
public:
    explicit MapRenderer(RenderSettings& settings, RequestHandler& rh);

    void DrawBusRouteLines();
    void AddBusNames();
    void DrawStopCircles();
    void AddStopNames();
    std::string DrawMap();

private:
    const RenderSettings& settings_;
    const RequestHandler& rh_;
    svg::Document image_;
    std::map<std::string_view, Bus*> buses_;
    std::map<std::string_view, Stop*> stops_;

    std::vector<geo::Coordinates> geo_coords_;
    std::vector<geo::Coordinates> CollectCoordinates();

};