#include "stat_reader.h"


namespace transport_catalogue {
    namespace output {
        using namespace std::literals;

        void ProcessOutputRequest(TransportCatalogue& transport_catalogue, std::istream& input, std::ostream& output) {
            std::string line;
            getline(input, line);
            int queries_count = std::stoi(line);
            BusInfo bus_info;
            StopInfo stop_info;

            while (queries_count) {
                getline(input, line);
                int64_t pos = line.find_first_of(' ');
                std::string request_type = line.substr(0, line.find_first_of(' '));
                std::string title = line.substr(pos + 1);

                if (request_type == "Bus"s) {
                    bus_info = transport_catalogue.GetBusInfo(title);
                    if (bus_info.stops_on_route != 0) {
                        output << "Bus "s << title << ": "s 
                                  << bus_info.stops_on_route << " stops on route, "s
                                  << bus_info.unique_stops << " unique stops, "s
                                  << std::setprecision(6) << bus_info.route_length << " route length, "s
                                  << bus_info.curvature << " curvature"s
                                  << std::endl;
                    } else {
                        output << "Bus "s << title << ": not found"s << std::endl;
                    }
                }
                if (request_type == "Stop"s) {
                    stop_info = transport_catalogue.GetStopInfo(title);
                    if (stop_info.search_status) {
                        if (stop_info.buses.size() == 0) {
                            output << "Stop "s << title << ": no buses"s << std::endl;
                        } else {
                            output << "Stop "s << title << ": buses"s;
                            for (const auto& bus_title : stop_info.buses) {
                               output << ' ' << bus_title;
                            }
                            output << std::endl;
                        }
                    } else {
                        output << "Stop "s << title << ": not found"s << std::endl;
                    }
                }
                queries_count--;
            }

        }
    } // namespace output
} // namespace transport_catalogue
