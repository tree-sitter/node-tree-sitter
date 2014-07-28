#include "node_tree_sitter/parser.h"
#include <uv.h>
#include <string>

using namespace v8;

Persistent<Function> Parser::constructor;

void Parser::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Parser"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  constructor = Persistent<Function>::New(tpl->GetFunction());
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
  Local<Object> instance = constructor->NewInstance(0, NULL);
  parser->Wrap(instance);
  return scope.Close(instance);
}
