#include "svg.h"

namespace svg {
using namespace std::literals;

void ColorPrint::operator()(std::monostate) {
    out << "none"sv;
}
void ColorPrint::operator()(std::string s) {
    out << s;
}
void ColorPrint::operator()(Rgb rgb) {
    out << "rgb("sv << static_cast<int>(rgb.red) << ","sv << static_cast<int>(rgb.green) << ","sv << static_cast<int>(rgb.blue) << ")";
}
void ColorPrint::operator()(Rgba rgba) {
    out << "rgba("sv << static_cast<int>(rgba.red) << ","sv << static_cast<int>(rgba.green) << ","sv << static_cast<int>(rgba.blue) << ","sv << rgba.opacity << ")";
}

std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit(ColorPrint{ out }, color);
    return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& stroke_linecap) {
    using namespace std::literals;
    switch (stroke_linecap) {
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

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& stroke_linejoin) {
    using namespace std::literals;
    switch (stroke_linejoin) {
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
// ---------- Object ------------------

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

//---------- Polyline ------------------
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::PointsOut(std::ostream& out) const {
    bool start = true;
    for (const auto p : points_) {
        if (start) {
            start = false;
            out << p.x << ","sv << p.y;
        }
        else {
            out << " "sv << p.x << ","sv << p.y;
        }
    }
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    PointsOut(out);
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}


//---------- Text------------------
Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

std::string Text::DataTransformated() const {
    std::string res;
    for (const char c : data_) {
        if (c == '\"') {
            res += "&quot;"s;
            continue;
        }
        if (c == '\'') {
            res += "&apos;"s;
            continue;
        }
        if (c == '<') {
            res += "&lt;"s;
            continue;
        }
        if (c == '>') {
            res += "&gt"s;
            continue;
        }
        if (c == '&') {
            res += "&amp;"s;
            continue;
        }
        res += c;
    }
    return res;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(out);
    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y;
    out << "\" font-size=\""sv << size_ << "\""sv;
    if (!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    out << ">"sv;
    if (!data_.empty()) {
        out << DataTransformated();
    }
    out << "</text>"sv;
}

//---------- Document------------------
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
    RenderContext ctx(out, 2, 2);
    for (const auto& obj : objects_) {
        obj->Render(ctx);
    }
    out << "</svg>\n"sv;
}

}  // namespace svg