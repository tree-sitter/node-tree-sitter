#include <napi.h>
#include "./binding.h"
#include "./language.h"
#include "./node.h"
#include "./parser.h"
#include "./query.h"
#include "./tree.h"
#include "./tree_cursor.h"
#include "./conversions.h"


using namespace Napi;

namespace node_tree_sitter {

InstanceData *GetInternalData(Env env) {
  return env.GetInstanceData<InstanceData>();
}

void InstanceDataFinalizer(Env env, InstanceData *instance) {
  free(instance->point_transfer_buffer);
}

}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  node_tree_sitter::InstanceData *instance = static_cast<node_tree_sitter::InstanceData *>(malloc(sizeof(node_tree_sitter::InstanceData)));

  node_tree_sitter::InitConversions(exports, instance);
  node_tree_sitter::InitNode(exports, instance);
  node_tree_sitter::InitLanguage(exports);
  node_tree_sitter::InitParser(exports, instance);
  node_tree_sitter::Query::Init(exports, instance);
  node_tree_sitter::InitTreeCursor(exports, instance);
  node_tree_sitter::Tree::Init(exports, instance);
  env.SetInstanceData<node_tree_sitter::InstanceData, node_tree_sitter::InstanceDataFinalizer>(instance);
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
