#include "./tree.h"
#include <string>
#include <napi.h>
#include <uv.h>
#include "./binding.h"
#include "./node.h"
#include "./logger.h"
#include "./util.h"
#include "./conversions.h"

namespace node_tree_sitter {

using namespace Napi;

void Tree::Init(Napi::Object &exports, InstanceData *instance) {
  Napi::Env env = exports.Env();

  Napi::Function ctor = DefineClass(env, "Tree", {
    InstanceMethod("_edit", &Tree::Edit),
    InstanceMethod("_rootNode", &Tree::RootNode),
    InstanceMethod("printDotGraph", &Tree::PrintDotGraph),
    InstanceMethod("getChangedRanges", &Tree::GetChangedRanges),
    InstanceMethod("getEditedRange", &Tree::GetEditedRange),
    InstanceMethod("_cacheNode", &Tree::CacheNode),
    InstanceMethod("_cacheNodes", &Tree::CacheNodes),
  });

  instance->tree_constructor = new Napi::FunctionReference();
  (*instance->tree_constructor) = Napi::Persistent(ctor);
  exports["Tree"] = ctor;
}

Tree::Tree(const Napi::CallbackInfo& info)
  : Napi::ObjectWrap<Tree>(info) {
  this->tree_ = nullptr;
}

Tree::~Tree() {
  if (this->tree_) ts_tree_delete(this->tree_);
  for (auto &entry : cached_nodes_) {
    entry.second->tree = nullptr;
  }
}

Napi::Value Tree::NewInstance(Napi::Env env, TSTree *tree) {
  if (tree) {
    InstanceData *instance = GetInternalData(env);
    Napi::Object js_tree = instance->tree_constructor->New({});

    Tree *ret_tree = nullptr;
    if (js_tree.IsObject()) {
      ret_tree = Napi::ObjectWrap<Tree>::Unwrap(js_tree);
      ret_tree->tree_ = tree;
      return js_tree;
    }
  }
  return env.Null();
}

const Tree *Tree::UnwrapTree(const Napi::Value &value) {
  const Tree *tree = nullptr;
  if (value.IsObject()) {
    tree = Napi::ObjectWrap<Tree>::Unwrap(value.As<Napi::Object>());
  }
  return tree;
}

#define read_number_from_js(out, value, name)                                \
  if (!value.IsNumber()) {                                                   \
    TypeError::New(env, name " must be an integer").ThrowAsJavaScriptException(); \
    return env.Undefined();                                                  \
  }                                                                          \
  *(out) = value.As<Napi::Number>().Uint32Value();

#define read_byte_count_from_js(out, value, name)   \
  read_number_from_js(out, value, name);            \
  (*out) *= 2

Napi::Value Tree::Edit(const CallbackInfo &info) {
  Napi::Env env = info.Env();

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
    Napi::Object js_node = entry.second->node.Value();
    TSNode node;
    node.id = entry.first;
    for (unsigned i = 0; i < 4; i++) {
      Napi::Value node_field = js_node[i + 2u];
      if (node_field.IsNumber()) {
        node.context[i] = node_field.As<Napi::Number>().Uint32Value();
      }
    }

    ts_node_edit(&node, &edit);

    for (unsigned i = 0; i < 4; i++) {
      js_node[i + 2u] = Napi::Number::New(env, node.context[i]);
    }
  }

  return info.This();
}

Napi::Value Tree::RootNode(const CallbackInfo &info) {
  return MarshalNode(info.Env(), this, ts_tree_root_node(tree_));
}

Napi::Value Tree::GetChangedRanges(const CallbackInfo &info) {
  Napi::Env env = info.Env();
  const Tree *other_tree = UnwrapTree(info[0]);
  if (!other_tree) return env.Undefined();

  uint32_t range_count;
  TSRange *ranges = ts_tree_get_changed_ranges(tree_, other_tree->tree_, &range_count);

  Napi::Array result = Napi::Array::New(env);
  for (unsigned i = 0; i < range_count; i++) {
    result[i] = RangeToJS(env, ranges[i]);
  }

  return result;
}

Napi::Value Tree::GetEditedRange(const CallbackInfo &info) {
  Napi::Env env = info.Env();
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
  ts_tree_print_dot_graph(this->tree_, stderr);
  return info.This();
}

static void FinalizeNode(Napi::Env, Tree::NodeCacheEntry *cache_entry) {
  assert(!cache_entry->node.IsEmpty());
  cache_entry->node.Reset();
  if (cache_entry->tree) {
    assert(cache_entry->tree->cached_nodes_.count(cache_entry->key));
    cache_entry->tree->cached_nodes_.erase(cache_entry->key);
  }
  delete cache_entry;
}

Napi::Value Tree::ExtractCacheNode(Napi::Env env, Napi::Object &js_node) {

  Napi::Value js_node_field1 = js_node[0u];
  Napi::Value js_node_field2 = js_node[1u];
  if (!js_node_field1.IsNumber() || !js_node_field2.IsNumber()) {
    return env.Undefined();
  }
  uint32_t key_parts[2] = {
    js_node_field1.As<Napi::Number>().Uint32Value(),
    js_node_field2.As<Napi::Number>().Uint32Value(),
  };
  const void *key = UnmarshalPointer(key_parts);

  auto cache_entry = new Tree::NodeCacheEntry{this, key, {}};
  cache_entry->node.Reset(js_node, 0);
  js_node.AddFinalizer(FinalizeNode, cache_entry);

  assert(!cached_nodes_.count(key));

  cached_nodes_[key] = cache_entry;
  return env.Undefined();
}

Napi::Value Tree::CacheNode(const Napi::CallbackInfo&info) {
  Napi::Env env = info.Env();
  Napi::Object js_node = info[0].As<Napi::Object>();
  return this->ExtractCacheNode(env, js_node);
}

Napi::Value Tree::CacheNodes(const Napi::CallbackInfo&info) {
  Napi::Env env = info.Env();

  if (!info[0].IsArray()) {
    Napi::TypeError::New(env, "Must pass an array of nodes").ThrowAsJavaScriptException();
    return env.Undefined();
  }
  Napi::Array js_nodes = info[0].As<Napi::Array>();
  uint32_t length = js_nodes.Length();

  for (uint32_t i = 0; i < length; i++) {
    Napi::Value js_node = js_nodes.Get(i);
    if (js_node.IsObject()) {
      continue;
    }
    Napi::TypeError::New(env, "Must pass an array of nodes").ThrowAsJavaScriptException();
    return env.Undefined();
  }
  for (uint32_t i = 0; i < length; i++) {
    Napi::Object js_node = js_nodes.Get(i).As<Napi::Object>();
    js_nodes.Set(i, this->ExtractCacheNode(env, js_node));
  }
  return js_nodes;
}

}  // namespace node_tree_sitter
