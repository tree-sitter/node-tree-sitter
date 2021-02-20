#ifndef NODE_TREE_SITTER_PARSER_H_
#define NODE_TREE_SITTER_PARSER_H_

#include <napi.h>
#include <tree_sitter/api.h>
#include "./binding.h"

namespace node_tree_sitter {

void InitParser(Napi::Object &exports, InstanceData *instance);

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_PARSER_H_
