#include <napi.h>
#include "./language.h"
#include "./node.h"
#include "./parser.h"
#include "./query.h"
#include "./tree.h"
#include "./tree_cursor.h"
#include "./conversions.h"

namespace node_tree_sitter {

using namespace Napi;

Object Init(Env env, Object exports) {
  InitConversions(exports);
  InitNode(exports);
  InitLanguage(exports);
  InitParser(exports);
  InitTreeCursor(exports);
  Query::Init(exports);
  Tree::Init(exports);
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)

}  // namespace node_tree_sitter
