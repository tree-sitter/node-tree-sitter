#ifndef NODE_TREE_SITTER_AST_NODE_H_
#define NODE_TREE_SITTER_AST_NODE_H_

#include <nan.h>
#include <tree_sitter/runtime.h>
#include <v8.h>
#include "./i_ast_node.h"

namespace node_tree_sitter {

class ASTNode : public IASTNode {
 public:
  static void Init(v8::Handle<v8::Object> exports);
  static v8::Handle<v8::Value> NewInstance(TSNode *);
  TSNode * node();

 private:
  explicit ASTNode(TSNode *);
  ~ASTNode();

  static NAN_METHOD(New);

  TSNode *node_;
  static v8::Persistent<v8::Function> constructor;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_AST_NODE_H_
