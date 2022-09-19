#include "map_renderer.h"

namespace map_renderer {

void MapRenderer::PrepareSVGDoc(const RenderSettings* r_set,
                                const std::map<std::string_view, domain::Bus*>& title_to_bus) {
    // инициализируем настройки
    r_set_ = r_set;
    // Заполняем непустые маршруты
    FillNonEmptyBuses(title_to_bus);
    // Заполняем координаты
    FillGeoCoords(title_to_bus);
    // Создаём проектор сферических координат на карту
    const SphereProjector proj = MakeSphereProjector();

    // нужно добавить все объекты в порядке очередности.
    // 1. линии маршрутов
    AddSVGBuses(proj);
    // 2. названия маршрутов,
    GetSVGBusTitles(proj);
    // 3. круги, обозначающие остановки,
    GetSVGStopCircles(proj);
    // 4. названия остановок.
    GetSVGStopTitles(proj);
}

const svg::Document& MapRenderer::GetRenderDocument() const {
    return doc_;
}

void MapRenderer::FillNonEmptyBuses(const std::map<std::string_view, domain::Bus*>& title_to_bus) {
    for (const auto [title, bus] : title_to_bus) {
        if (!bus->stops.empty())
            non_empty_buses_.push_back(bus);
    }
}
    
void MapRenderer::FillGeoCoords(const std::map<std::string_view, domain::Bus*>& title_to_bus) {
    for (const auto [title, bus] : title_to_bus) {
        for (const auto& stop : bus->stops) {
            geo_coords_.push_back(stop->coords);
        }
    }
}
    
const SphereProjector MapRenderer::MakeSphereProjector() {
    const double WIDTH = r_set_->width;
    const double HEIGHT = r_set_->height;
    const double PADDING = r_set_->padding;
    return {
        geo_coords_.begin(), geo_coords_.end(), WIDTH, HEIGHT, PADDING
    };
}
    
void MapRenderer::SetCommonTextBusProp(svg::Text& text, 
                          const svg::Point screen_coord,
                          const std::string& title) const {
    text.SetPosition(screen_coord);
    text.SetOffset({r_set_->bus_label_offset[0], r_set_->bus_label_offset[1]});
    text.SetFontSize(r_set_->bus_label_font_size);
    text.SetFontFamily("Verdana"s);
    text.SetFontWeight("bold"s);
    text.SetData(title);
}
    
void MapRenderer::SetAddLayerTextBusProp(svg::Text& text) const {
    text.SetFillColor(r_set_->underlayer_color);
    text.SetStrokeColor(r_set_->underlayer_color);
    text.SetStrokeWidth(r_set_->underlayer_width);
    text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}
    
void MapRenderer::SetCommonTextStopProp(svg::Text& text, 
                           const svg::Point screen_coord,
                           const std::string& title) const {
    text.SetPosition(screen_coord);
    text.SetOffset({r_set_->stop_label_offset[0], r_set_->stop_label_offset[1]});
    text.SetFontSize(r_set_->stop_label_font_size);
    text.SetFontFamily("Verdana"s);
    text.SetData(title);
}
    
void MapRenderer::SetAddLayerTextStopProp(svg::Text& text) const {
    text.SetFillColor(r_set_->underlayer_color);
    text.SetStrokeColor(r_set_->underlayer_color);
    text.SetStrokeWidth(r_set_->underlayer_width);
    text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}
    
void MapRenderer::AddSVGBuses(const SphereProjector& proj) {
    std::vector<svg::Polyline> svg_buses;
    size_t color_count = 0;
    for (const auto bus : non_empty_buses_) {
        // если на маршруте нет остановок, то его нужно пропустить
        // не нужно, так как отобраны маршруты только с остановками
        svg::Polyline svg_bus;
        svg_bus.SetFillColor(svg::NoneColor);

        if (color_count >= r_set_->color_palette.size())
            color_count = 0;

        svg_bus.SetStrokeColor(r_set_->color_palette[color_count++]);
        svg_bus.SetStrokeWidth(r_set_->line_width);
        svg_bus.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        svg_bus.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        // Проецируем координаты
        for (const auto stop : bus->stops) {
            const svg::Point screen_coord = proj(stop->coords);
            svg_bus.AddPoint(screen_coord);
        }
        // Если маршрут не кольцевой, то нужно нарисовать маршрут и в обратном направлении
        if (!bus->is_circular) {
            for (auto it = bus->stops.rbegin() + 1; it != bus->stops.rend(); ++it) {
                const svg::Point screen_coord = proj((*(it))->coords);
                svg_bus.AddPoint(screen_coord);
            }
        }
        doc_.Add(std::move(svg_bus));
    }
}
    
void MapRenderer::GetSVGBusTitles(const SphereProjector& proj) {
    size_t color_count = 0;
    for (const auto bus : non_empty_buses_) {
        if (color_count >= r_set_->color_palette.size())
            color_count = 0;

        svg::Text svg_underlayer;
        svg::Text svg_title;

        const svg::Point screen_coord = proj(bus->stops.front()->coords);
        SetCommonTextBusProp(svg_underlayer, screen_coord, bus->title);
        SetCommonTextBusProp(svg_title, screen_coord, bus->title);
        SetAddLayerTextBusProp(svg_underlayer);
        svg_title.SetFillColor(r_set_->color_palette[color_count]);
        //добавляем в правильном порядке в массив
        doc_.Add(std::move(svg_underlayer));
        doc_.Add(std::move(svg_title));

        // если маршрут некольцевой и конечные не совпадают, — для второй конечной.
        if (!bus->is_circular && bus->stops.front() != bus->stops.back()) {
            const svg::Point screen_coord = proj(bus->stops.back()->coords);
            SetCommonTextBusProp(svg_underlayer, screen_coord, bus->title);
            SetCommonTextBusProp(svg_title, screen_coord, bus->title);
            SetAddLayerTextBusProp(svg_underlayer);
            svg_title.SetFillColor(r_set_->color_palette[color_count]);
            //добавляем в правильном порядке в массив
            doc_.Add(std::move(svg_underlayer));
            doc_.Add(std::move(svg_title));
        }
        ++color_count;
    }
}
    
void MapRenderer::GetSVGStopCircles(const SphereProjector& proj) {
    std::map<std::string, svg::Circle> title_to_stop_circles;
    for (const auto bus : non_empty_buses_) {
        svg::Circle svg_stop_circle;

        for (const auto stop : bus->stops) {
            const svg::Point screen_coord = proj(stop->coords);
            svg_stop_circle.SetCenter(screen_coord);
            svg_stop_circle.SetRadius(r_set_->stop_radius);
            svg_stop_circle.SetFillColor("white"s);
            title_to_stop_circles.insert({stop->title, svg_stop_circle});
        }
    }

    for (auto& [title, circle] : title_to_stop_circles) {
        doc_.Add(std::move(circle));
    }
}
    
void MapRenderer::GetSVGStopTitles(const SphereProjector& proj) {
    std::map<std::string, std::vector<svg::Text>> title_to_stops;
    for (const auto bus : non_empty_buses_) {
        for (const auto stop : bus->stops) {
            svg::Text svg_underlayer;
            svg::Text svg_title;
            const svg::Point screen_coord = proj(stop->coords);
            SetCommonTextStopProp(svg_underlayer, screen_coord, stop->title);
            SetCommonTextStopProp(svg_title, screen_coord, stop->title);
            SetAddLayerTextStopProp(svg_underlayer);
            svg_title.SetFillColor("black"s);
            //добавляем в правильном порядке в массив
            if (!title_to_stops.count(stop->title)) {
                title_to_stops[stop->title].push_back(svg_underlayer);
                title_to_stops[stop->title].push_back(svg_title);
            }
        }
    }

    for (const auto& [title, titles_layers]: title_to_stops) {
        for (const auto& title_layer : titles_layers) {
            doc_.Add(std::move(title_layer));
        }
    }
}
    
} // namespace map_renderer
/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */