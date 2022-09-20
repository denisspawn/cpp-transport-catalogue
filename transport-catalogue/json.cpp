#include "json.h"

using namespace std;
namespace json {
namespace {
    
Node LoadNode(istream& input);
    
std::string LoadLiteral(std::istream& input) {
    std::string result;
    while (std::isalpha(input.peek())) {
        result.push_back(input.get());
    }
    return result;
}

Node LoadArray(istream& input) {
    Array result;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    
    if (it == end)
        throw ParsingError("LoadArray error. Hasn't found closing symbol: ]"s);
    
    for (char c; input >> c && c != ']'; ) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    return Node(move(result));
}

using Number = std::variant<int, double>;
Node LoadNumber(istream& input) {
    using namespace std::literals;

    std::string parsed_num;
    
    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(std::stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(s);
}
    
Node LoadDict(istream& input) {
    Dict result;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    
    if (it == end)
        throw ParsingError("LoadDict error. Doesn't find close simbol: }"s);

    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }

    return Node(move(result));
}
    
Node LoadNull(std::istream& input) {
    if (LoadLiteral(input) == "null"s)
        return Node{ nullptr };
    
    throw ParsingError("Failed to parse Null value"s);
}
    
Node LoadBool(std::istream& input) {
    const std::string line = LoadLiteral(input);
    if (line == "true"sv) {
        return Node{ true };
    }
    if (line == "false"sv) {
        return Node{ false };
    }
    throw ParsingError("Failed to parse Bool as bool"s);
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    } else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    } else {
        input.putback(c);
        return LoadNumber(input);
    }
}
    
}  // namespace

//Node::Node() : value_(nullptr) {}
Node::Node(int val) : value_(move(val)) {}
Node::Node(double val) : value_(move(val)) {}
Node::Node(const string& val) : value_(move(val)) {}
Node::Node(nullptr_t val): value_(move(val)) {}
Node::Node(bool val): value_(move(val)) {}
Node::Node(const Dict& val): value_(move(val)) {}
Node::Node(const Array& val): value_(move(val)) {}

bool Node::IsInt() const {
    return holds_alternative<int>(value_);
}
    
//Возвращает true, если в Node хранится int либо double.
bool Node::IsDouble() const {
    return holds_alternative<double>(value_) || holds_alternative<int>(value_);
}
    
//Возвращает true, если в Node хранится double.
bool Node::IsPureDouble() const { 
    return holds_alternative<double>(value_);
}
    
bool Node::IsBool() const {
    return holds_alternative<bool>(value_);
}
    
bool Node::IsString() const {
    return holds_alternative<string>(value_);
}
    
bool Node::IsNull() const {
    return holds_alternative<nullptr_t>(value_);
}
    
bool Node::IsArray() const {
    return holds_alternative<Array>(value_);
}
    
bool Node::IsMap() const {
    return holds_alternative<Dict>(value_);
}
    
int Node::AsInt() const {
    if (!IsInt()) {
        throw std::logic_error("The Node doesn't contain an int type value");
    }
    return get<int>(value_);
}
    
bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error("The Node doesn't contain an bool type value");
    }
    return get<bool>(value_);
}
// возвращает значение типа double, если внутри хранится double либо int. В последнем случае возвращается приведённое в double значение.
double Node::AsDouble() const {
    if (!IsDouble()) {
        throw std::logic_error("The Node doesn't contain an double type value");
    }
    if (IsPureDouble()) {
        return get<double>(value_);
    } else {
        return static_cast<double>(get<int>(value_));
    }
    
}
    
const string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("The Node doesn't contain an string type value");
    }
    return get<string>(value_);
}
    
const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw std::logic_error("The Node doesn't contain an Array type value");
    }
    return get<Array>(value_);
}
    
const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw std::logic_error("The Node doesn't contain an Dict type value");
    }
    return get<Dict>(value_);    
}
    
bool Node::operator==(const Node rhs) const {
    return GetValue() == rhs.GetValue();
}
    
bool Node::operator!=(const Node rhs) const {
    return GetValue() != rhs.GetValue();
}
    
const Node::Value& Node::GetValue() const {
    return value_; 
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}
    
bool Document::operator==(const Document rhs) const {
    return GetRoot() == rhs.GetRoot();
}

bool Document::operator!=(const Document rhs) const {
    return GetRoot() != rhs.GetRoot();
} 

Document Load(istream& input) {
    return Document{LoadNode(input)};
}
    
void PrintValue(std::nullptr_t, const PrintContext& ctx) {
    ctx.out << "null"sv;
}
    
void PrintValue(const Array& array, const PrintContext& ctx) {
    ctx.out << '[';
    int i = 0;
    if (i == 1) {
        ctx.out << ", "sv;
    }
    for (const auto& a : array) {
        if (i == 1) {
            ctx.out << ", "sv;
        }
        PrintNode(a, ctx);
        i = 1;
    }
    ctx.out << ']';
}
    
void PrintValue(const Dict& dict, const PrintContext& ctx) {
    ctx.out << '{';
    int i = 0;
    if (i == 1) {
        ctx.out << ", "sv;
    }
    for (const auto& [a, b] : dict) {
        if (i == 1) {
            ctx.out << ", "sv;
        }
        ctx.out << '\n';
        ctx.Indented().PrintIndent();
        ctx.out << "\""sv << a << "\": "sv;
        PrintNode(b, ctx.Indented());
        i = 1;
    }
    ctx.out << '\n';
    ctx.PrintIndent();
    ctx.out << '}';
}
    
void PrintValue(const std::string& str, const PrintContext& ctx) {
    ctx.out << "\""s;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i]=='"') {
            ctx.out << "\\\""s;
        } else if (str[i]=='\r') {
            ctx.out << "\\r"s;
        } else if (str[i]=='\n') {
            ctx.out << "\\n"s;
        } else if (str[i]=='\\') {
            ctx.out << "\\\\"s;
        } else { ctx.out << str[i];}
    }
    ctx.out << "\""s;
}
    
void PrintValue(const bool bl, const PrintContext& ctx) {
    if (bl) {
        ctx.out << "true"s;
    } else {
        ctx.out << "false"s;
    }
}
    
void PrintValue(int i, const PrintContext& ctx) {
    ctx.out << i;
}
    
void PrintValue(double d, const PrintContext& ctx) {
    ctx.out << d;
}

void PrintNode(const Node& node, const PrintContext& ctx) {  
    std::visit(
        [&ctx](const auto& value){ PrintValue(value, ctx); },
        node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    PrintContext ctx{output};
    PrintNode(doc.GetRoot(), ctx);
}
    
}  // namespace json
