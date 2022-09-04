#pragma once
#include "geo.h"
#include <vector>
#include <string>
#include <map>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <set>

namespace transport_catalogue {
    namespace details {
        struct Stop {
            std::string title;
            Coordinates coords;
        };

        struct StopInfo {
            bool search_status;
            std::set<std::string_view> buses;
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
    } // namespace details

    using namespace details;
    class TransportCatalogue {
    public:
        void AddBus(Bus bus);
        void AddStop(Stop stop);
        void AddStopDistances(StopDistances stop_distances);
        const Bus* SearchBus(const std::string& title);
        const Stop* SearchStop(const std::string& title);
        BusInfo GetBusInfo(const std::string& title);
        StopInfo GetStopInfo(const std::string& title);
        int GetStopsDistance(const Stop* from, const Stop* to);
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

        size_t CountUniqueStops(const std::vector<const Stop*>& stops);
        double GetRouteCircleLength(const Bus* bus);
        double GetRouteDirectLength(const Bus* bus);
        double GetRouteGeoLength(const Bus* bus);
    };
} // namespace transport_catalogue