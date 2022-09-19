#pragma once

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <map>

#include "svg.h"
#include "domain.h"
#include "geo.h"

namespace map_renderer {
using namespace std::literals;
    
struct RenderSettings {
    double width = 0.0;
    double height = 0.0;
    double padding = 0.0;
    double line_width = 0.0;
    double stop_radius = 0.0;
    int bus_label_font_size = 0;
    std::vector<double> bus_label_offset;
    int stop_label_font_size = 0;
    std::vector<double> stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width = 0.0;
    std::vector<svg::Color> color_palette;
}; // нужно заполнить данную структуру в json_reader::ProcessRenderSettingRequest
    
inline const double EPSILON = 1e-6;
inline bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};
    
class MapRenderer {
public:
    MapRenderer() = default;
    
    void PrepareSVGDoc(const RenderSettings* r_set,
                       const std::map<std::string_view, domain::Bus*>& title_to_bus);
    
    const svg::Document& GetRenderDocument() const;
    
private:
    const RenderSettings* r_set_ = nullptr;
    // Точки, подлежащие проецированию
    std::vector<geo::Coordinates> geo_coords_;
    std::vector<domain::Bus*> non_empty_buses_;
    svg::Document doc_;
    
    void FillNonEmptyBuses(const std::map<std::string_view, domain::Bus*>& title_to_bus);
    void FillGeoCoords(const std::map<std::string_view, domain::Bus*>& title_to_bus);
    const SphereProjector MakeSphereProjector();
    void SetCommonTextBusProp(svg::Text& text, 
                              const svg::Point screen_coord,
                              const std::string& title) const;
    
    void SetAddLayerTextBusProp(svg::Text& text) const;
    void SetCommonTextStopProp(svg::Text& text, 
                               const svg::Point screen_coord,
                               const std::string& title) const;
    
    void SetAddLayerTextStopProp(svg::Text& text) const;
    void AddSVGBuses(const SphereProjector& proj);
    void GetSVGBusTitles(const SphereProjector& proj);
    void GetSVGStopCircles(const SphereProjector& proj);
    void GetSVGStopTitles(const SphereProjector& proj);
};
    
} //namespace map_renderer
/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 * request_handler получает данные о маршрутах у транспортного справочника и передаёт их модулю map_renderer.
 */