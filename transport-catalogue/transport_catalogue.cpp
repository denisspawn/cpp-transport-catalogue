#include "transport_catalogue.h"

using namespace std::literals;

namespace transport_catalogue {
    void TransportCatalogue::AddStop(Stop stop) {
        Stop* added_stop = &(stops_.emplace_back(stop));
        title_to_stop_.insert({added_stop->title, added_stop});

        stop_to_buses_[added_stop];
    }

    const Stop* TransportCatalogue::SearchStop(const std::string& title) {
        auto it = title_to_stop_.find(title);
        if (it == title_to_stop_.end())
            return nullptr;
        return it->second;
    }

    void TransportCatalogue::AddBus(Bus bus) {
        Bus* added_bus = &(buses_.emplace_back(bus));
        title_to_bus_.insert({added_bus->title, added_bus});

        for (const Stop* stop : added_bus->stops) {
            stop_to_buses_[stop].insert(added_bus);
        }
    }

    const Bus* TransportCatalogue::SearchBus(const std::string& title) {
        auto it = title_to_bus_.find(title);
        if (it == title_to_bus_.end())
            return nullptr;
        return it->second;
    }

    size_t TransportCatalogue::CountUniqueStops(const std::vector<const Stop*>& stops) {
        std::unordered_set<std::string> unique_names;
        for (const Stop* stop : stops) {
            unique_names.insert(stop->title);
        }
        return unique_names.size();
    }

    int TransportCatalogue::GetStopsDistance(const Stop* from, const Stop* to) {
        auto distance_it = distances_between_stops_.find({from, to});
        if (distance_it != distances_between_stops_.end())
            return distance_it->second;
        return 0;
    }

    double TransportCatalogue::GetRouteCircleLength(const Bus* bus) {
        double route_length = 0.0;
        if (!bus->stops.size())
            return route_length;
        for (auto from = bus->stops.begin(); ; ++from) {
            auto to = std::next(from);
            if (to == bus->stops.end())
                break;
            const Stop* stop_from = *(from);
            const Stop* stop_to = *(to);

            int distance = GetStopsDistance(stop_from, stop_to);
            if (distance == 0) distance = GetStopsDistance(stop_to, stop_from);

            if (distance > 0) {
                route_length += distance;
            } else {
                route_length += ComputeDistance(stop_from->coords, stop_to->coords);
            }
        }
        return route_length;
    }

    double TransportCatalogue::GetRouteDirectLength(const Bus* bus) {
        double route_length = 0.0;
        if (!bus->stops.size())
            return route_length;
        for (auto from = bus->stops.begin(); ; ++from) {
            auto to = std::next(from);
            if (to == bus->stops.end())
                break;
            const Stop* stop_from = *(from);
            const Stop* stop_to = *(to);

            int distances_from_to = GetStopsDistance(stop_from, stop_to);
            int distances_to_from = GetStopsDistance(stop_to, stop_from);

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

    double TransportCatalogue::GetRouteGeoLength(const Bus* bus) {
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

    BusInfo TransportCatalogue::GetBusInfo(const std::string& title) {
        static BusInfo bus_info;
        bus_info = {0, 0, 0.0, 0.0};

        const Bus* bus_p;
        bus_p = SearchBus(title);

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

        return bus_info;
    }

    StopInfo TransportCatalogue::GetStopInfo(const std::string& title) {
        static StopInfo stop_info;
        stop_info = { false, {} };

        const Stop* stop_p;
        stop_p = SearchStop(title);

        if (stop_p != nullptr) {
            auto it = stop_to_buses_.find(stop_p);
            if (it != stop_to_buses_.end()) {
                stop_info.search_status = true;
                for (const auto& bus : it->second) {
                    stop_info.buses.insert(bus->title);
                }
                return stop_info;
            }
        }

        return stop_info;
    }

    void TransportCatalogue::AddStopDistances(StopDistances stop_distances) {
        const Stop* stop_from = SearchStop(stop_distances.title);
        for (auto& [to_stop_title, distance] : stop_distances.stop_to_distance) {
            const Stop* stop_to = SearchStop(to_stop_title);
            distances_between_stops_.insert({{stop_from, stop_to}, distance});
        }
    }
} // namespace transport_catalogue
