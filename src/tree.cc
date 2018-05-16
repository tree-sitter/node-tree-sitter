#include "./tree.h"
#include <string>
#include <v8.h>
#include <nan.h>
#include "./input_reader.h"
#include "./node.h"
#include "./logger.h"
#include "./util.h"
#include "./conversions.h"
#include "./tree_cursor.h"

namespace node_tree_sitter {

using namespace v8;

Nan::Persistent<Function> Tree::constructor;
Nan::Persistent<FunctionTemplate> Tree::constructor_template;

void Tree::Init(Local<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Local<String> class_name = Nan::New("Tree").ToLocalChecked();
  tpl->SetClassName(class_name);

  Nan::SetAccessor(
    tpl->InstanceTemplate(),
    Nan::New("rootNode").ToLocalChecked(),
    RootNode);

  FunctionPair methods[] = {
    {"edit", Edit},
    {"walk", Walk},
    {"printDotGraph", PrintDotGraph},
    {"getChangedRanges", GetChangedRanges},
  };

  for (size_t i = 0; i < length_of_array(methods); i++) {
    Nan::SetPrototypeMethod(tpl, methods[i].name, methods[i].callback);
  }

  Local<Function> ctor = tpl->GetFunction();

  constructor_template.Reset(tpl);
  constructor.Reset(ctor);
  exports->Set(class_name, ctor);
}

Tree::Tree(TSTree *tree) : tree_(tree) {}

Tree::~Tree() { ts_tree_delete(tree_); }

Local<Value> Tree::NewInstance(TSTree *tree) {
  Local<Object> self;
  MaybeLocal<Object> maybe_self = Nan::New(constructor)->NewInstance(Nan::GetCurrentContext());
  if (maybe_self.ToLocal(&self)) {
    (new Tree(tree))->Wrap(self);
    return self;
  } else {
    return Nan::Null();
  }
}

const TSTree *Tree::UnwrapTree(const Local<Value> &value) {
  if (!value->IsObject()) return nullptr;
  Local<Object> js_tree = Local<Object>::Cast(value);
  if (!Nan::New(constructor_template)->HasInstance(js_tree)) {
    return nullptr;
  }

  return ObjectWrap::Unwrap<Tree>(js_tree)->tree_;
}

void Tree::New(const Nan::FunctionCallbackInfo<Value> &info) {
  info.GetReturnValue().Set(Nan::Null());
}

void Tree::Edit(const Nan::FunctionCallbackInfo<Value> &info) {
  Local<Object> arg = Local<Object>::Cast(info[0]);
  Tree *tree = ObjectWrap::Unwrap<Tree>(info.This());

  auto start_byte = ByteCountFromJS(arg->Get(Nan::New("startIndex").ToLocalChecked()));
  if (start_byte.IsNothing()) return;

  auto bytes_removed = ByteCountFromJS(arg->Get(Nan::New("lengthRemoved").ToLocalChecked()));
  if (bytes_removed.IsNothing()) return;

  auto bytes_added = ByteCountFromJS(arg->Get(Nan::New("lengthAdded").ToLocalChecked()));
  if (bytes_added.IsNothing()) return;

  auto start_position = PointFromJS(arg->Get(Nan::New("startPosition").ToLocalChecked()));
  if (start_position.IsNothing()) return;

  auto extent_removed = PointFromJS(arg->Get(Nan::New("extentRemoved").ToLocalChecked()));
  if (extent_removed.IsNothing()) return;

  auto extent_added = PointFromJS(arg->Get(Nan::New("extentAdded").ToLocalChecked()));
  if (extent_added.IsNothing()) return;

  TSInputEdit edit;
  edit.start_byte = start_byte.FromJust();
  edit.bytes_removed = bytes_removed.FromJust();
  edit.bytes_added = bytes_added.FromJust();
  edit.start_point = start_position.FromJust();
  edit.extent_removed = extent_removed.FromJust();
  edit.extent_added = extent_added.FromJust();
  ts_tree_edit(tree->tree_, &edit);
  info.GetReturnValue().Set(info.This());
}

void Tree::Walk(const Nan::FunctionCallbackInfo<Value> &info) {
  Tree *tree = ObjectWrap::Unwrap<Tree>(info.This());
  TSTreeCursor cursor = ts_tree_cursor_new(tree->tree_);
  info.GetReturnValue().Set(TreeCursor::NewInstance(cursor));
}

void Tree::RootNode(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  Tree *tree = ObjectWrap::Unwrap<Tree>(info.This());
  TSNode node = ts_tree_root_node(tree->tree_);
  if (node.id) {
    info.GetReturnValue().Set(Node::NewInstance(node));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

void Tree::GetChangedRanges(const Nan::FunctionCallbackInfo<Value> &info) {
  Tree *tree = ObjectWrap::Unwrap<Tree>(info.This());
  const TSTree *other_tree = UnwrapTree(info[0]);
  if (!other_tree) {
    Nan::ThrowTypeError("Argument must be a tree");
    return;
  }

  uint32_t range_count;
  TSRange *ranges = ts_tree_get_changed_ranges(tree->tree_, other_tree, &range_count);

  Local<Array> result = Nan::New<Array>();
  for (size_t i = 0; i < range_count; i++) {
    result->Set(i, RangeToJS(ranges[i]));
  }

  info.GetReturnValue().Set(result);
}

void Tree::PrintDotGraph(const Nan::FunctionCallbackInfo<Value> &info) {
  Tree *tree = ObjectWrap::Unwrap<Tree>(info.This());
  ts_tree_print_dot_graph(tree->tree_, stderr);
  info.GetReturnValue().Set(info.This());
}

}  // namespace node_tree_sitter
