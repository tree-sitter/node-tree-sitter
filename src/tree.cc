#include "./tree.h"
#include <string>
#include <napi.h>
#include "./node.h"
#include "./logger.h"
#include "./util.h"
#include "./conversions.h"

namespace node_tree_sitter {

using namespace Napi;

FunctionReference Tree::constructor;

void Tree::Init(Object &exports) {
  Napi::Env env = exports.Env();

  Function ctor = DefineClass(env, "Tree", {
    InstanceMethod("edit", &Tree::Edit, napi_writable),
    InstanceMethod("rootNode", &Tree::RootNode, napi_configurable),
    InstanceMethod("printDotGraph", &Tree::PrintDotGraph),
    InstanceMethod("getChangedRanges", &Tree::GetChangedRanges),
    InstanceMethod("getEditedRange", &Tree::GetEditedRange),
    InstanceMethod("_cacheNode", &Tree::CacheNode),
  });

  constructor.Reset(ctor, 1);
  exports["Tree"] = ctor;
}

Tree::Tree(const Napi::CallbackInfo& info)
  : Napi::ObjectWrap<Tree>(info),
    tree_(nullptr) {}

Tree::~Tree() {
  if (tree_) ts_tree_delete(tree_);
  for (auto &entry : cached_nodes_) {
    entry.second->tree = nullptr;
  }
}

Napi::Value Tree::NewInstance(Napi::Env env, TSTree *tree) {
  if (tree) {
    Object js_tree = constructor.Value().New({});
    Tree::Unwrap(js_tree)->tree_ = tree;
    return js_tree;
  }
  return env.Null();
}

const Tree *Tree::UnwrapTree(const Napi::Value &value) {
  return Tree::Unwrap(value.As<Object>());
}

#define read_number_from_js(out, value, name)                                \
  if (!value.IsNumber()) {                                                   \
    TypeError::New(env, name " must be an integer").ThrowAsJavaScriptException(); \
    return env.Undefined();                                                  \
  }                                                                          \
  *(out) = value.As<Number>().Uint32Value();

#define read_byte_count_from_js(out, value, name)   \
  read_number_from_js(out, value, name);            \
  (*out) *= 2

Napi::Value Tree::Edit(const CallbackInfo &info) {
  auto env = info.Env();

  TSInputEdit edit;
  read_number_from_js(&edit.start_point.row, info[0], "startPosition.row");
  read_byte_count_from_js(&edit.start_point.column, info[1], "startPosition.column");
  read_number_from_js(&edit.old_end_point.row, info[2], "oldEndPosition.row");
  read_byte_count_from_js(&edit.old_end_point.column, info[3], "oldEndPosition.column");
  read_number_from_js(&edit.new_end_point.row, info[4], "newEndPosition.row");
  read_byte_count_from_js(&edit.new_end_point.column, info[5], "newEndPosition.column");
  read_byte_count_from_js(&edit.start_byte, info[6], "startIndex");
  read_byte_count_from_js(&edit.old_end_byte, info[7], "oldEndIndex");
  read_byte_count_from_js(&edit.new_end_byte, info[8], "newEndIndex");

  ts_tree_edit(tree_, &edit);

  for (auto &entry : cached_nodes_) {
    Object js_node = entry.second->node.Value();
    TSNode node;
    node.id = entry.first;
    for (unsigned i = 0; i < 4; i++) {
      Napi::Value node_field = js_node[i + 2u];
      if (node_field.IsNumber()) {
        node.context[i] = node_field.As<Number>().Uint32Value();
      }
    }

    ts_node_edit(&node, &edit);

    for (unsigned i = 0; i < 4; i++) {
      js_node[i + 2u] = Number::New(env, node.context[i]);
    }
  }

  return info.This();
}

Napi::Value Tree::RootNode(const CallbackInfo &info) {
  return MarshalNode(info.Env(), this, ts_tree_root_node(tree_));
}

Napi::Value Tree::GetChangedRanges(const CallbackInfo &info) {
  auto env = info.Env();
  const Tree *other_tree = UnwrapTree(info[0]);
  if (!other_tree) return env.Undefined();

  uint32_t range_count;
  TSRange *ranges = ts_tree_get_changed_ranges(tree_, other_tree->tree_, &range_count);

  Array result = Array::New(env);
  for (unsigned i = 0; i < range_count; i++) {
    result[i] = RangeToJS(env, ranges[i]);
  }

  return result;
}

Napi::Value Tree::GetEditedRange(const CallbackInfo &info) {
  auto env = info.Env();
  TSNode root = ts_tree_root_node(tree_);
  if (!ts_node_has_changes(root)) return env.Undefined();
  TSRange result = {
    ts_node_start_point(root),
    ts_node_end_point(root),
    ts_node_start_byte(root),
    ts_node_end_byte(root),
  };

  TSTreeCursor cursor = ts_tree_cursor_new(root);

  while (true) {
    if (!ts_tree_cursor_goto_first_child(&cursor)) break;
    while (true) {
      TSNode node = ts_tree_cursor_current_node(&cursor);
      if (ts_node_has_changes(node)) {
        result.start_byte = ts_node_start_byte(node);
        result.start_point = ts_node_start_point(node);
        break;
      } else if (!ts_tree_cursor_goto_next_sibling(&cursor)) {
        break;
      }
    }
  }

  while (ts_tree_cursor_goto_parent(&cursor)) {}

  while (true) {
    if (!ts_tree_cursor_goto_first_child(&cursor)) break;
    while (true) {
      TSNode node = ts_tree_cursor_current_node(&cursor);
      if (ts_node_has_changes(node)) {
        result.end_byte = ts_node_end_byte(node);
        result.end_point = ts_node_end_point(node);
      }

      if (!ts_tree_cursor_goto_next_sibling(&cursor)) {
        break;
      }
    }
  }

  ts_tree_cursor_delete(&cursor);
  return RangeToJS(env, result);
}

Napi::Value Tree::PrintDotGraph(const CallbackInfo &info) {
  ts_tree_print_dot_graph(tree_, stderr);
  return info.This();
}

static void FinalizeNode(Env env, Tree::NodeCacheEntry *cache_entry) {
  //assert(!cache_entry->node.IsEmpty());
  cache_entry->node.Reset();
  if (cache_entry->tree) {
    //assert(cache_entry->tree->cached_nodes_.count(cache_entry->key));
    cache_entry->tree->cached_nodes_.erase(cache_entry->key);
  }
  delete cache_entry;
}

Napi::Value Tree::CacheNode(const CallbackInfo &info) {
  auto env = info.Env();
  Object js_node = info[0].As<Object>();

  Napi::Value js_node_field1 = js_node[0u];
  Napi::Value js_node_field2 = js_node[1u];
  if (!js_node_field1.IsNumber() || !js_node_field2.IsNumber()) {
    return env.Undefined();
  }
  uint32_t key_parts[2] = {
    js_node_field1.As<Number>().Uint32Value(),
    js_node_field2.As<Number>().Uint32Value(),
  };
  const void *key = UnmarshalPointer(key_parts);

  auto cache_entry = new NodeCacheEntry{this, key, {}};
  cache_entry->node.Reset(js_node, 0);
  js_node.AddFinalizer(&FinalizeNode, cache_entry);

  //assert(!cached_nodes_.count(key));

  cached_nodes_[key] = cache_entry;
  return env.Undefined();
}


}  // namespace node_tree_sitter
