#include "node_tree_sitter/parser.h"
#include "./document.h"
#include "./ast_node.h"
#include "./ast_node_array.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
  Document::Init(exports);
  Parser::Init(exports);
  ASTNode::Init(exports);
  ASTNodeArray::Init(exports);
}

NODE_MODULE(tree_sitter_runtime_binding, InitAll)
