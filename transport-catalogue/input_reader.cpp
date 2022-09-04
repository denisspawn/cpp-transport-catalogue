#include "input_reader.h"

using namespace std::literals;

namespace transport_catalogue {
    namespace input {
        Stop SplitStopQuery(const std::string& stop_query) {
            int64_t pos = stop_query.find_first_not_of(' ');

            Stop stop;
            int64_t end;
            if (stop_query.find(':') != std::string::npos) {
                end = stop_query.find(':');
                stop.title = stop_query.substr(pos, end);
                pos = stop_query.find_first_not_of(' ', end + 1);
            }
            if (stop_query.find(',') != std::string::npos) {
                end = stop_query.find(',');
                stop.coords.lat = stod(stop_query.substr(pos, end - pos));
                pos = stop_query.find_first_not_of(' ', end + 1);
            }
            stop.coords.lng = stod(stop_query.substr(pos));
            return stop;
        }

        StopDistances SplitStopQueryDistance(const std::string& stop_query) {
            long unsigned int pos = stop_query.find_first_not_of(' ');

            StopDistances stop_distances;
            long unsigned int end;
            if (stop_query.find(':') != std::string::npos) {
                end = stop_query.find(':');
                stop_distances.title = stop_query.substr(pos, end);
                int64_t first_comma = stop_query.find(',') + 1;
                pos = stop_query.find(',', first_comma) + 2;
            }
            while (pos != std::string::npos) {
                int distance = stoi(stop_query.substr(pos));
                pos = stop_query.find(" to ", pos) + 4;
                end = stop_query.find(',', pos);
                if (end != std::string::npos) {
                    stop_distances.stop_to_distance.insert({stop_query.substr(pos, end - pos), {distance}});
                    pos = end + 1;
                } else {
                    stop_distances.stop_to_distance.insert({stop_query.substr(pos, end - pos), {distance}});
                    pos = end;
                }
            }

            return stop_distances;
        }

        Bus SplitBusQuery(TransportCatalogue& transport_catalogue, const std::string& bus_query) {
            int64_t pos = bus_query.find_first_not_of(' ');

            Bus bus;
            int64_t end;
            if (bus_query.find(':') != std::string::npos) {
                end = bus_query.find(':');
                bus.title = bus_query.substr(pos, end);
                pos = bus_query.find_first_not_of(' ', end + 1);
            }
            char curr_separ = bus_query.find('-') != std::string::npos ? '-' : '>';
            while (bus_query.find(curr_separ, pos) != std::string::npos) {
                end = bus_query.find(curr_separ, pos);
                std::string bus_stop_title = bus_query.substr(pos, (end - 1) - pos);
                bus.stops.push_back(transport_catalogue.SearchStop(bus_stop_title));
                bus.is_circular = curr_separ == '>' ? true : false;
                pos = bus_query.find_first_not_of(' ', end + 1);
            }
            std::string last_stop_name = bus_query.substr(pos);
            bus.stops.push_back(transport_catalogue.SearchStop(last_stop_name));
            return bus;
        }

        void ProcessQueries(TransportCatalogue& transport_catalogue, Queries& queries) {
            // first of all, we should process Stop requests.
            for (const std::string& stop_query : queries.stop_queries) {
                transport_catalogue.AddStop(SplitStopQuery(stop_query));
            }
            // than we can process rest of queries (distances should be processed after processing of stops)
            for (const std::string& stop_query : queries.stop_queries) {
                if (std::count(stop_query.begin(), stop_query.end(), ',') > 1)
                    transport_catalogue.AddStopDistances(SplitStopQueryDistance(stop_query));
            }
            for (const std::string& bus_query : queries.bus_queries) {
                transport_catalogue.AddBus(SplitBusQuery(transport_catalogue, bus_query));
            }
        }

        void ProcessInputRequest(TransportCatalogue& transport_catalogue, std::istream& input) {
            std::string line;
            getline(input, line);
            int queries_count = std::stoi(line);

            Queries queries;
            while (queries_count) {
                getline(input, line);
                std::string query_type = line.substr(0, line.find_first_of(' '));
                if (query_type == "Stop")
                    queries.stop_queries.insert(line.substr(query_type.size() + 1));
                else if (query_type == "Bus")
                    queries.bus_queries.insert(line.substr(query_type.size() + 1));
                queries_count--;
            }
            ProcessQueries(transport_catalogue, queries);
        }
    } // namespace input
} // namespace transport_catalogue
