#ifndef NODE_TREE_SITTER_AST_NODE_H_
#define NODE_TREE_SITTER_AST_NODE_H_

#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>
#include "nan.h"
#include "tree_sitter/runtime.h"

namespace node_tree_sitter {

class ASTNode : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);
  static v8::Handle<v8::Value> NewInstance(TSNode *);

 private:
  explicit ASTNode(TSNode *);
  ~ASTNode();

  static NAN_METHOD(New);
  static NAN_METHOD(ToString);
  static NAN_METHOD(Parent);
  static NAN_METHOD(Next);
  static NAN_METHOD(Prev);
  static NAN_METHOD(NodeAt);

  static NAN_GETTER(Name);
  static NAN_GETTER(Position);
  static NAN_GETTER(Size);
  static NAN_GETTER(Parent);
  static NAN_GETTER(Children);

  static v8::Persistent<v8::Function> constructor;
  TSNode *node_;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_AST_NODE_H_
