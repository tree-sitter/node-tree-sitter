#ifndef NODE_TREE_SITTER_BINDING_H_
#define NODE_TREE_SITTER_BINDING_H_

#include <napi.h>

namespace node_tree_sitter {

struct InstanceData {
  uint32_t *point_transfer_buffer;
  uint32_t point_transfer_buffer_length;
  Napi::FunctionReference *node_constructor;
  Napi::FunctionReference *query_constructor;
  Napi::FunctionReference *tree_constructor;
  Napi::FunctionReference *parser_constructor;
  Napi::FunctionReference *tree_cursor_constructor;
  uint32_t *node_transfer_buffer;
  uint32_t node_transfer_buffer_length;
  Napi::Reference<Napi::Uint32Array> *node_transfer_array_ref;
};

InstanceData *GetInternalData(Napi::Env);

} // namespace node_tree_sitter

#endif // NODE_TREE_SITTER_BINDING_H_