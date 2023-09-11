#ifndef NODE_TREE_SITTER_NODE_H_
#define NODE_TREE_SITTER_NODE_H_

#include "tree_sitter/api.h"
#include "./tree.h"

#include <nan.h>
#include <node_object_wrap.h>
#include <v8.h>

namespace node_tree_sitter::node_methods {

void Init(v8::Local<v8::Object>);
void MarshalNode(const Nan::FunctionCallbackInfo<v8::Value> &info, const Tree *, TSNode);
v8::Local<v8::Value> GetMarshalNode(const Nan::FunctionCallbackInfo<v8::Value> &info, const Tree *tree, TSNode node);
v8::Local<v8::Value> GetMarshalNodes(const Nan::FunctionCallbackInfo<v8::Value> &info, const Tree *tree, const TSNode *nodes, uint32_t node_count);
TSNode UnmarshalNode(const Tree *tree);

static inline const void *UnmarshalNodeId(const uint32_t *buffer) {
  const void *result = nullptr;
  memcpy(&result, buffer, sizeof(result));
  return result;
}

static inline void MarshalNodeId(const void *node_id, uint32_t *buffer) {
  memset(buffer, 0, sizeof(uint64_t));
  memcpy(buffer, &node_id, sizeof(node_id));
}

} // namespace node_tree_sitter::node_methods

#endif  // NODE_TREE_SITTER_NODE_H_
