#include <nan.h>
#include <cstdlib>
#include <tree_sitter/api.h>

#ifndef NODE_TREE_SITTER_ADDON_DATA_H_
#define NODE_TREE_SITTER_ADDON_DATA_H_

namespace node_tree_sitter {

class AddonData {
public:
  explicit AddonData(v8::Isolate* isolate) {
    // Ensure this per-addon-instance data is deleted at environment cleanup.
    node::AddEnvironmentCleanupHook(isolate, DeleteInstance, this);
  }

  ~AddonData() {
    ts_query_cursor_delete(ts_query_cursor);
  }

  // conversions
  Nan::Persistent<v8::String> row_key;
  Nan::Persistent<v8::String> column_key;
  Nan::Persistent<v8::String> start_index_key;
  Nan::Persistent<v8::String> start_position_key;
  Nan::Persistent<v8::String> end_index_key;
  Nan::Persistent<v8::String> end_position_key;
  uint32_t *point_transfer_buffer = nullptr;

  // node
  uint32_t *transfer_buffer = nullptr;
  uint32_t transfer_buffer_length = 0;
  Nan::Persistent<v8::Object> module_exports;
  TSTreeCursor scratch_cursor = {nullptr, nullptr, {0, 0}};

  // parser
  Nan::Persistent<v8::Function> parser_constructor;

  // query
  TSQueryCursor *ts_query_cursor = nullptr;
  Nan::Persistent<v8::Function> query_constructor;
  Nan::Persistent<v8::FunctionTemplate> query_constructor_template;

  // tree_cursor
  Nan::Persistent<v8::Function> tree_cursor_constructor;

  // tree
  Nan::Persistent<v8::Function> tree_constructor;
  Nan::Persistent<v8::FunctionTemplate> tree_constructor_template;

private:
  static void DeleteInstance(void* data) {
    delete static_cast<AddonData*>(data);
  }
};

}

#endif  // NODE_TREE_SITTER_ADDON_DATA_H_
