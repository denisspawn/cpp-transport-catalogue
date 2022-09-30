#include <cassert>
#include <chrono>
#include <sstream>
#include <string_view>

#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "json_builder.h"

/*
int main() {
    
    json::Print(
        json::Document{
            json::Builder{}
            .StartDict()
                .Key("key1"s).Value(123)
                .Key("key2"s).Value("value2"s)
                .Key("key3"s).StartArray()
                    .Value(456)
                    .StartDict().EndDict()
                    .StartDict()
                        .Key(""s)
                        .Value(nullptr)
                    .EndDict()
                    .Value(""s)
                .EndArray()
            .EndDict()
            .Build()
        },
        cout
    );
    cout << endl;
    
    json::Print(
        json::Document{
            json::Builder{}
            .Value("just a string"s)
            .Build()
        },
        cout
    );
    cout << endl;
    
    json::Print(
        json::Document{
            json::Builder{}
            .StartDict()
                .Key("key1"s).Value(123)
                .Key("key2"s).Value("value2"s)
                .Key("key3"s).StartDict()
                    .Key("key1"s).Value(123)
                .EndDict()
                .Key("key4"s).StartArray()
                    .Value(456)
                    .Value(465)
                .EndArray()
            .EndDict()
            .Build()
        },
        cout
    );
    cout << endl;
    
    json::Print(
        json::Document{
            json::Builder{}
            .StartDict()
                .Key("key1"s).StartArray()
                    .Value(456)
                .EndArray()
            .EndDict()
            .Build()
        },
        cout
    );
    cout << endl;
}
*/
int main() {
    /*
     * Примерная структура программы:
     *
     * Считать JSON из stdin
     * Построить на его основе JSON базу данных транспортного справочника
     * Выполнить запросы к справочнику, находящиеся в массиве "stat_requests", построив JSON-массив
     * с ответами.
     * Вывести в stdout ответы в виде JSON
     */
    
    transport_catalogue::TransportCatalogue transport_catalogue;
    map_renderer::MapRenderer map_renderer;
    
    transport_catalogue::request_handler::RequestHandler request_handler(transport_catalogue, map_renderer);
    transport_catalogue::json_reader::JSONReader json_reader(transport_catalogue, request_handler);
    json_reader.ProcessRequest(std::cin);
    
    const json::Array& response = json_reader.GetStatRespose();
    //json::Print(json::Document{response}, std::cout);
    
    json::Print(
        json::Document{
            json::Builder{}
            .Value(response)
            .Build()
        },
        std::cout
    );
    
    //map_renderer.PrepareSVGDoc(json_reader.GetRenderSettings(), 
    //                           request_handler.GetTitleToBus());
    
    //const svg::Document& doc = request_handler.RenderMap();
    //doc.Render(std::cout);
}

/*
{
    "base_requests": [
      {
        "type": "Bus",
        "name": "114",
        "stops": ["Морской вокзал", "Ривьерский мост"],
        "is_roundtrip": false
      },
      {
        "type": "Stop",
        "name": "Ривьерский мост",
        "latitude": 43.587795,
        "longitude": 39.716901,
        "road_distances": {"Морской вокзал": 850}
      },
      {
        "type": "Stop",
        "name": "Морской вокзал",
        "latitude": 43.581969,
        "longitude": 39.719848,
        "road_distances": {"Ривьерский мост": 850}
      }
    ],
    "render_settings": {
      "width": 200,
      "height": 200,
      "padding": 30,
      "stop_radius": 5,
      "line_width": 14,
      "bus_label_font_size": 20,
      "bus_label_offset": [7, 15],
      "stop_label_font_size": 20,
      "stop_label_offset": [7, -3],
      "underlayer_color": [255,255,255,0.85],
      "underlayer_width": 3,
      "color_palette": ["green", [255,160,0],"red"]
    },
    "stat_requests": [
      { "id": 1, "type": "Map" },
      { "id": 2, "type": "Stop", "name": "Ривьерский мост" },
      { "id": 3, "type": "Bus", "name": "114" }
    ]
  }
  */