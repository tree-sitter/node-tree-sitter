#ifndef NODE_TREE_SITTER_AST_NODE_ARRAY_H_
#define NODE_TREE_SITTER_AST_NODE_ARRAY_H_

#include <v8.h>
#include <node.h>
#include "tree_sitter/runtime.h"

namespace node_tree_sitter {

class ASTNodeArray : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);
  static v8::Handle<v8::Value> NewInstance(TSNode *);

 private:
  explicit ASTNodeArray(TSNode *);
  ~ASTNodeArray();

  static v8::Handle<v8::Value> New(const v8::Arguments &args);
  static v8::Handle<v8::Value> Length(v8::Local<v8::String>, const v8::AccessorInfo &);
  static v8::Handle<v8::Value> GetIndex(size_t, const v8::AccessorInfo &);

  TSNode *parent_node_;
  static v8::Persistent<v8::Function> constructor;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_AST_NODE_ARRAY_H_
