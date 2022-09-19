#include "transport_catalogue.h"

namespace transport_catalogue {
using namespace std::literals;

void TransportCatalogue::AddStop(Stop stop) {
    Stop* added_stop = &(stops_.emplace_back(stop));
    title_to_stop_.insert({added_stop->title, added_stop});

    stop_to_buses_[added_stop];
}

const Stop* TransportCatalogue::SearchStop(const std::string& title) const {
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

const Bus* TransportCatalogue::SearchBus(const std::string& title) const {
    auto it = title_to_bus_.find(title);
    if (it == title_to_bus_.end())
        return nullptr;
    return it->second;
}

int TransportCatalogue::GetStopsDistance(const Stop* from, const Stop* to) const {
    auto distance_it = distances_between_stops_.find({from, to});
    if (distance_it != distances_between_stops_.end())
        return distance_it->second;
    return 0;
}
    
const std::unordered_set<const Bus*>* TransportCatalogue::GetStopBuses(const Stop* stop) const {
    auto it = stop_to_buses_.find(stop);
    if (it != stop_to_buses_.end()) {
        return &(it->second);
    }
    return nullptr;
}


void TransportCatalogue::AddStopDistances(StopDistances stop_distances) {
    const Stop* stop_from = SearchStop(stop_distances.title);
    for (auto& [to_stop_title, distance] : stop_distances.stop_to_distance) {
        const Stop* stop_to = SearchStop(to_stop_title);
        distances_between_stops_.insert({{stop_from, stop_to}, distance});
    }
}
    
} // namespace transport_catalogue
