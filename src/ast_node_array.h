#ifndef NODE_TREE_SITTER_AST_NODE_ARRAY_H_
#define NODE_TREE_SITTER_AST_NODE_ARRAY_H_

#include <nan.h>
#include <node_object_wrap.h>
#include <tree_sitter/runtime.h>
#include <v8.h>

namespace node_tree_sitter {

class ASTNodeArray : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);
  static v8::Handle<v8::Value> NewInstance(TSNode *);

 private:
  explicit ASTNodeArray(TSNode *);
  ~ASTNodeArray();

  static NAN_METHOD(New);
  static NAN_GETTER(Length);
  static NAN_INDEX_GETTER(GetIndex);

  TSNode *parent_node_;
  static v8::Persistent<v8::Function> constructor;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_AST_NODE_ARRAY_H_
