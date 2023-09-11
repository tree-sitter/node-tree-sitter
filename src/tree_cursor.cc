#include "./tree_cursor.h"
#include "./conversions.h"
#include "./node.h"
#include "./tree.h"
#include "./util.h"
#include "tree_sitter/api.h"

#include <nan.h>
#include <v8.h>
#include <iostream>

namespace node_tree_sitter {

using namespace v8;

Nan::Persistent<Function> TreeCursor::constructor;
Nan::Persistent<FunctionTemplate> TreeCursor::constructor_template;

void TreeCursor::Init(v8::Local<v8::Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  Local<String> class_name = Nan::New("TreeCursor").ToLocalChecked();
  tpl->SetClassName(class_name);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  GetterPair getters[] = {
    {"startIndex", StartIndex},
    {"endIndex", EndIndex},
    {"nodeType", NodeType},
    {"nodeIsNamed", NodeIsNamed},
    {"nodeIsMissing", NodeIsMissing},
    {"currentFieldId", CurrentFieldId},
    {"currentFieldName", CurrentFieldName},
    {"currentDepth", CurrentDepth},
    {"currentDescendantIndex", CurrentDescendantIndex},
  };

  FunctionPair methods[] = {
    {"startPosition", StartPosition},
    {"endPosition", EndPosition},
    {"gotoFirstChild", GotoFirstChild},
    {"gotoLastChild", GotoLastChild},
    {"gotoParent", GotoParent},
    {"gotoNextSibling", GotoNextSibling},
    {"gotoPreviousSibling", GotoPreviousSibling},
    {"gotoDescendant", GotoDescendant},
    {"gotoFirstChildForIndex", GotoFirstChildForIndex},
    {"gotoFirstChildForPosition", GotoFirstChildForPosition},
    {"currentNode", CurrentNode},
    {"reset", Reset},
    {"resetTo", ResetTo},
  };

  for (auto & getter : getters) {
    Nan::SetAccessor(
      tpl->InstanceTemplate(),
      Nan::New(getter.name).ToLocalChecked(),
      getter.callback
    );
  }

  for (auto & method : methods) {
    Nan::SetPrototypeMethod(tpl, method.name, method.callback);
  }

  Local<Function> constructor_local = Nan::GetFunction(tpl).ToLocalChecked();
  Nan::Set(exports, class_name, constructor_local);
  constructor_template.Reset(tpl);
  constructor.Reset(Nan::Persistent<Function>(constructor_local));
}

TreeCursor::TreeCursor(TSTreeCursor cursor) : cursor_(cursor) {}

TreeCursor::~TreeCursor() { ts_tree_cursor_delete(&cursor_); }

Local<Value> TreeCursor::NewInstance(TSTreeCursor cursor) {
  Local<Object> self;
  MaybeLocal<Object> maybe_self = Nan::New(constructor)->NewInstance(Nan::GetCurrentContext());
  if (maybe_self.ToLocal(&self)) {
    (new TreeCursor(cursor))->Wrap(self);
    return self;
  }
  return Nan::Null();
}

TreeCursor *TreeCursor::UnwrapTreeCursor(const Local<Value> &value) {
  if (!value->IsObject()) {
    return nullptr;
  }
  Local<Object> js_cursor = Local<Object>::Cast(value);
  if (!Nan::New(constructor_template)->HasInstance(js_cursor)) { {
}
    return nullptr;
  }
  return ObjectWrap::Unwrap<TreeCursor>(js_cursor);
}

void TreeCursor::New(const Nan::FunctionCallbackInfo<Value> &info) {
  info.GetReturnValue().Set(Nan::Null());
}

#define read_number_from_js(out, value, name)        \
  maybe_number = Nan::To<uint32_t>(value);           \
  if (maybe_number.IsNothing()) {                    \
    Nan::ThrowTypeError(name " must be an integer"); \
    return;                                          \
  }                                                  \
  *(out) = maybe_number.FromJust();

#define read_byte_count_from_js(out, value, name)   \
  read_number_from_js(out, value, name);            \
  *(out) *= 2

void TreeCursor::GotoFirstChild(const Nan::FunctionCallbackInfo<Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());
  bool result = ts_tree_cursor_goto_first_child(&cursor->cursor_);
  info.GetReturnValue().Set(Nan::New(result));
}

void TreeCursor::GotoLastChild(const Nan::FunctionCallbackInfo<Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());
  bool result = ts_tree_cursor_goto_last_child(&cursor->cursor_);
  info.GetReturnValue().Set(Nan::New(result));
}

void TreeCursor::GotoParent(const Nan::FunctionCallbackInfo<Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());
  bool result = ts_tree_cursor_goto_parent(&cursor->cursor_);
  info.GetReturnValue().Set(Nan::New(result));
}

void TreeCursor::GotoNextSibling(const Nan::FunctionCallbackInfo<Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());
  bool result = ts_tree_cursor_goto_next_sibling(&cursor->cursor_);
  info.GetReturnValue().Set(Nan::New(result));
}

void TreeCursor::GotoPreviousSibling(const Nan::FunctionCallbackInfo<Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());
  bool result = ts_tree_cursor_goto_previous_sibling(&cursor->cursor_);
  info.GetReturnValue().Set(Nan::New(result));
}

void TreeCursor::GotoDescendant(const Nan::FunctionCallbackInfo<Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());

  uint32_t goal_descendant_index = 0;
  Nan::Maybe<uint32_t> maybe_number = Nan::Nothing<uint32_t>();
  read_number_from_js(&goal_descendant_index, info[0], "descendantIndex");

  ts_tree_cursor_goto_descendant(&cursor->cursor_, goal_descendant_index);
}

