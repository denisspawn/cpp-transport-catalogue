#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <iomanip>

namespace svg {
using namespace std::literals;

struct Rgb {
    Rgb() = default;
    Rgb(uint8_t r, uint8_t g, uint8_t b)
        : red(r)
        , green(g)
        , blue(b) {}
    
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba {
    Rgba() = default;
    Rgba(uint8_t r, uint8_t g, uint8_t b, double op)
        : red(r)
        , green(g)
        , blue(b) 
        , opacity(op) {}
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

//using Color = std::string;
using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
// Объявив в заголовочном файле константу со спецификатором inline,
// мы сделаем так, что она будет одной на все единицы трансляции,
// которые подключают этот заголовок.
// В противном случае каждая единица трансляции будет использовать свою копию этой константы
inline const Color NoneColor{"none"s};
const Color color1;                          // none
const Color color2 = Rgb{215, 30, 25};       // rgb(215,30,25)
const Color color3 = NoneColor;              // none
const Color color4 = Rgba{15, 15, 25, 0.7};  // rgba(15,15,25,0.7)
const Color color5 = "red"s;                 // red
const Color color6 = Rgb{};                  // rgb(0,0,0)
const Color color7 = Rgba{};                 // rgba(0,0,0,1.0);
    
struct OstreamColorPrinter {
    std::ostream& out;
    
    void operator()(std::monostate) const {
        out << "none"s;
    }
    void operator()(std::string color) const {
        out << color;
    }
    void operator()(Rgb rgb) const {
        out << "rgb("s 
            // + нужен, чтобы преобразовать uint8_t в int иначе в потоке отображаются "кракозябры"
            << static_cast<int>(rgb.red)   << ","s 
            << static_cast<int>(rgb.green) << ","s 
            << static_cast<int>(rgb.blue)  << ")"s;
    }
    void operator()(Rgba rgba) const {
        out << "rgba("s
            // + нужен, чтобы преобразовать uint8_t в int иначе в потоке отображаются "кракозябры"
            << static_cast<int>(rgba.red)     << ","s
            << static_cast<int>(rgba.green)   << ","s
            << static_cast<int>(rgba.blue)    << ","s
            << rgba.opacity  << ")"s;
    }
};
    
inline std::ostream& operator<<(std::ostream& out, Color color) {
    std::visit(OstreamColorPrinter{out}, color);
    return out;
}
    
enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};
    
inline std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
    using namespace std::literals;
    switch (line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
    }
    return out;
}

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};
    
inline std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
    using namespace std::literals;
    switch (line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
    }
    return out;
}

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};
  
template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        stroke_linecap_ = line_cap;
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        stroke_linejoin_ = line_join;
        return AsOwner();
    }

protected:
    ~PathProps() = default;
    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;
        if (fill_color_) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }
        if (stroke_width_) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }
        if (stroke_linecap_) {
            out << " stroke-linecap=\""sv << *stroke_linecap_ << "\""sv;
        }
        if (stroke_linejoin_) {
            out << " stroke-linejoin=\""sv << *stroke_linejoin_ << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }
    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_linecap_;
    std::optional<StrokeLineJoin> stroke_linejoin_;
};
    
/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
// Наследованием от PathProps<Circle> мы «‎сообщаем» родителю,
// что владельцем свойств является класс Circle
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle() = default;
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    Polyline() = default;
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);
    
private:
    void RenderObject(const RenderContext& context) const override;
    std::vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    Text() = default;
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    // Прочие данные и методы, необходимые для реализации элемента <text>
private:
    void RenderObject(const RenderContext& context) const override;
    Point position_;
    Point offset_;
    uint32_t size_ = 1;
    std::string font_weight_;
    std::string font_family_;
    std::string data_;
};
    
class ObjectContainer {
public:
    template <typename T>
    void Add(T obj);
    
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    virtual ~ObjectContainer() = default;
};
    
class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};

class Document : public ObjectContainer {
public:
    /*
     Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
     Пример использования:
     Document doc;
     doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
    */

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

    // Прочие методы и данные, необходимые для реализации класса Document
private:
    std::vector<std::unique_ptr<Object>> objects_;
};

template <typename T>
void ObjectContainer::Add(T obj) {
    //objects_.emplace_back(std::make_unique<T>(std::move(obj)));
    AddPtr(std::make_unique<T>(std::move(obj)));
}

}  // namespace svg
