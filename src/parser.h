#ifndef TREE_SITTER_PARSER_H
#define TREE_SITTER_PARSER_H

#include <v8.h>
#include <node.h>
#include "tree_sitter/runtime.h"

class Parser : public node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> exports);

  static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);
  TSParser * value() const;

private:
  explicit Parser(TSParser *);

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Persistent<v8::Function> constructor;

  TSParser *value_;
};

#endif  // TREE_SITTER_PARSER_H
