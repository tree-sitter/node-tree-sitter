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
  tpl->PrototypeTemplate()->Set(
      NanNew("parent"),
      NanNew<FunctionTemplate>(Parent)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      NanNew("next"),
      NanNew<FunctionTemplate>(Next)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      NanNew("prev"),
      NanNew<FunctionTemplate>(Prev)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      NanNew("nodeAt"),
      NanNew<FunctionTemplate>(NodeAt)->GetFunction());

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

NAN_METHOD(ASTNode::Parent) {
  NanScope();
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(args.This()->ToObject());
  TSNode *parent = ts_node_parent(node->node_);
  if (parent)
    NanReturnValue(NewInstance(parent));
  else
    NanReturnNull();
}

NAN_METHOD(ASTNode::Next) {
  NanScope();
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(args.This()->ToObject());
  TSNode *sibling = ts_node_next_sibling(node->node_);
  if (sibling)
    NanReturnValue(NewInstance(sibling));
  else
    NanReturnNull();
}

NAN_METHOD(ASTNode::Prev) {
  NanScope();
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(args.This()->ToObject());
  TSNode *sibling = ts_node_prev_sibling(node->node_);
  if (sibling)
    NanReturnValue(NewInstance(sibling));
  else
    NanReturnNull();
}

NAN_METHOD(ASTNode::NodeAt) {
  NanScope();
  ASTNode *astNode = ObjectWrap::Unwrap<ASTNode>(args.This()->ToObject());
  TSNode *node = astNode->node_;
  switch (args.Length()) {
    case 2: {
      Handle<Integer> min = Handle<Integer>::Cast(args[0]);
      Handle<Integer> max = Handle<Integer>::Cast(args[1]);
      TSNode *result = ts_node_find_for_range(node, min->Int32Value(), max->Int32Value());
      NanReturnValue(ASTNode::NewInstance(result));
    }
    case 1: {
      Handle<Integer> pos = Handle<Integer>::Cast(args[0]);
      TSNode *result = ts_node_find_for_pos(node, pos->Int32Value());
      NanReturnValue(ASTNode::NewInstance(result));
    }
    default:
      NanThrowTypeError("Must provide 1 or 2 numeric arguments");
      NanReturnUndefined();
  }
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
