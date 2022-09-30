#include "request_handler.h"

namespace transport_catalogue {
namespace request_handler {
    
using namespace std::literals;
    
size_t RequestHandler::CountUniqueStops(const std::vector<const Stop*>& stops) const {
    std::unordered_set<std::string> unique_names;
    for (const Stop* stop : stops) {
        unique_names.insert(stop->title);
    }
    return unique_names.size();
}
    
double RequestHandler::GetRouteCircleLength(const Bus* bus) const {
    double route_length = 0.0;
    if (!bus->stops.size())
        return route_length;
    for (auto from = bus->stops.begin(); ; ++from) {
        auto to = std::next(from);
        if (to == bus->stops.end())
            break;
        const Stop* stop_from = *(from);
        const Stop* stop_to = *(to);

        int distance = db_.GetStopsDistance(stop_from, stop_to);
        if (distance == 0) distance = db_.GetStopsDistance(stop_to, stop_from);

        if (distance > 0) {
            route_length += distance;
        } else {
            route_length += ComputeDistance(stop_from->coords, stop_to->coords);
        }
    }
    return route_length;
}
    
double RequestHandler::GetRouteDirectLength(const Bus* bus) const {
    double route_length = 0.0;
    if (!bus->stops.size())
        return route_length;
    for (auto from = bus->stops.begin(); ; ++from) {
        auto to = std::next(from);
        if (to == bus->stops.end())
            break;
        const Stop* stop_from = *(from);
        const Stop* stop_to = *(to);

        int distances_from_to = db_.GetStopsDistance(stop_from, stop_to);
        int distances_to_from = db_.GetStopsDistance(stop_to, stop_from);

        if (distances_from_to > 0) {
            route_length += distances_from_to;
            if (distances_to_from > 0) {
                route_length += distances_to_from;
            } else {
                route_length += distances_from_to;
            }
        } else if (distances_to_from > 0) {
            route_length += (distances_to_from * 2);
        } else {
            route_length += (ComputeDistance(stop_from->coords, stop_to->coords) * 2);
        }
    }

    return route_length;
}
    
double RequestHandler::GetRouteGeoLength(const Bus* bus) const {
    double route_geo_length = 0.0;
    if (!bus->stops.size())
        return route_geo_length;

    for (auto from = bus->stops.begin(); ; ++from) {
        auto to = std::next(from);
        if (to == bus->stops.end())
            break;
        const Stop* stop_from = *(from);
        const Stop* stop_to = *(to);
        route_geo_length += ComputeDistance(stop_from->coords, stop_to->coords);
    }
    if (!bus->is_circular)
        route_geo_length *= 2;

    return route_geo_length;
}
    
std::optional<BusInfo> RequestHandler::GetBusStat(const std::string& bus_name) const {
    BusInfo bus_info;

    const Bus* bus_p;
    bus_p = db_.SearchBus(bus_name);

    if (bus_p != nullptr) {
        if (bus_p->is_circular) {
            bus_info.stops_on_route = bus_p->stops.size();
            bus_info.unique_stops = CountUniqueStops(bus_p->stops);
            bus_info.route_length = GetRouteCircleLength(bus_p);
            bus_info.curvature = bus_info.route_length / GetRouteGeoLength(bus_p);
            return bus_info;
        }
        if (!bus_p->is_circular) {
            bus_info.stops_on_route = (bus_p->stops.size() * 2) - 1;
            bus_info.unique_stops = CountUniqueStops(bus_p->stops);
            bus_info.route_length = GetRouteDirectLength(bus_p);
            bus_info.curvature = bus_info.route_length / GetRouteGeoLength(bus_p);
            return bus_info;
        }
    }

    return std::nullopt;
}
    
const std::unordered_set<const Bus*>* RequestHandler::GetBusesByStop(const std::string& stop_name) const {
    const Stop* stop_p;
    stop_p = db_.SearchStop(stop_name);
    
    if (stop_p == nullptr)
        return nullptr;

    return db_.GetStopBuses(stop_p);
}
    
const std::map<std::string_view, Bus*>& RequestHandler::GetTitleToBus() const {
    return db_.GetTitleToBus();
}
    
const svg::Document& RequestHandler::RenderMap() const {
    return renderer_.GetRenderDocument();
}
    
const map_renderer::MapRenderer& RequestHandler::GetMapRenderer() const {
    return renderer_;
}

} // namespace handler
} // namespace transport_catalogue