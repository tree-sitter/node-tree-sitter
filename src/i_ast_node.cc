#include "./i_ast_node.h"
#include <v8.h>
#include <node_object_wrap.h>
#include <tree_sitter/runtime.h>
#include "./ast_node.h"
#include "./ast_node_array.h"

namespace node_tree_sitter {

using namespace v8;

void IASTNode::SetUp(Local<FunctionTemplate> tpl) {
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Properties
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("children"),
      Children);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("position"),
      Position);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("size"),
      Size);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("name"),
      Name);

  // Prototype
  tpl->PrototypeTemplate()->Set(
      NanNew("toString"),
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
}

static inline TSNode * unwrap(const Local<Object> &object) {
  return node::ObjectWrap::Unwrap<IASTNode>(object->ToObject())->node();
}

NAN_METHOD(IASTNode::ToString) {
  NanScope();
  TSNode *node = unwrap(args.This());
  if (node) {
    NanReturnValue(NanNew(ts_node_string(node)));
  }
  NanReturnValue(NanNew("(NULL)"));
}

NAN_METHOD(IASTNode::Parent) {
  NanScope();
  TSNode *node = unwrap(args.This());
  if (node) {
    TSNode *parent = ts_node_parent(node);
    if (parent)
      NanReturnValue(ASTNode::NewInstance(parent));
  }
  NanReturnNull();
}

NAN_METHOD(IASTNode::Next) {
  NanScope();
  TSNode *node = unwrap(args.This());
  if (node) {
    TSNode *sibling = ts_node_next_sibling(node);
    if (sibling)
      NanReturnValue(ASTNode::NewInstance(sibling));
  }
  NanReturnNull();
}

NAN_METHOD(IASTNode::Prev) {
  NanScope();
  TSNode *node = unwrap(args.This());
  if (node) {
    TSNode *sibling = ts_node_prev_sibling(node);
    if (sibling)
      NanReturnValue(ASTNode::NewInstance(sibling));
  }
  NanReturnNull();
}

NAN_METHOD(IASTNode::NodeAt) {
  NanScope();
  TSNode *node = unwrap(args.This());
  if (node) {
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
    }
  }
  NanReturnNull();
}

NAN_GETTER(IASTNode::Name) {
  NanScope();
  TSNode *node = unwrap(args.This());
  if (node)
    NanReturnValue(NanNew(ts_node_name(node)));
  NanReturnNull();
}

NAN_GETTER(IASTNode::Size) {
  NanScope();
  TSNode *node = unwrap(args.This());
  if (node)
    NanReturnValue(NanNew<Number>(ts_node_size(node).chars));
  NanReturnNull();
}

NAN_GETTER(IASTNode::Position) {
  NanScope();
  TSNode *node = unwrap(args.This());
  if (node)
    NanReturnValue(NanNew<Number>(ts_node_pos(node).chars));
  NanReturnNull();
}

NAN_GETTER(IASTNode::Children) {
  NanScope();
  TSNode *node = unwrap(args.This());
  if (node)
    NanReturnValue(ASTNodeArray::NewInstance(node));
  NanReturnNull();
}

}  // namespace node_tree_sitter
