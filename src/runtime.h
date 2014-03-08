#ifndef TREE_SITTER_BINDING_RUNTIME_H
#define TREE_SITTER_BINDING_RUNTIME_H

#include <v8.h>
#include <node.h>
#include "tree_sitter/runtime.h"

class ParseConfig : public node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> exports);
  static v8::Handle<v8::Value> NewInstance(const v8::Arguments& args);
  ts_parse_config value() const;

private:
  explicit ParseConfig(ts_parse_config);

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Persistent<v8::Function> constructor;

  ts_parse_config value_;
};

ts_input ParseInput(v8::Handle<v8::Object>);
v8::Handle<v8::Value> LoadParserLib(const v8::Arguments &args);

class Document : public node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> exports);

private:
  explicit Document();
  ~Document();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> SetInput(const v8::Arguments& args);
  static v8::Handle<v8::Value> SetParser(const v8::Arguments& args);
  static v8::Handle<v8::Value> ToString(const v8::Arguments& args);
  static v8::Persistent<v8::Function> constructor;

  ts_document *value_;
};

#endif
