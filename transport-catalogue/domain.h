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