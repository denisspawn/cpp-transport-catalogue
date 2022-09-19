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

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
/*
class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};
*/
