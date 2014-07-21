#include "./ast_node_array.h"
#include "./ast_node.h"
#include <node.h>
#include <v8.h>

using namespace v8;

Persistent<Function> ASTNodeArray::constructor;

void ASTNodeArray::Init(Handle<Object> exports) {
  // Constructor
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("ASTNodeArray"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Properties
  tpl->InstanceTemplate()->SetIndexedPropertyHandler(
      IndexedPropertyGetter(GetIndex),
      IndexedPropertySetter(0));
  tpl->InstanceTemplate()->SetAccessor(
      String::NewSymbol("length"),
      AccessorGetter(Length),
      AccessorSetter(0));

  constructor = Persistent<Function>::New(tpl->GetFunction());
}

ASTNodeArray::ASTNodeArray(TSNode *node) : parent_node_(node) {
  ts_node_retain(node);
}

ASTNodeArray::~ASTNodeArray() {
  ts_node_release(parent_node_);
}

Handle<Value> ASTNodeArray::NewInstance(TSNode *node) {
  HandleScope scope;
  Local<Object> instance = constructor->NewInstance(0, NULL);
  ASTNodeArray *array = new ASTNodeArray(node);
  array->Wrap(instance);
  return scope.Close(instance);
}

Handle<Value> ASTNodeArray::New(const Arguments& args) {
  HandleScope scope;
  return scope.Close(Undefined());
}

Handle<Value> ASTNodeArray::GetIndex(size_t index, const AccessorInfo &info) {
  Local<Object> self = info.This();
  ASTNodeArray *array = ObjectWrap::Unwrap<ASTNodeArray>(self);
  TSNode *child = ts_node_child(array->parent_node_, index);
  if (child)
    return ASTNode::NewInstance(child);
  else
    return Undefined();
}

Handle<Value> ASTNodeArray::Length(Local<String>, const AccessorInfo &info) {
  HandleScope scope;
  Local<Object> self = info.This();
  ASTNodeArray *array = ObjectWrap::Unwrap<ASTNodeArray>(self);
  return scope.Close(Integer::New(ts_node_child_count(array->parent_node_)));
}
