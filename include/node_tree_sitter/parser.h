#ifndef NODE_TREE_SITTER_PARSER_H_
#define NODE_TREE_SITTER_PARSER_H_

#include <v8.h>
#include <node.h>
#include "tree_sitter/runtime.h"

class Parser : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);
  static v8::Handle<v8::Value> NewInstance(TSParser *);
  static bool HasInstance(v8::Handle<v8::Value>);
  TSParser * value() const;

 private:
  explicit Parser(TSParser *);
  static v8::Handle<v8::Value> New(const v8::Arguments& args);

  static v8::Persistent<v8::Function> constructor_;
  static v8::Persistent<v8::FunctionTemplate> template_;
  TSParser *value_;
};

#endif  // NODE_TREE_SITTER_PARSER_H_
