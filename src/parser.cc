#include "node_tree_sitter/parser.h"
#include <uv.h>
#include <string>

using namespace v8;

Persistent<Function> Parser::constructor_;
Persistent<FunctionTemplate> Parser::template_;

void Parser::Init(Handle<Object> exports) {
  template_ = Persistent<FunctionTemplate>::New(FunctionTemplate::New(New));
  template_->SetClassName(String::NewSymbol("Parser"));
  template_->InstanceTemplate()->SetInternalFieldCount(1);

  constructor_ = Persistent<Function>::New(template_->GetFunction());
  exports->Set(String::NewSymbol("Parser"), constructor_);
}

Parser::Parser(TSParser *value) : value_(value) {}

TSParser * Parser::value() const {
  return value_;
}

Handle<Value> Parser::New(const Arguments &args) {
  HandleScope scope;
  return scope.Close(Undefined());
}

Handle<Value> Parser::NewInstance(TSParser *value) {
  HandleScope scope;
  Parser *parser = new Parser(value);
  Local<Object> instance = constructor_->NewInstance(0, NULL);
  parser->Wrap(instance);
  return scope.Close(instance);
}

bool Parser::HasInstance(Handle<Value> object) {
  return template_->HasInstance(object);
}
