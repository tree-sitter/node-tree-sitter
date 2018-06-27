#ifndef NODE_TREE_SITTER_NODE_H_
#define NODE_TREE_SITTER_NODE_H_

#include <nan.h>
#include <v8.h>
#include <node_object_wrap.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {
namespace node_methods {

void Init(v8::Local<v8::Object>);
void MarshalNode(TSNode);

}  // namespace node_methods
}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_NODE_H_
