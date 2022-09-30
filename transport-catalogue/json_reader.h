#pragma once

#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"
#include "map_renderer.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
#include <utility>

namespace transport_catalogue {
namespace json_reader {
        
class JSONReader {
public:
    JSONReader(TransportCatalogue& tr_cat, request_handler::RequestHandler& req_nadler)
        : tr_cat_(tr_cat)
        , req_hand_(req_nadler) {}

    void ProcessRequest(std::istream& input);
    const json::Array& GetStatRespose() const;
    const map_renderer::RenderSettings* GetRenderSettings() const;
    
private:
    Stop MakeStop(const json::Dict& stop_query);
    StopDistances MakeStopDistance(const json::Dict& stop_query);
    Bus MakeBus(const json::Dict& bus_query);
    void ProcessBaseRequest(const json::Array& base_requests);
    void ProcessStatRequest(const json::Array& base_requests);
    void ProcessRenderSettingRequest(const json::Dict& stat_requests);
    svg::Color CooseColorFormat(const json::Node color) const;

    TransportCatalogue& tr_cat_;
    request_handler::RequestHandler& req_hand_;
    
    json::Document queries_;
    // JSON format respons for "stat_requests"
    json::Array stat_response_;
    // JSON processed "render_settings"
    map_renderer::RenderSettings r_set_;
};
        
} // namespace json
} // namespace transport_catalogue