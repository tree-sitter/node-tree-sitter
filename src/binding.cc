#include <node.h>
#include <v8.h>
#include "./addon_data.h"
#include "./language.h"
#include "./node.h"
#include "./parser.h"
#include "./query.h"
#include "./tree.h"
#include "./tree_cursor.h"
#include "./conversions.h"

namespace node_tree_sitter {

using namespace v8;

void InitAll(Local<Object> exports, Local<Value> m_, Local<Context> context) {
  Isolate* isolate = context->GetIsolate();

  AddonData* data = new AddonData(isolate);

  Local<External> data_ext = External::New(isolate, data);

  InitConversions(exports, data_ext);
  node_methods::Init(exports, data_ext);
  language_methods::Init(exports);
  Parser::Init(exports, data_ext);
  Query::Init(exports, data_ext);
  Tree::Init(exports, data_ext);
  TreeCursor::Init(exports, data_ext);
}

NODE_MODULE_CONTEXT_AWARE(tree_sitter_runtime_binding, InitAll)

}  // namespace node_tree_sitter
