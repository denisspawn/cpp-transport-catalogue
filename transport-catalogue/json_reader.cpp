#include "json_reader.h"

namespace transport_catalogue {
namespace json_reader {
    
using namespace std::literals;

Stop JSONReader::MakeStop(const json::Dict& stop_query) {
    domain::Stop stop;
    stop.title = stop_query.at("name"s).AsString();
    stop.coords.lat = stop_query.at("latitude"s).AsDouble();
    stop.coords.lng = stop_query.at("longitude"s).AsDouble();
    return stop;
}

StopDistances JSONReader::MakeStopDistance(const json::Dict& stop_query) {
    domain::StopDistances stop_distances;
    stop_distances.title = stop_query.at("name"s).AsString();
    if (stop_query.count("road_distances"s)) {
        for (const auto& [name, distance] : stop_query.at("road_distances"s).AsDict()) {
            stop_distances.stop_to_distance.insert({name, {distance.AsInt()}});
        }
    }
    return stop_distances;
}

Bus JSONReader::MakeBus(const json::Dict& bus_query) {
    domain::Bus bus;
    bus.title = bus_query.at("name"s).AsString();
    if (bus_query.count("is_roundtrip"s)) {
        bus.is_circular = bus_query.at("is_roundtrip"s).AsBool();
    }
    if (bus_query.count("stops"s)) {
        for (const auto& stop_title : bus_query.at("stops"s).AsArray()) {
           bus.stops.push_back(tr_cat_.SearchStop(stop_title.AsString())); 
        }
    }
    return bus;
}

void JSONReader::ProcessBaseRequest(const json::Array& base_requests) {
    // first of all, we should process Stop requests.
    for (const auto& request : base_requests) {
        const json::Dict& map_base_requests = request.AsDict();
        if (map_base_requests.at("type"s) == "Stop"s) {
            tr_cat_.AddStop(MakeStop(map_base_requests));
        }
    }
    // than we can process rest of queries (distances should be processed after processing of stops)
    for (const auto& request : base_requests) {
        const json::Dict& map_base_requests = request.AsDict();
        if (map_base_requests.count("road_distances"s)) {
            tr_cat_.AddStopDistances(MakeStopDistance(map_base_requests));
        }
    }
    for (const auto& request : base_requests) {
        const json::Dict& map_base_requests = request.AsDict();
        if (map_base_requests.at("type"s) == "Bus"s) {
            tr_cat_.AddBus(MakeBus(map_base_requests));
        }
    }
}

void JSONReader::ProcessStatRequest(const json::Array& stat_requests) {
    stat_response_.reserve(stat_requests.size());

    for (const auto& request : stat_requests) {
        const json::Dict& map_stat_request = request.AsDict();
        std::optional<domain::BusInfo> bus_info;
        const std::unordered_set<const Bus*>* buses_by_stop;
        std::string title;
        
        if (map_stat_request.at("type"s) == "Bus"s) {
            title = map_stat_request.at("name"s).AsString();
            bus_info = req_hand_.GetBusStat(title);
            if (bus_info != std::nullopt) {
                stat_response_.emplace_back(json::Dict{
                    {"curvature"s, bus_info->curvature},
                    {"request_id"s, map_stat_request.at("id"s).AsInt()},
                    {"route_length"s, bus_info->route_length},
                    {"stop_count"s, static_cast<int>(bus_info->stops_on_route)},
                    {"unique_stop_count"s, static_cast<int>(bus_info->unique_stops)},
                });
            } else {
                stat_response_.emplace_back(json::Dict{
                    {"request_id"s, map_stat_request.at("id"s).AsInt()},
                    {"error_message"s, "not found"s},
                });
            }

        } else if (map_stat_request.at("type"s) == "Stop"s) {
            title = map_stat_request.at("name"s).AsString();
            buses_by_stop = req_hand_.GetBusesByStop(title);
            if (buses_by_stop != nullptr) {
                json::Array buses;
                buses.reserve(buses_by_stop->size());
                for (const auto bus : *buses_by_stop) {
                    buses.push_back(std::string(bus->title));
                }
                // нужно для упорядоченного отображения маршрутов
                sort(buses.begin(), buses.end(),
                    [](const auto& lhs, const auto& rhs) {
                    return lhs.AsString() < rhs.AsString();
                });
                stat_response_.emplace_back(json::Dict{
                    {"buses"s, buses},
                    {"request_id"s, map_stat_request.at("id"s).AsInt()},
                });
            } else {
                stat_response_.emplace_back(json::Dict{
                    {"request_id"s, map_stat_request.at("id"s).AsInt()},
                    {"error_message"s, "not found"s},
                });
            }
        } else if (map_stat_request.at("type"s) == "Map"s) {
            map_renderer::MapRenderer& map_renderer = const_cast<map_renderer::MapRenderer&>(req_hand_.GetMapRenderer());
            map_renderer.PrepareSVGDoc(GetRenderSettings(), req_hand_.GetTitleToBus());
            const svg::Document& doc = req_hand_.RenderMap();
            std::ostringstream out;
            doc.Render(out);
            std::string str_map = out.str();
            //std::cout << resp << std::endl;
            //doc.Render(std::cout);
            stat_response_.emplace_back(json::Dict{
                {"map"s, str_map},
                {"request_id"s, map_stat_request.at("id"s).AsInt()},
            });
        }
    }
}
    
svg::Color JSONReader::CooseColorFormat(const json::Node color) const {
    if (color.IsArray()) {
        json::Array color_arr = color.AsArray();
        if (color_arr.size() == 3) {
            svg::Rgb rgb{static_cast<uint8_t>(color_arr[0].AsInt()),
                         static_cast<uint8_t>(color_arr[1].AsInt()),
                         static_cast<uint8_t>(color_arr[2].AsInt())
                        };
            return rgb;
        }
        if (color_arr.size() == 4) {
            svg::Rgba rgba{static_cast<uint8_t>(color_arr[0].AsInt()),
                           static_cast<uint8_t>(color_arr[1].AsInt()),
                           static_cast<uint8_t>(color_arr[2].AsInt()),
                           color_arr[3].AsDouble()
                          };
            return rgba;
        }
    } else if (color.IsString()) {
        return color.AsString();
    }
    return {};
}
    
void JSONReader::ProcessRenderSettingRequest(const json::Dict& setting_requests) {
    r_set_.width = setting_requests.at("width"s).AsDouble();
    r_set_.height = setting_requests.at("height"s).AsDouble();
    r_set_.padding = setting_requests.at("padding"s).AsDouble();
    r_set_.stop_radius = setting_requests.at("stop_radius"s).AsDouble();
    r_set_.line_width = setting_requests.at("line_width"s).AsDouble();
    
    r_set_.bus_label_font_size = setting_requests.at("bus_label_font_size"s).AsInt();
    for (const auto& num : setting_requests.at("bus_label_offset"s).AsArray()) {
        r_set_.bus_label_offset.push_back(num.AsDouble());
    }
    
    r_set_.stop_label_font_size = setting_requests.at("stop_label_font_size"s).AsInt();
    for (const auto& num : setting_requests.at("stop_label_offset"s).AsArray()) {
        r_set_.stop_label_offset.push_back(num.AsDouble());
    }
    
    r_set_.underlayer_color = CooseColorFormat(setting_requests.at("underlayer_color"s));
    
    r_set_.underlayer_width = setting_requests.at("underlayer_width"s).AsDouble();
    
    for (const auto& color : setting_requests.at("color_palette"s).AsArray()) {
        r_set_.color_palette.push_back(CooseColorFormat(color));
    }
}

void JSONReader::ProcessRequest(std::istream& input) {
    queries_ = json::Load(input);
    const json::Dict& requests = queries_.GetRoot().AsDict();
    ProcessBaseRequest(requests.at("base_requests"s).AsArray());
    ProcessRenderSettingRequest(requests.at("render_settings"s).AsDict());
    ProcessStatRequest(requests.at("stat_requests"s).AsArray());
}
    
const json::Array& JSONReader::GetStatRespose() const {
    return stat_response_;
}
    
const map_renderer::RenderSettings* JSONReader::GetRenderSettings() const {
    return &r_set_;
}
    
} // namespace json
} // namespace transport_catalogue