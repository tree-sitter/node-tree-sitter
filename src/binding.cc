#include <node.h>
#include <v8.h>
#include "./node.h"
#include "./parser.h"
#include "./tree.h"
#include "./tree_cursor.h"
#include "./conversions.h"

namespace node_tree_sitter {

using namespace v8;

void InitAll(Local<Object> exports) {
  InitConversions(exports);
  node_methods::Init(exports);
  Parser::Init(exports);
  Tree::Init(exports);
  TreeCursor::Init(exports);
}

NODE_MODULE(tree_sitter_runtime_binding, InitAll)

}  // namespace node_tree_sitter
