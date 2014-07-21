#ifndef TREE_SITTER_AST_NODE_ARRAY_H
#define TREE_SITTER_AST_NODE_ARRAY_H

#include <v8.h>
#include <node.h>
#include "tree_sitter/runtime.h"

class ASTNodeArray : public node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> exports);
  static v8::Handle<v8::Value> NewInstance(TSNode *);
  TSNode * value() const;

  explicit ASTNodeArray(TSNode *);
  ~ASTNodeArray();

private:
  static v8::Handle<v8::Value> Length(v8::Local<v8::String>, const v8::AccessorInfo &);
  static v8::Handle<v8::Value> GetIndex(size_t, const v8::AccessorInfo &);

  TSNode *value_;
  static v8::Persistent<v8::Function> constructor;
};

#endif  // TREE_SITTER_AST_NODE_ARRAY_H
