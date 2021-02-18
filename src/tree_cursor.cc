#include "./tree_cursor.h"
#include <tree_sitter/api.h>
#include <napi.h>
#include "./util.h"
#include "./conversions.h"
#include "./node.h"
#include "./tree.h"

namespace node_tree_sitter {

using namespace Napi;

class TreeCursor : public Napi::ObjectWrap<TreeCursor> {
 public:
  static void Init(Napi::Object &exports, InstanceData * instance) {
    Napi::Env env = exports.Env();

    Function ctor = DefineClass(env, "TreeCursor", {
      InstanceAccessor("startIndex", &TreeCursor::StartIndex, nullptr),
      InstanceAccessor("endIndex", &TreeCursor::EndIndex, nullptr),
      InstanceAccessor("nodeType", &TreeCursor::NodeType, nullptr),
      InstanceAccessor("nodeIsNamed", &TreeCursor::NodeIsNamed, nullptr),
      InstanceAccessor("currentFieldName", &TreeCursor::CurrentFieldName, nullptr),

      InstanceMethod("_startPosition", &TreeCursor::StartPosition, napi_configurable),
      InstanceMethod("_endPosition", &TreeCursor::EndPosition, napi_configurable),
      InstanceMethod("gotoParent", &TreeCursor::GotoParent),
      InstanceMethod("gotoFirstChild", &TreeCursor::GotoFirstChild),
      InstanceMethod("gotoFirstChildForIndex", &TreeCursor::GotoFirstChildForIndex),
      InstanceMethod("gotoNextSibling", &TreeCursor::GotoNextSibling),
      InstanceMethod("_currentNode", &TreeCursor::CurrentNode, napi_configurable),
      InstanceMethod("_reset", &TreeCursor::Reset),
    });

    instance->tree_cursor_constructor = new Napi::FunctionReference();
    (*instance->tree_cursor_constructor) = Napi::Persistent(ctor);
    exports.Set("TreeCursor", ctor);
  }

  TreeCursor(const CallbackInfo &info)
    : Napi::ObjectWrap<TreeCursor>(info),
      cursor_({0, 0, {0, 0}})
      {}

  ~TreeCursor() { ts_tree_cursor_delete(&cursor_); }

  Napi::Value GotoParent(const CallbackInfo &info) {
    auto env = info.Env();
    bool result = ts_tree_cursor_goto_parent(&cursor_);
    return Boolean::New(env, result);
  }

  Napi::Value GotoFirstChild(const CallbackInfo &info) {
    auto env = info.Env();
    bool result = ts_tree_cursor_goto_first_child(&cursor_);
    return Boolean::New(env, result);
  }

  Napi::Value GotoFirstChildForIndex(const CallbackInfo &info) {
    auto env = info.Env();
    auto js_index = info[0].As<Napi::Number>();
    if (!js_index.IsNumber()) {
      TypeError::New(env, "Argument must be an integer").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    uint32_t goal_byte = js_index.Uint32Value() * 2;
    int64_t child_index = ts_tree_cursor_goto_first_child_for_byte(&cursor_, goal_byte);
    if (child_index < 0) {
      return env.Null();
    } else {
      return Number::New(env, child_index);
    }
  }

  Napi::Value GotoNextSibling(const CallbackInfo &info) {
    auto env = info.Env();
    bool result = ts_tree_cursor_goto_next_sibling(&cursor_);
    return Boolean::New(env, result);
  }

  Napi::Value StartPosition(const CallbackInfo &info) {
    auto env = info.Env();
    TSNode node = ts_tree_cursor_current_node(&cursor_);
    TransferPoint(env, ts_node_start_point(node));
    return env.Undefined();
  }

  Napi::Value EndPosition(const CallbackInfo &info) {
    auto env = info.Env();
    TSNode node = ts_tree_cursor_current_node(&cursor_);
    TransferPoint(env, ts_node_end_point(node));
    return env.Undefined();
  }

  Napi::Value CurrentNode(const CallbackInfo &info) {
    auto env = info.Env();
    Napi::Value js_tree = info.This().As<Napi::Object>()["tree"];
    const Tree *tree = Tree::UnwrapTree(js_tree.As<Napi::Object>());
    TSNode node = ts_tree_cursor_current_node(&cursor_);
    return MarshalNode(env, tree, node);
  }

  Napi::Value Reset(const CallbackInfo &info) {
    auto env = info.Env();
    Napi::Value js_tree = info.This().As<Napi::Object>()["tree"];
    const Tree *tree = Tree::UnwrapTree(js_tree.As<Napi::Object>());
    TSNode node = UnmarshalNode(env, tree);
    ts_tree_cursor_reset(&cursor_, node);
    return env.Undefined();
  }

  Napi::Value NodeType(const CallbackInfo &info) {
    auto env = info.Env();
    TSNode node = ts_tree_cursor_current_node(&cursor_);
    return String::New(env, ts_node_type(node));
  }

  Napi::Value NodeIsNamed(const CallbackInfo &info) {
    auto env = info.Env();
    TSNode node = ts_tree_cursor_current_node(&cursor_);
    return Boolean::New(env, ts_node_is_named(node));
  }

  Napi::Value CurrentFieldName(const CallbackInfo &info) {
    auto env = info.Env();
    const char *field_name = ts_tree_cursor_current_field_name(&cursor_);
    if (field_name) {
      return String::New(env, field_name);
    } else {
      return env.Undefined();
    }
  }

  Napi::Value StartIndex(const CallbackInfo &info) {
    auto env = info.Env();
    TSNode node = ts_tree_cursor_current_node(&cursor_);
    return ByteCountToJS(env, ts_node_start_byte(node));
  }

  Napi::Value EndIndex(const CallbackInfo &info) {
    auto env = info.Env();
    TSNode node = ts_tree_cursor_current_node(&cursor_);
    return ByteCountToJS(env, ts_node_end_byte(node));
  }

  TSTreeCursor cursor_;
  static Napi::FunctionReference constructor;
};

void InitTreeCursor(Napi::Object &exports, InstanceData * instance) {
  TreeCursor::Init(exports, instance);
}

Napi::Value NewTreeCursor(Napi::Env env, TSTreeCursor cursor) {
  InstanceData *instance = GetInternalData(env);
  Napi::Object js_cursor = instance->tree_cursor_constructor->New({});

  TreeCursor *ret_tree_cursor = nullptr;
  if (js_cursor.IsObject()) {
    ret_tree_cursor = Napi::ObjectWrap<TreeCursor>::Unwrap(js_cursor);
    ret_tree_cursor->cursor_ = cursor;
    return js_cursor;
  }
  return env.Null();
}

FunctionReference TreeCursor::constructor;

}  // namespace node_tree_sitter
