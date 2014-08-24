#include <v8.h>
#include <node.h>
#include "./ast_node_array.h"
#include "./ast_node.h"

namespace node_tree_sitter {

using namespace v8;

Persistent<Function> ASTNodeArray::constructor;

void ASTNodeArray::Init(Handle<Object> exports) {
  // Constructor
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
  tpl->SetClassName(NanNew("ASTNodeArray"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Properties
  tpl->InstanceTemplate()->SetIndexedPropertyHandler(
      GetIndex,
      NULL);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("length"),
      Length,
      NULL);

  NanAssignPersistent(constructor, tpl->GetFunction());
}

ASTNodeArray::ASTNodeArray(TSNode *node) : parent_node_(node) {
  ts_node_retain(node);
}

ASTNodeArray::~ASTNodeArray() {
  ts_node_release(parent_node_);
}

Handle<Value> ASTNodeArray::NewInstance(TSNode *node) {
  Local<Object> instance = NanNew(constructor)->NewInstance(0, NULL);
  (new ASTNodeArray(node))->Wrap(instance);
  return instance;
}

NAN_METHOD(ASTNodeArray::New) {
  NanScope();
  NanReturnUndefined();
}

NAN_INDEX_GETTER(ASTNodeArray::GetIndex) {
  NanScope();
  ASTNodeArray *array = ObjectWrap::Unwrap<ASTNodeArray>(args.This());
  TSNode *child = ts_node_child(array->parent_node_, index);
  if (child)
    NanReturnValue(ASTNode::NewInstance(child));
  else
    NanReturnUndefined();
}

NAN_GETTER(ASTNodeArray::Length) {
  NanScope();
  ASTNodeArray *array = ObjectWrap::Unwrap<ASTNodeArray>(args.This());
  NanReturnValue(NanNew<Integer>(ts_node_child_count(array->parent_node_)));
}

}  // namespace node_tree_sitter
