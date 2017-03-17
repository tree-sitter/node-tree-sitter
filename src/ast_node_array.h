#ifndef NODE_TREE_SITTER_AST_NODE_ARRAY_H_
#define NODE_TREE_SITTER_AST_NODE_ARRAY_H_

#include <nan.h>
#include <node_object_wrap.h>
#include <tree_sitter/runtime.h>
#include <v8.h>

namespace node_tree_sitter {

class ASTNodeArray : public Nan::ObjectWrap {
 public:
  static void Init();
  static v8::Local<v8::Value> NewInstance(TSNode, TSDocument *, size_t, bool);

 private:
  explicit ASTNodeArray(TSNode, TSDocument *, size_t, bool);

  static NAN_METHOD(New);
  static NAN_GETTER(Length);
  static NAN_INDEX_GETTER(GetIndex);

  TSNode parent_node_;
  TSDocument *document_;
  size_t parse_count_;
  bool is_named_;

  static Nan::Persistent<v8::Function> constructor;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_AST_NODE_ARRAY_H_
