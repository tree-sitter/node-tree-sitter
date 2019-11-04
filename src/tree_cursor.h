#ifndef NODE_TREE_SITTER_TREE_CURSOR_H_
#define NODE_TREE_SITTER_TREE_CURSOR_H_

#include <napi.h>
#include <tree_sitter/api.h>

namespace node_tree_sitter {

void InitTreeCursor(Napi::Object &);
Napi::Value NewTreeCursor(TSTreeCursor);

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_TREE_CURSOR_H_
