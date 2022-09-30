#include "json_builder.h"

namespace json {
    
Context::Context(Builder& builder)
    : builder_(builder) {}

KeyContext Context::Key(const std::string& key) {
    return builder_.Key(key);
}
    
Context Context::Value(Node::Value value) {
    return builder_.Value(value);
}
    
StartDictContext Context::StartDict() {
    return builder_.StartDict();
}
    
StartArrayContext Context::StartArray() {
    return builder_.StartArray();
}
    
Context Context::EndDict() {
    return builder_.EndDict();
}
    
Context Context::EndArray() {
    return builder_.EndArray();
}
    
Node Context::Build() {
    return builder_.Build();
}
    
KeyContext::KeyContext(Builder& builder)
    : Context(builder) {}
    
ValueContext KeyContext::Value(Node::Value value) {
    builder_.Value(value);
    return ValueContext(builder_);
}
    
ValueContext::ValueContext(Builder& builder)
    : Context(builder) {}
    
StartDictContext::StartDictContext(Builder& builder)
    : Context(builder) {}
    
ValueArrayContext::ValueArrayContext(Builder& builder)
    :Context(builder) {
}

ValueArrayContext ValueArrayContext::Value(Node::Value value) {
    builder_.Value(value);
    return ValueArrayContext(builder_);
}
    
StartArrayContext::StartArrayContext(Builder& builder)
    : Context(builder) {}
    
ValueArrayContext StartArrayContext::Value(Node::Value value) {
    builder_.Value(value);
    return ValueArrayContext(builder_);
}
    
Node Builder::CreateNode(Node::Value value) {
    Node node;
    if (std::holds_alternative<Array>(value)) {
        Array arr = std::get<Array>(value);
        node = Node(std::move(arr));
    }
    else if (std::holds_alternative<Dict>(value)) {
        Dict dict = std::get<Dict>(value);
        node = Node(std::move(dict));
    }
    else if (std::holds_alternative<bool>(value)) {
        bool b = std::get<bool>(value);
        node = Node(b);
    }
    else if (std::holds_alternative<int>(value)) {
        int i = std::get<int>(value);
        node = Node(i);
    }
    else if (std::holds_alternative<std::string>(value)) {
        std::string s = std::get<std::string>(value);
        node = Node(std::move(s));
    }
    else if (std::holds_alternative<double>(value)) {
        double d = std::get<double>(value);
        node = Node(d);
    }
    else {
        node = Node();
    }
    return node;
}

void Builder::AddNode(Node node) {
    if (nodes_stack_.empty()) {
        if (!root_.IsNull()) {
            throw std::logic_error("Root already exists");
        }
        root_ = node;
        return;
    }
    if (!nodes_stack_.back()->IsString() && !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Attempt to create value failed");
    }
    if (nodes_stack_.back()->IsString()) {
        std::string s = nodes_stack_.back()->AsString();
        nodes_stack_.pop_back();
        std::get<Dict>(nodes_stack_.back()->GetValue()).emplace(std::move(s), node);
        return;
    }
    if (nodes_stack_.back()->IsArray()) {
        std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(node);
        return;
    }
}
    
KeyContext Builder::Key(const std::string& key) {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Attempt to create a Key outside a Dict");
    }
  
    std::unique_ptr<Node> key_ptr = std::make_unique<Node>(key);
    if (nodes_stack_.back()->IsDict()) {
        nodes_stack_.emplace_back(std::move(key_ptr));

    }
    return KeyContext(*this);
}

Context Builder::Value(Node::Value value) {
    Node node = CreateNode(value);
    AddNode(node);
    return Context(*this);
}
    
StartDictContext Builder::StartDict() {
    std::unique_ptr<Node> dict_ptr = std::make_unique<Node>(Dict());
    nodes_stack_.emplace_back(std::move(dict_ptr));
    return StartDictContext(*this);
}
    
StartArrayContext Builder::StartArray() {
    std::unique_ptr<Node> arr_ptr = std::make_unique<Node>(Array());
    nodes_stack_.emplace_back(std::move(arr_ptr));
    return StartArrayContext(*this);
}
    
Context Builder::EndDict() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Attempt to close as Dict without opening");
    }
        
    Node node = *nodes_stack_.back();
    if (node.IsDict() == false) {
        throw std::logic_error("Attempt to close as Dict node object that isn't dict");
    }
    nodes_stack_.pop_back();
    AddNode(node);
    return Context(*this);
}

Context Builder::EndArray() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Attempt to close as Array without opening");
    }
    
    Node node = *nodes_stack_.back();
    if (node.IsArray() == false) {
        throw std::logic_error("Attempt to close as Array node object that isn't array");
    }
    nodes_stack_.pop_back();
    AddNode(node);
    return Context(*this);
}

Node Builder::Build() {
    if (root_.IsNull()) {
        throw std::logic_error("Attempt to build an empty json");
    }
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Attempt to build an incomplete json");
    }
    return root_;
}
    
} // namespace json