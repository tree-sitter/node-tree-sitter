#include <napi.h>
#include <cstdlib>
#include <tree_sitter/api.h>

#ifndef NODE_TREE_SITTER_ADDON_DATA_H_
#define NODE_TREE_SITTER_ADDON_DATA_H_

namespace node_tree_sitter {

class AddonData {
public:
  explicit AddonData(Napi::Env env) {}

  ~AddonData() {
    ts_query_cursor_delete(ts_query_cursor);
  }

  // conversions
  uint32_t *point_transfer_buffer = nullptr;

  // node
  uint32_t *transfer_buffer = nullptr;
  uint32_t transfer_buffer_length = 0;
  Napi::ObjectReference module_exports;
  TSTreeCursor scratch_cursor = {nullptr, nullptr, {0, 0}};

  // parser
  Napi::FunctionReference parser_constructor;
  Napi::FunctionReference string_slice;

  // query
  TSQueryCursor *ts_query_cursor = nullptr;
  Napi::FunctionReference query_constructor;

  // tree_cursor
  Napi::FunctionReference tree_cursor_constructor;

  // tree
  Napi::FunctionReference tree_constructor;
};

}

#endif  // NODE_TREE_SITTER_ADDON_DATA_H_
