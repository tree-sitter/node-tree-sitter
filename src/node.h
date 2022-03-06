#ifndef NODE_TREE_SITTER_NODE_H_
#define NODE_TREE_SITTER_NODE_H_

#include <napi.h>
#include <tree_sitter/api.h>
#include "./tree.h"
#include "./util.h"

namespace node_tree_sitter {

void InitNode(Napi::Object &exports);
Napi::Value MarshalNode(Napi::Env, const Tree *, TSNode);
Napi::Value GetMarshalNode(Napi::Env env, const Tree*, TSNode);
Napi::Value GetMarshalNodes(Napi::Env env, const Tree*, const TSNode*, uint32_t);
TSNode UnmarshalNode(Napi::Env env, const Tree *tree);

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_NODE_H_
