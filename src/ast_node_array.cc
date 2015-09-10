#include "./ast_node_array.h"
#include <nan.h>
#include <tree_sitter/runtime.h>
#include <v8.h>
#include <vector>
#include <string>
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

  // Array methods
  Handle<Array> array = NanNew<Array>();
  const char * array_methods[] = {
    "every",
    "filter",
    "forEach",
    "indexOf",
    "map",
    "reduce",
    "reduceRight",
    "some",
  };

  for (size_t i = 0; i < (sizeof(array_methods) / sizeof(array_methods[0])); i++)
    tpl->PrototypeTemplate()->Set(
        NanNew(array_methods[i]),
        array->Get(NanNew(array_methods[i])));

  NanAssignPersistent(constructor, tpl->GetFunction());
}

ASTNodeArray::ASTNodeArray(TSNode node, TSDocument *document, size_t parse_count, bool is_named) :
  parent_node_(node), document_(document), parse_count_(parse_count), is_named_(is_named)  {}

Handle<Value> ASTNodeArray::NewInstance(TSNode node, TSDocument *document, size_t parse_count, bool is_named) {
  Local<Object> instance = NanNew(constructor)->NewInstance(0, NULL);
  (new ASTNodeArray(node, document, parse_count, is_named))->Wrap(instance);
  return instance;
}

NAN_METHOD(ASTNodeArray::New) {
  NanScope();
  NanReturnUndefined();
}

NAN_INDEX_GETTER(ASTNodeArray::GetIndex) {
  NanScope();
  ASTNodeArray *array = ObjectWrap::Unwrap<ASTNodeArray>(args.This()->ToObject());
  TSNode child = array->is_named_ ?
    ts_node_named_child(array->parent_node_, index) :
    ts_node_child(array->parent_node_, index);
  if (child.data)
    NanReturnValue(ASTNode::NewInstance(child, array->document_, array->parse_count_));
  else
    NanReturnUndefined();
}

NAN_GETTER(ASTNodeArray::Length) {
  NanScope();
  ASTNodeArray *array = ObjectWrap::Unwrap<ASTNodeArray>(args.This()->ToObject());
  size_t length = array->is_named_ ?
    ts_node_named_child_count(array->parent_node_) :
    ts_node_child_count(array->parent_node_);
  NanReturnValue(NanNew<Number>(length));
}

}  // namespace node_tree_sitter
