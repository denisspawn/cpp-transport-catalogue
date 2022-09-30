#include "svg.h"

namespace svg {
    
using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------
Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
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
    
// ---------- Polyline ------------------
Polyline& Polyline::AddPoint(Point point) {
    points_.emplace_back(std::move(point));
    return *this;
}
 
void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    for (const auto& point : points_) {
        out << point.x << ","sv << point.y;
        if (&point != &points_.back())
          out << " "sv;  
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}
    
//---------- Text -----------------------
Text& Text::SetPosition(Point pos) {
    position_ = std::move(pos);
    return *this;
}
    
Text& Text::SetOffset(Point offset) {
    offset_ = std::move(offset);
    return *this;
}
    
Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}
    
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}
    
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}
    
Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}
    
void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(out);
    out << " x=\""sv << position_.x << "\""sv << " "sv << "y=\""sv << position_.y << "\""sv;
    out << " dx=\""sv << offset_.x << "\""sv << " "sv << "dy=\""sv << offset_.y << "\""sv;
    out << " font-size=\""sv << size_ << "\""sv;
    if (font_family_ != "")
        out << " font-family=\""sv << font_family_ << "\""sv;
    if (font_weight_ != "")
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    out << ">";
    for (auto c : data_) {
        if (c == '"') out << "&quot;"sv;
        else if (c == '\'')  out << "&apos;"sv;
        else if (c == '<') out << "&lt;"sv;
        else if (c == '>') out << "&gt;"sv;
        else if (c == '&') out << "&amp;"sv;
        else out << c;
    }
    out << "</text>"sv;
}
    
// ---------- Document ------------------
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}
    
void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (const auto& object : objects_) {
        RenderContext ctx(out, 2, 2);
        object->Render(ctx);
    }
    out << "</svg>"sv;
}

}  // namespace svg