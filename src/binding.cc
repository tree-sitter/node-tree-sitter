#include <napi.h>
#include "./language.h"
#include "./node.h"
#include "./parser.h"
#include "./query.h"
#include "./tree.h"
#include "./tree_cursor.h"
#include "./conversions.h"


using namespace Napi;

Napi::Object Init(Env env, Napi::Object exports) {
  node_tree_sitter::InitConversions(exports);
  node_tree_sitter::InitNode(exports);
  node_tree_sitter::InitLanguage(exports);
  node_tree_sitter::InitParser(exports);
  node_tree_sitter::Query::Init(exports);
  node_tree_sitter::InitTreeCursor(exports);
  node_tree_sitter::Tree::Init(exports);
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
