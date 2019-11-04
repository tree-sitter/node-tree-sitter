#ifndef NODE_TREE_SITTER_PARSER_H_
#define NODE_TREE_SITTER_PARSER_H_

#include <napi.h>
#include <tree_sitter/api.h>

namespace node_tree_sitter {

void InitParser(Napi::Object &exports);

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_PARSER_H_
