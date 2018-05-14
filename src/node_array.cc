#include "./node_array.h"
#include <nan.h>
#include <tree_sitter/runtime.h>
#include <v8.h>
#include <vector>
#include <string>
#include "./util.h"
#include "./node.h"

namespace node_tree_sitter {

using namespace v8;

Nan::Persistent<Function> NodeArray::constructor;

void NodeArray::Init(v8::Local<v8::Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("NodeArray").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetIndexedPropertyHandler(
    tpl->InstanceTemplate(),
    GetIndex,
    NULL);

  Nan::SetAccessor(
    tpl->InstanceTemplate(),
    Nan::New("length").ToLocalChecked(),
    Length, NULL);

  Nan::SetAccessor(
    tpl->InstanceTemplate(),
    Nan::New("isNamed").ToLocalChecked(),
    IsNamed);

  const char * array_methods[] = {
    "every",
    "filter",
    "find",
    "findIndex",
    "forEach",
    "indexOf",
    "map",
    "reduce",
    "reduceRight",
    "slice",
    "some",
  };

  Local<Function> constructor_local = tpl->GetFunction();
  Local<Object> prototype = Local<Object>::Cast(constructor_local->Get(Nan::New("prototype").ToLocalChecked()));

  Local<Array> array = Nan::New<Array>();
  for (size_t i = 0; i < length_of_array(array_methods); i++) {
    Local<String> method_name = Nan::New(array_methods[i]).ToLocalChecked();
    prototype->Set(method_name, array->Get(method_name));
  }

  exports->Set(Nan::New("NodeArray").ToLocalChecked(), constructor_local);
  constructor.Reset(Nan::Persistent<Function>(constructor_local));
}

NodeArray::NodeArray(TSNode node, bool is_named) : parent_node_(node), is_named_(is_named)  {}

Local<Value> NodeArray::NewInstance(TSNode node, bool is_named) {
  MaybeLocal<Object> maybe_self = Nan::New(constructor)->NewInstance(Nan::GetCurrentContext());
  Local<Object> self;
  if (maybe_self.ToLocal(&self)) {
    (new NodeArray(node, is_named))->Wrap(self);
    return self;
  } else {
    return Nan::Null();
  }
}

void NodeArray::New(const Nan::FunctionCallbackInfo<Value> &info) {
  info.GetReturnValue().Set(Nan::Null());
}

void NodeArray::GetIndex(uint32_t index, const Nan::PropertyCallbackInfo<Value> &info) {
  NodeArray *array = ObjectWrap::Unwrap<NodeArray>(info.This());
  TSNode child = array->is_named_ ?
    ts_node_named_child(array->parent_node_, index) :
    ts_node_child(array->parent_node_, index);
  if (child.subtree) {
    info.GetReturnValue().Set(Node::NewInstance(child));
  }
}

void NodeArray::Length(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  NodeArray *array = ObjectWrap::Unwrap<NodeArray>(info.This());
  uint32_t length = array->is_named_ ?
    ts_node_named_child_count(array->parent_node_) :
    ts_node_child_count(array->parent_node_);
  info.GetReturnValue().Set(Nan::New<Number>(length));
}

void NodeArray::IsNamed(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  NodeArray *array = ObjectWrap::Unwrap<NodeArray>(info.This());
  info.GetReturnValue().Set(array->is_named_);
}

}  // namespace node_tree_sitter
