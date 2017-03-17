#include "./ast_node_array.h"
#include <nan.h>
#include <tree_sitter/runtime.h>
#include <v8.h>
#include <vector>
#include <string>
#include "./util.h"
#include "./ast_node.h"

namespace node_tree_sitter {

using namespace v8;

Nan::Persistent<Function> ASTNodeArray::constructor;

void ASTNodeArray::Init() {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("ASTNodeArray").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetIndexedPropertyHandler(
    tpl->InstanceTemplate(),
    GetIndex,
    NULL);

  Nan::SetAccessor(
    tpl->InstanceTemplate(),
    Nan::New("length").ToLocalChecked(),
    Length, NULL);

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

  constructor.Reset(Nan::Persistent<Function>(constructor_local));
}

ASTNodeArray::ASTNodeArray(TSNode node, TSDocument *document, size_t parse_count, bool is_named) :
  parent_node_(node), document_(document), parse_count_(parse_count), is_named_(is_named)  {}

Local<Value> ASTNodeArray::NewInstance(TSNode node, TSDocument *document, size_t parse_count, bool is_named) {
  MaybeLocal<Object> maybe_self = Nan::New(constructor)->NewInstance(Nan::GetCurrentContext());
  Local<Object> self;
  if (maybe_self.ToLocal(&self)) {
    (new ASTNodeArray(node, document, parse_count, is_named))->Wrap(self);
    return self;
  } else {
    return Nan::Null();
  }
}

NAN_METHOD(ASTNodeArray::New) {
  info.GetReturnValue().Set(Nan::Null());
}

NAN_INDEX_GETTER(ASTNodeArray::GetIndex) {
  ASTNodeArray *array = ObjectWrap::Unwrap<ASTNodeArray>(info.This());
  TSNode child = array->is_named_ ?
    ts_node_named_child(array->parent_node_, index) :
    ts_node_child(array->parent_node_, index);
  if (child.data)
    info.GetReturnValue().Set(ASTNode::NewInstance(child, array->document_, array->parse_count_));
  else
    info.GetReturnValue().Set(Nan::Undefined());
}

NAN_GETTER(ASTNodeArray::Length) {
  ASTNodeArray *array = ObjectWrap::Unwrap<ASTNodeArray>(info.This());
  uint32_t length = array->is_named_ ?
    ts_node_named_child_count(array->parent_node_) :
    ts_node_child_count(array->parent_node_);
  info.GetReturnValue().Set(Nan::New<Number>(length));
}

}  // namespace node_tree_sitter
