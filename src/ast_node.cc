#include "./ast_node.h"
#include "./ast_node_array.h"
#include <node.h>
#include <v8.h>

namespace node_tree_sitter {

using namespace v8;

Persistent<Function> ASTNode::constructor;

void ASTNode::Init(Handle<Object> exports) {
  // Constructor
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
  tpl->SetClassName(NanNew<String>("ASTNode"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Properties
  tpl->InstanceTemplate()->SetAccessor(
      NanNew<String>("children"),
      Children);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew<String>("position"),
      Position);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew<String>("size"),
      Size);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew<String>("name"),
      Name);

  // Prototype
  tpl->PrototypeTemplate()->Set(
      NanNew<String>("toString"),
      NanNew<FunctionTemplate>(ToString)->GetFunction());

  NanAssignPersistent(constructor, tpl->GetFunction());
}

ASTNode::ASTNode(TSNode *node) : node_(node) {
  ts_node_retain(node_);
}

ASTNode::~ASTNode() {
  ts_node_release(node_);
}

Handle<Value> ASTNode::NewInstance(TSNode *node) {
  Local<Object> instance = NanNew(constructor)->NewInstance(0, NULL);
  (new ASTNode(node))->Wrap(instance);
  return instance;
}

NAN_METHOD(ASTNode::New) {
  NanScope();
  NanReturnUndefined();
}

NAN_METHOD(ASTNode::ToString) {
  NanScope();
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(args.This()->ToObject());
  NanReturnValue(NanNew<String>(ts_node_string(node->node_)));
}

NAN_GETTER(ASTNode::Name) {
  NanScope();
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(args.This()->ToObject());
  NanReturnValue(NanNew<String>(ts_node_name(node->node_)));
}

NAN_GETTER(ASTNode::Size) {
  NanScope();
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(args.This()->ToObject());
  NanReturnValue(NanNew<Integer>(ts_node_size(node->node_)));
}

NAN_GETTER(ASTNode::Position) {
  NanScope();
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(args.This()->ToObject());
  NanReturnValue(NanNew<Integer>(ts_node_pos(node->node_)));
}

NAN_GETTER(ASTNode::Children) {
  NanScope();
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(args.This()->ToObject());
  NanReturnValue(ASTNodeArray::NewInstance(node->node_));
}

}  // namespace node_tree_sitter
