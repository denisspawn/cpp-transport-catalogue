#pragma once

#include "domain.h"
#include <vector>
#include <string>
#include <map>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <set>

namespace transport_catalogue {
using namespace domain;
    
class TransportCatalogue {
public:
    void AddBus(Bus bus);
    void AddStop(Stop stop);
    void AddStopDistances(StopDistances stop_distances);
    const Bus* SearchBus(const std::string& title) const;
    const Stop* SearchStop(const std::string& title) const;
    int GetStopsDistance(const Stop* from, const Stop* to) const;
    const std::unordered_set<const Bus*>* GetStopBuses(const Stop* stop) const;
    const std::map<std::string_view, Bus*>& GetTitleToBus() const {
        return title_to_bus_;
    }
private:
    std::deque<Stop> stops_;
    std::map<std::string_view, Stop*> title_to_stop_;
    std::deque<Bus> buses_;
    std::map<std::string_view, Bus*> title_to_bus_;
    std::unordered_map<const Stop*, std::unordered_set<const Bus*>> stop_to_buses_;

    class Hasher {
    private:
        // хэшируем указатель произвольного типа, хотя можно указать и конкретный. В данном случае Stop*
        std::hash<const void*> hasher_;
    public:
        size_t operator()(const std::pair<const Stop*, const Stop*>& stop_pointers) const {
            return static_cast<size_t>(hasher_(stop_pointers.first) + hasher_(stop_pointers.second) * 37 * 37);
        }
    };
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, Hasher> distances_between_stops_;
};
    
} // namespace transport_catalogue
