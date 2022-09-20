#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {
    
// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    
    Node() = default;
    Node (const Dict& val);
    Node (const Array& val);
    Node(std::nullptr_t val);
    Node(bool val);
    Node(int val);
    Node(double val);
    Node(const std::string& val);
    
    bool IsInt() const;
    bool IsDouble() const; // возвращает true, если в Node хранится int либо double.
    bool IsPureDouble() const; // возвращает true, если в Node хранится double.
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const; // возвращает значение типа double, если внутри хранится double либо int. В последнем случае возвращается приведённое в double значение.
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    
    bool operator==(const Node rhs) const;
    bool operator!=(const Node rhs) const;
    
    const Value& GetValue() const;
private:
    Value value_;
};

class Document {
public:
    Document() = default;
    explicit Document(Node root);

    const Node& GetRoot() const;
    
    bool operator==(const Document rhs) const;
    bool operator!=(const Document rhs) const; 
private:
    Node root_;
};

Document Load(std::istream& input);

template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
    PrintValue(value, ctx);
}
void PrintValue(int i, std::ostream& out);
void PrintValue(double d, std::ostream& out);
void PrintValue(std::nullptr_t, std::ostream& out);
void PrintValue(const Array& array, std::ostream& out);
void PrintValue(const Dict& dict, std::ostream& out);
void PrintValue(const std::string& str, std::ostream& out);
void PrintValue(const bool bl, std::ostream& out);
void PrintNode(const Node& node, const PrintContext& ctx);

void Print(const Document& doc, std::ostream& output);
    
Node LoadNumber(std::istream& input);
Node LoadString(std::istream& input);

}  // namespace json
