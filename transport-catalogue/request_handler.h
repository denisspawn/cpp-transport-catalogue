#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h" //сказано, что не должен зависеть от него, но без этого инклюда не передать параметр в конструктор
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

namespace transport_catalogue {
namespace request_handler {
using namespace domain;

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    //RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);
    RequestHandler(const TransportCatalogue& db, const map_renderer::MapRenderer& renderer) 
        : db_(db)
        , renderer_(renderer) {};

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusInfo> GetBusStat(const std::string& bus_name) const;
    const std::unordered_set<const Bus*>* GetBusesByStop(const std::string& stop_name) const;
    const std::map<std::string_view, Bus*>& GetTitleToBus() const;
    const svg::Document& RenderMap() const;
    const map_renderer::MapRenderer& GetMapRenderer() const;
    
private:
    size_t CountUniqueStops(const std::vector<const Stop*>& stops) const;
    double GetRouteCircleLength(const Bus* bus) const;
    double GetRouteDirectLength(const Bus* bus) const;
    double GetRouteGeoLength(const Bus* bus) const;
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& db_;
    const map_renderer::MapRenderer& renderer_;
};
    
} // namespace request_handler
} // namespace transport_catalogue