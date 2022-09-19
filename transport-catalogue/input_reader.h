#pragma once

#include "transport_catalogue.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
#include <utility>

namespace transport_catalogue {
    namespace input {
        struct Queries {
            std::unordered_set<std::string> stop_queries;
            std::unordered_set<std::string> bus_queries;
        };

        Stop SplitStopQuery(const std::string& stop_query);
        StopDistances SplitStopQueryDistance(const std::string& stop_query);
        Bus SplitBusQuery(TransportCatalogue& transport_catalogue, const std::string& bus_query);
        void ProcessQueries(TransportCatalogue& transport_catalogue, Queries& queries);
        void ProcessInputRequest(TransportCatalogue& transport_catalogue, std::istream& input);
    } // namespace input
} // namespace transport_catalogue
