#include <node.h>
#include <v8.h>
#include "./ast_node.h"
#include "./ast_node_array.h"
#include "./document.h"
#include "./input_reader.h"
#include "./conversions.h"

namespace node_tree_sitter {

using namespace v8;

void InitAll(Local<Object> exports) {
  InitConversions();
  ASTNode::Init(exports);
  ASTNodeArray::Init(exports);
  InputReader::Init();
  Document::Init(exports);
  InitConversions();
}

NODE_MODULE(tree_sitter_runtime_binding, InitAll)

}  // namespace node_tree_sitter
