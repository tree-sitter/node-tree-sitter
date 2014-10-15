#ifndef NODE_TREE_SITTER_I_AST_NODE_H_
#define NODE_TREE_SITTER_I_AST_NODE_H_

#include <nan.h>
#include <node_object_wrap.h>
#include <tree_sitter/runtime.h>
#include <v8.h>

namespace node_tree_sitter {

class IASTNode : public node::ObjectWrap {
 public:
  static void SetUp(v8::Local<v8::FunctionTemplate>);
  virtual TSNode * node() = 0;

 private:
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
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_AST_NODE_H_