void TreeCursor::GotoFirstChildForIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());

  uint32_t goal_byte = 0;
  Nan::Maybe<uint32_t> maybe_number = Nan::Nothing<uint32_t>();
  read_byte_count_from_js(&goal_byte, info[0], "goalByte");

  int64_t child_index = ts_tree_cursor_goto_first_child_for_byte(&cursor->cursor_, goal_byte);
  if (child_index < 0) {
    info.GetReturnValue().Set(Nan::Null());
  } else {
    info.GetReturnValue().Set(Nan::New(static_cast<uint32_t>(child_index)));
  }
}

void TreeCursor::GotoFirstChildForPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());

  TSPoint goal_point;
  Nan::Maybe<uint32_t> maybe_number = Nan::Nothing<uint32_t>();
  read_number_from_js(&goal_point.row, info[0], "goalPoint.row");
  read_byte_count_from_js(&goal_point.column, info[1], "goalPoint.column");

  int64_t child_index = ts_tree_cursor_goto_first_child_for_point(&cursor->cursor_, goal_point);
  if (child_index < 0) {
    info.GetReturnValue().Set(Nan::Null());
  } else {
    info.GetReturnValue().Set(Nan::New(static_cast<uint32_t>(child_index)));
  }
}

void TreeCursor::StartPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());
  TSNode node = ts_tree_cursor_current_node(&cursor->cursor_);
  TransferPoint(ts_node_start_point(node));
}

void TreeCursor::EndPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());
  TSNode node = ts_tree_cursor_current_node(&cursor->cursor_);
  TransferPoint(ts_node_end_point(node));
}

void TreeCursor::CurrentNode(const Nan::FunctionCallbackInfo<Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());
  Local<String> key = Nan::New<String>("tree").ToLocalChecked();
  const Tree *tree = Tree::UnwrapTree(Nan::Get(info.This(), key).ToLocalChecked());
  TSNode node = ts_tree_cursor_current_node(&cursor->cursor_);
  node_methods::MarshalNode(info, tree, node);
}

void TreeCursor::Reset(const Nan::FunctionCallbackInfo<Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());
  Local<String> key = Nan::New<String>("tree").ToLocalChecked();
  const Tree *tree = Tree::UnwrapTree(Nan::Get(info.This(), key).ToLocalChecked());
  TSNode node = node_methods::UnmarshalNode(tree);
  ts_tree_cursor_reset(&cursor->cursor_, node);
}

void TreeCursor::ResetTo(const Nan::FunctionCallbackInfo<Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());
  auto *other_cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info[0]->ToObject(Nan::GetCurrentContext()).ToLocalChecked());

  ts_tree_cursor_reset_to(&cursor->cursor_, &other_cursor->cursor_);
}

void TreeCursor::NodeType(v8::Local<v8::String> /*prop*/, const Nan::PropertyCallbackInfo<v8::Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());
  TSNode node = ts_tree_cursor_current_node(&cursor->cursor_);
  info.GetReturnValue().Set(Nan::New(ts_node_type(node)).ToLocalChecked());
}

void TreeCursor::NodeIsNamed(v8::Local<v8::String> /*prop*/, const Nan::PropertyCallbackInfo<v8::Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());
  TSNode node = ts_tree_cursor_current_node(&cursor->cursor_);

  info.GetReturnValue().Set(Nan::New(ts_node_is_named(node)));
}

void TreeCursor::NodeIsMissing(v8::Local<v8::String> /*prop*/, const Nan::PropertyCallbackInfo<v8::Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());
  TSNode node = ts_tree_cursor_current_node(&cursor->cursor_);

  info.GetReturnValue().Set(Nan::New(ts_node_is_missing(node)));
}

void TreeCursor::CurrentFieldId(v8::Local<v8::String> /*prop*/, const Nan::PropertyCallbackInfo<v8::Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());
  TSFieldId field_id = ts_tree_cursor_current_field_id(&cursor->cursor_);

  if (field_id != 0) {
    info.GetReturnValue().Set(Nan::New(field_id));
  }
}

void TreeCursor::CurrentFieldName(v8::Local<v8::String> /*prop*/, const Nan::PropertyCallbackInfo<v8::Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());
  const char *field_name = ts_tree_cursor_current_field_name(&cursor->cursor_);

  if (field_name != nullptr) {
    info.GetReturnValue().Set(Nan::New(field_name).ToLocalChecked());
  }
}

void TreeCursor::CurrentDepth(v8::Local<v8::String> /*prop*/, const Nan::PropertyCallbackInfo<v8::Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());

  info.GetReturnValue().Set(Nan::New(ts_tree_cursor_current_depth(&cursor->cursor_)));
}

void TreeCursor::CurrentDescendantIndex(v8::Local<v8::String> /*prop*/, const Nan::PropertyCallbackInfo<v8::Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());

  info.GetReturnValue().Set(Nan::New(ts_tree_cursor_current_descendant_index(&cursor->cursor_)));
}

void TreeCursor::StartIndex(v8::Local<v8::String> /*prop*/, const Nan::PropertyCallbackInfo<v8::Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());
  TSNode node = ts_tree_cursor_current_node(&cursor->cursor_);
  info.GetReturnValue().Set(ByteCountToJS(ts_node_start_byte(node)));
}

void TreeCursor::EndIndex(v8::Local<v8::String> /*prop*/, const Nan::PropertyCallbackInfo<v8::Value> &info) {
  auto *cursor = Nan::ObjectWrap::Unwrap<TreeCursor>(info.This());
  TSNode node = ts_tree_cursor_current_node(&cursor->cursor_);
  info.GetReturnValue().Set(ByteCountToJS(ts_node_end_byte(node)));
}

} // namespace node_tree_sitter
