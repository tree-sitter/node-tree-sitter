#include "./ast_node.h"
#include "./ast_node_array.h"
#include "./document.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
  ASTNode::Init(exports);
  ASTNodeArray::Init(exports);
  Document::Init(exports);
}

NODE_MODULE(tree_sitter_runtime_binding, InitAll)
