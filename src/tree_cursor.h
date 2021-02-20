#ifndef NODE_TREE_SITTER_TREE_CURSOR_H_
#define NODE_TREE_SITTER_TREE_CURSOR_H_

#include <napi.h>
#include <tree_sitter/api.h>
#include "./binding.h"

namespace node_tree_sitter {

void InitTreeCursor(Napi::Object &, InstanceData *);
Napi::Value NewTreeCursor(Napi::Env, TSTreeCursor);

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_TREE_CURSOR_H_
