#include "./tree_cursor.h"
#include <napi.h>
#include <tree_sitter/api.h>
#include "./util.h"
#include "./conversions.h"
#include "./node.h"
#include "./tree.h"

using namespace Napi;

namespace node_tree_sitter {

void TreeCursor::Init(Napi::Env env, Napi::Object exports) {
  auto data = env.GetInstanceData<AddonData>();

  Function ctor = DefineClass(env, "TreeCursor", {
    InstanceAccessor("startIndex", &TreeCursor::StartIndex, nullptr, napi_default_method),
    InstanceAccessor("endIndex", &TreeCursor::EndIndex, nullptr, napi_default_method),
    InstanceAccessor("nodeType", &TreeCursor::NodeType, nullptr, napi_default_method),
    InstanceAccessor("nodeIsNamed", &TreeCursor::NodeIsNamed, nullptr, napi_default_method),
    InstanceAccessor("nodeIsMissing", &TreeCursor::NodeIsMissing, nullptr, napi_default_method),
    InstanceAccessor("currentFieldName", &TreeCursor::CurrentFieldName, nullptr, napi_default_method),

    InstanceMethod("startPosition", &TreeCursor::StartPosition, napi_default_method),
    InstanceMethod("endPosition", &TreeCursor::EndPosition, napi_default_method),
    InstanceMethod("gotoParent", &TreeCursor::GotoParent, napi_default_method),
    InstanceMethod("gotoFirstChild", &TreeCursor::GotoFirstChild, napi_default_method),
    InstanceMethod("gotoFirstChildForIndex", &TreeCursor::GotoFirstChildForIndex, napi_default_method),
    InstanceMethod("gotoNextSibling", &TreeCursor::GotoNextSibling, napi_default_method),
    InstanceMethod("currentNode", &TreeCursor::CurrentNode, napi_default_method),
    InstanceMethod("reset", &TreeCursor::Reset, napi_default_method),
  });

  exports["TreeCursor"] = ctor;
  data->tree_cursor_constructor = Napi::Persistent(ctor);
}

Napi::Value TreeCursor::NewInstance(Napi::Env env, TSTreeCursor cursor) {
  auto data = env.GetInstanceData<AddonData>();

  Object self = data->tree_cursor_constructor.New({});
  TreeCursor::Unwrap(self)->cursor_ = cursor;
  return self;
}

TreeCursor::TreeCursor(const Napi::CallbackInfo& info) : Napi::ObjectWrap<TreeCursor>(info), cursor_() {}

TreeCursor::~TreeCursor() { ts_tree_cursor_delete(&cursor_); }

Napi::Value TreeCursor::GotoParent(const Napi::CallbackInfo &info) {
  bool result = ts_tree_cursor_goto_parent(&cursor_);
  return Boolean::New(info.Env(), result);
}

Napi::Value TreeCursor::GotoFirstChild(const Napi::CallbackInfo &info) {
  bool result = ts_tree_cursor_goto_first_child(&cursor_);
  return Boolean::New(info.Env(), result);
}

Napi::Value TreeCursor::GotoFirstChildForIndex(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  if (!info[0].IsNumber()) {
    throw TypeError::New(env, "Argument must be an integer");
  }
  auto index = info[0].As<Number>();
  uint32_t goal_byte = index.Uint32Value() * 2;
  int64_t child_index = ts_tree_cursor_goto_first_child_for_byte(&cursor_, goal_byte);
  if (child_index < 0) {
    return env.Null();
  } else {
    return Number::New(env, child_index);
  }
}

Napi::Value TreeCursor::GotoNextSibling(const Napi::CallbackInfo &info) {
  bool result = ts_tree_cursor_goto_next_sibling(&cursor_);
  return Boolean::New(info.Env(), result);
}

Napi::Value TreeCursor::StartPosition(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  TSNode node = ts_tree_cursor_current_node(&cursor_);
  TransferPoint(env, ts_node_start_point(node));
  return env.Undefined();
}

Napi::Value TreeCursor::EndPosition(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  TSNode node = ts_tree_cursor_current_node(&cursor_);
  TransferPoint(env, ts_node_end_point(node));
  return env.Undefined();
}

Napi::Value TreeCursor::CurrentNode(const Napi::CallbackInfo &info) {
  const Tree *tree = Tree::UnwrapTree(info.This().As<Object>()["tree"]);
  TSNode node = ts_tree_cursor_current_node(&cursor_);
  return node_methods::MarshalNode(info, tree, node);
}

Napi::Value TreeCursor::Reset(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info.This().As<Object>()["tree"]);
  TSNode node = node_methods::UnmarshalNode(env, tree);
  ts_tree_cursor_reset(&cursor_, node);
  return env.Undefined();
}

Napi::Value TreeCursor::NodeType(const Napi::CallbackInfo &info) {
  TSNode node = ts_tree_cursor_current_node(&cursor_);
  return String::New(info.Env(), ts_node_type(node));;
}

Napi::Value TreeCursor::NodeIsNamed(const Napi::CallbackInfo &info) {
  TSNode node = ts_tree_cursor_current_node(&cursor_);
  return Boolean::New(info.Env(), ts_node_is_named(node));
}

Napi::Value TreeCursor::NodeIsMissing(const Napi::CallbackInfo &info) {
  TSNode node = ts_tree_cursor_current_node(&cursor_);
  return Boolean::New(info.Env(), ts_node_is_missing(node));
}

Napi::Value TreeCursor::CurrentFieldName(const Napi::CallbackInfo &info) {
  const char *field_name = ts_tree_cursor_current_field_name(&cursor_);
  if (field_name) {
    return String::New(info.Env(), field_name);
  }
  return info.Env().Undefined();
}

Napi::Value TreeCursor::StartIndex(const Napi::CallbackInfo &info) {
  TSNode node = ts_tree_cursor_current_node(&cursor_);
  return ByteCountToJS(info.Env(), ts_node_start_byte(node));
}

Napi::Value TreeCursor::EndIndex(const Napi::CallbackInfo &info) {
  TSNode node = ts_tree_cursor_current_node(&cursor_);
  return ByteCountToJS(info.Env(), ts_node_end_byte(node));
}

}
