#ifndef NODE_TREE_SITTER_AST_NODE_H_
#define NODE_TREE_SITTER_AST_NODE_H_

#include <v8.h>
#include <node.h>
#include "tree_sitter/runtime.h"

namespace node_tree_sitter {

class ASTNode : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);
  static v8::Handle<v8::Value> NewInstance(TSNode *);

 private:
  explicit ASTNode(TSNode *);
  ~ASTNode();

  static v8::Handle<v8::Value> New(const v8::Arguments &args);
  static v8::Handle<v8::Value> ToString(const v8::Arguments &args);

  static v8::Handle<v8::Value> Name(v8::Local<v8::String>, const v8::AccessorInfo &);
  static v8::Handle<v8::Value> Position(v8::Local<v8::String>, const v8::AccessorInfo &);
  static v8::Handle<v8::Value> Size(v8::Local<v8::String>, const v8::AccessorInfo &);
  static v8::Handle<v8::Value> Parent(v8::Local<v8::String>, const v8::AccessorInfo &);
  static v8::Handle<v8::Value> Children(v8::Local<v8::String>, const v8::AccessorInfo &);

  static v8::Persistent<v8::Function> constructor;
  TSNode *node_;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_AST_NODE_H_
