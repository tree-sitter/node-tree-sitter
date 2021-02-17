#ifndef NODE_TREE_SITTER_NODE_H_
#define NODE_TREE_SITTER_NODE_H_

#include <napi.h>
#include <tree_sitter/api.h>
#include "./binding.h"
#include "./tree.h"
#include "./util.h"

using namespace Napi;

namespace node_tree_sitter {

void InitNode(Napi::Object &, InstanceData *);
Napi::Value MarshalNode(Napi::Env, const Tree *, TSNode);
Napi::Value MarshalNodes(
  Env env,
  const Tree *tree,
  const TSNode *nodes,
  uint32_t node_count
);
TSNode UnmarshalNode(Napi::Env env, const Tree *tree);

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_NODE_H_
