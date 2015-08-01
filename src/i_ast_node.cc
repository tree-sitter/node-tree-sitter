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

static inline IASTNode *unwrap(const Local<Object> &object) {
  IASTNode *node = node::ObjectWrap::Unwrap<IASTNode>(object->ToObject());
  if (node && node->node().data)
    return node;
  else
    return NULL;
}

IASTNode::IASTNode(TSDocument *document) : document_(document) {}

NAN_METHOD(IASTNode::ToString) {
  NanScope();
  IASTNode *node = unwrap(args.This());
  if (node) {
    NanReturnValue(NanNew(ts_node_string(node->node(), node->document_)));
  }
  NanReturnValue(NanNew("(NULL)"));
}

NAN_METHOD(IASTNode::Parent) {
  NanScope();
  IASTNode *node = unwrap(args.This());
  if (node) {
    TSNode parent = ts_node_parent(node->node());
    if (parent.data)
      NanReturnValue(ASTNode::NewInstance(parent, node->document_));
  }
  NanReturnNull();
}

NAN_METHOD(IASTNode::Next) {
  NanScope();
  IASTNode *node = unwrap(args.This());
  if (node) {
    TSNode sibling = ts_node_next_sibling(node->node());
    if (sibling.data)
      NanReturnValue(ASTNode::NewInstance(sibling, node->document_));
  }
  NanReturnNull();
}

NAN_METHOD(IASTNode::Prev) {
  NanScope();
  IASTNode *node = unwrap(args.This());
  if (node) {
    TSNode sibling = ts_node_prev_sibling(node->node());
    if (sibling.data)
      NanReturnValue(ASTNode::NewInstance(sibling, node->document_));
  }
  NanReturnNull();
}

NAN_METHOD(IASTNode::NodeAt) {
  NanScope();
  IASTNode *node = unwrap(args.This());
  if (node) {
    switch (args.Length()) {
      case 2: {
        Handle<Integer> min = Handle<Integer>::Cast(args[0]);
        Handle<Integer> max = Handle<Integer>::Cast(args[1]);
        TSNode result = ts_node_find_for_range(node->node(), min->Int32Value(), max->Int32Value());
        NanReturnValue(ASTNode::NewInstance(result, node->document_));
      }
      case 1: {
        Handle<Integer> pos = Handle<Integer>::Cast(args[0]);
        TSNode result = ts_node_find_for_pos(node->node(), pos->Int32Value());
        NanReturnValue(ASTNode::NewInstance(result, node->document_));
      }
      default:
        NanThrowTypeError("Must provide 1 or 2 numeric arguments");
    }
  }
  NanReturnNull();
}

NAN_GETTER(IASTNode::Name) {
  NanScope();
  IASTNode *node = unwrap(args.This());
  if (node)
    NanReturnValue(NanNew(ts_node_name(node->node(), node->document_)));
  NanReturnNull();
}

NAN_GETTER(IASTNode::Size) {
  NanScope();
  IASTNode *node = unwrap(args.This());
  if (node)
    NanReturnValue(NanNew<Number>(ts_node_size(node->node()).chars));
  NanReturnNull();
}

NAN_GETTER(IASTNode::Position) {
  NanScope();
  IASTNode *node = unwrap(args.This());
  if (node)
    NanReturnValue(NanNew<Number>(ts_node_pos(node->node()).chars));
  NanReturnNull();
}

NAN_GETTER(IASTNode::Children) {
  NanScope();
  IASTNode *node = unwrap(args.This());
  if (node)
    NanReturnValue(ASTNodeArray::NewInstance(node->node(), node->document_));
  NanReturnNull();
}

}  // namespace node_tree_sitter
