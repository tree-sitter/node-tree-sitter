#ifndef TREE_SITTER_DOCUMENT_H
#define TREE_SITTER_DOCUMENT_H

#include <v8.h>
#include <node.h>
#include "tree_sitter/runtime.h"

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
  static v8::Handle<v8::Value> RootNode(const v8::Arguments& args);

  static v8::Persistent<v8::Function> constructor;

  TSDocument *value_;
};


#endif  // TREE_SITTER_DOCUMENT_H
