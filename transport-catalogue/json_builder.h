#pragma once

#include "json.h"

#include <memory>

namespace json {
using namespace std::literals;
    
class Builder;
class KeyContext;
class ValueContext;
class StartDictContext;
class StartArrayContext;
    
class Context {
public:
    Context(Builder& builder);
    
    KeyContext Key(const std::string& key);
    Context Value(Node::Value value);
    StartDictContext StartDict();
    StartArrayContext StartArray();
    Context EndDict();
    Context EndArray();
    Node Build();
    
protected:
    Builder& builder_;
};
    
class KeyContext : public Context {
public:
    KeyContext(Builder& builder);
    ValueContext Value(Node::Value value);
    KeyContext Key(const std::string& key) = delete;
    Context EndDict() = delete;
    Context EndArray() = delete;
    Node Build() = delete;
};
    
class ValueContext : public Context {
public:
    ValueContext(Builder& builder);
    Context Value(Node::Value value) = delete;
    StartDictContext StartDict() = delete;
    StartArrayContext StartArray() = delete;
    Context EndArray() = delete;
    Node Build() = delete;
};
    
class StartDictContext : public Context {
public:
    StartDictContext(Builder& builder);
    Context Value(Node::Value value) = delete;
    StartDictContext StartDict() = delete;
    StartArrayContext StartArray() = delete;
    Context EndArray() = delete;
    Node Build() = delete;
};
    
class ValueArrayContext : public Context {
public:
    ValueArrayContext(Builder& builder);
    ValueArrayContext Value(Node::Value value);
    KeyContext Key(const std::string& key) = delete;
    Context EndDict() = delete;
    Node Build() = delete;
};
    
class StartArrayContext : public Context {
public:
    StartArrayContext(Builder& builder);
    ValueArrayContext Value(Node::Value value);
    KeyContext Key(const std::string& key) = delete;
    Context EndDict() = delete;
    Node Build() = delete;
};
    
class Builder {
public:
    KeyContext Key(const std::string& key);
    Context Value(Node::Value value);
    StartDictContext StartDict();
    StartArrayContext StartArray();
    Context EndDict();
    Context EndArray();
    Node Build();
        
private:
    Node root_;
    std::vector<std::unique_ptr<Node>>nodes_stack_;
    
    Node CreateNode(Node::Value value);
    void AddNode(Node node);
    
};
    
} // namespace json
