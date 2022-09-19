#pragma once

#include "geo.h"
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <set>

namespace domain {
    
struct Stop {
    std::string title;
    geo::Coordinates coords;
};

struct StopDistances {
    std::string title;
    std::unordered_map<std::string, int> stop_to_distance;
};

struct Bus {
    std::string title;
    bool is_circular;
    std::vector<const Stop*> stops;
};

struct BusInfo {
    size_t stops_on_route = 0;
    size_t unique_stops = 0;
    double route_length = 0.0;
    double curvature = 0.0;
};
    
} //namespace domain

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */
