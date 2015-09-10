#include "./ast_node.h"
#include <nan.h>
#include <tree_sitter/runtime.h>
#include <v8.h>
#include "./ast_node_array.h"

namespace node_tree_sitter {

using namespace v8;

Persistent<Function> ASTNode::constructor;

ASTNode *ASTNode::Unwrap(const Local<Object> &object) {
  ASTNode *node = node::ObjectWrap::Unwrap<ASTNode>(object->ToObject());
  if (node && node->node_.data)
    return node;
  else
    return NULL;
}

ASTNode *ASTNode::UnwrapValid(const Local<Object> &object) {
  ASTNode *node = Unwrap(object);
  if (node && node->parse_count_ == ts_document_parse_count(node->document_))
    return node;
  else
    return NULL;
}

void ASTNode::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
  tpl->SetClassName(NanNew("ASTNode"));

  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Properties
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("position"),
      Position);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("size"),
      Size);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("name"),
      Name);

  tpl->InstanceTemplate()->SetAccessor(
      NanNew("children"),
      Children,
      0, Handle<Value>(), DEFAULT, DontEnum);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("namedChildren"),
      NamedChildren,
      0, Handle<Value>(), DEFAULT, DontEnum);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("parent"),
      Parent,
      0, Handle<Value>(), DEFAULT, DontEnum);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("nextSibling"),
      NextSibling,
      0, Handle<Value>(), DEFAULT, DontEnum);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("nextNamedSibling"),
      NextNamedSibling,
      0, Handle<Value>(), DEFAULT, DontEnum);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("previousSibling"),
      PreviousSibling,
      0, Handle<Value>(), DEFAULT, DontEnum);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("previousNamedSibling"),
      PreviousNamedSibling,
      0, Handle<Value>(), DEFAULT, DontEnum);

  // Prototype
  tpl->PrototypeTemplate()->Set(
      NanNew("toString"),
      NanNew<FunctionTemplate>(ToString)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      NanNew("descendantForRange"),
      NanNew<FunctionTemplate>(DescendantForRange)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      NanNew("namedDescendantForRange"),
      NanNew<FunctionTemplate>(NamedDescendantForRange)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      NanNew("isValid"),
      NanNew<FunctionTemplate>(IsValid)->GetFunction());

  NanAssignPersistent(constructor, tpl->GetFunction());
}

ASTNode::ASTNode(TSNode node, TSDocument *document, size_t parse_count) :
  node_(node), document_(document), parse_count_(parse_count) {}

Handle<Value> ASTNode::NewInstance(TSNode node, TSDocument *document, size_t parse_count) {
  Local<Object> instance = NanNew(constructor)->NewInstance(0, NULL);
  (new ASTNode(node, document, parse_count))->Wrap(instance);
  return instance;
}

NAN_METHOD(ASTNode::New) {
  NanScope();
  NanReturnUndefined();
}

NAN_METHOD(ASTNode::ToString) {
  NanScope();
  ASTNode *node = UnwrapValid(args.This());
  if (node)
    NanReturnValue(NanNew(ts_node_string(node->node_, node->document_)));
  NanReturnNull();
}

NAN_METHOD(ASTNode::IsValid) {
  NanScope();
  ASTNode *node = Unwrap(args.This());
  if (node)
    NanReturnValue(NanNew<Boolean>(node->parse_count_ == ts_document_parse_count(node->document_)));
  NanReturnNull();
}

NAN_METHOD(ASTNode::NamedDescendantForRange) {
  NanScope();
  ASTNode *node = UnwrapValid(args.This());
  if (node) {
    switch (args.Length()) {
      case 2: {
        Handle<Integer> min = Handle<Integer>::Cast(args[0]);
        Handle<Integer> max = Handle<Integer>::Cast(args[1]);
        TSNode result = ts_node_named_descendent_for_range(node->node_, min->Int32Value(), max->Int32Value());
        NanReturnValue(ASTNode::NewInstance(result, node->document_, node->parse_count_));
      }
      case 1: {
        Handle<Integer> pos = Handle<Integer>::Cast(args[0]);
        TSNode result = ts_node_named_descendent_for_range(node->node_, pos->Int32Value(), pos->Int32Value());
        NanReturnValue(ASTNode::NewInstance(result, node->document_, node->parse_count_));
      }
      default:
        NanThrowTypeError("Must provide 1 or 2 numeric arguments");
    }
  }
  NanReturnNull();
}

NAN_METHOD(ASTNode::DescendantForRange) {
  NanScope();
  ASTNode *node = UnwrapValid(args.This());
  if (node) {
    switch (args.Length()) {
      case 2: {
        Handle<Integer> min = Handle<Integer>::Cast(args[0]);
        Handle<Integer> max = Handle<Integer>::Cast(args[1]);
        TSNode result = ts_node_descendent_for_range(node->node_, min->Int32Value(), max->Int32Value());
        NanReturnValue(ASTNode::NewInstance(result, node->document_, node->parse_count_));
      }
      case 1: {
        Handle<Integer> pos = Handle<Integer>::Cast(args[0]);
        TSNode result = ts_node_descendent_for_range(node->node_, pos->Int32Value(), pos->Int32Value());
        NanReturnValue(ASTNode::NewInstance(result, node->document_, node->parse_count_));
      }
      default:
        NanThrowTypeError("Must provide 1 or 2 numeric arguments");
    }
  }
  NanReturnNull();
}

NAN_GETTER(ASTNode::Name) {
  NanScope();
  ASTNode *node = UnwrapValid(args.This());
  if (node)
    NanReturnValue(NanNew(ts_node_name(node->node_, node->document_)));
  NanReturnNull();
}

NAN_GETTER(ASTNode::Size) {
  NanScope();
  ASTNode *node = UnwrapValid(args.This());
  if (node)
    NanReturnValue(NanNew<Number>(ts_node_size(node->node_).chars));
  NanReturnNull();
}

NAN_GETTER(ASTNode::Position) {
  NanScope();
  ASTNode *node = UnwrapValid(args.This());
  if (node)
    NanReturnValue(NanNew<Number>(ts_node_pos(node->node_).chars));
  NanReturnNull();
}

NAN_GETTER(ASTNode::Children) {
  NanScope();
  ASTNode *node = UnwrapValid(args.This());
  if (node)
    NanReturnValue(ASTNodeArray::NewInstance(node->node_, node->document_, node->parse_count_, false));
  NanReturnNull();
}

NAN_GETTER(ASTNode::NamedChildren) {
  NanScope();
  ASTNode *node = UnwrapValid(args.This());
  if (node)
    NanReturnValue(ASTNodeArray::NewInstance(node->node_, node->document_, node->parse_count_, true));
  NanReturnNull();
}

NAN_GETTER(ASTNode::Parent) {
  NanScope();
  ASTNode *node = UnwrapValid(args.This());
  if (node) {
    TSNode parent = ts_node_parent(node->node_);
    if (parent.data)
      NanReturnValue(ASTNode::NewInstance(parent, node->document_, node->parse_count_));
  }
  NanReturnNull();
}

NAN_GETTER(ASTNode::NextSibling) {
  NanScope();
  ASTNode *node = UnwrapValid(args.This());
  if (node) {
    TSNode sibling = ts_node_next_sibling(node->node_);
    if (sibling.data)
      NanReturnValue(ASTNode::NewInstance(sibling, node->document_, node->parse_count_));
  }
  NanReturnNull();
}

NAN_GETTER(ASTNode::NextNamedSibling) {
  NanScope();
  ASTNode *node = UnwrapValid(args.This());
  if (node) {
    TSNode sibling = ts_node_next_named_sibling(node->node_);
    if (sibling.data)
      NanReturnValue(ASTNode::NewInstance(sibling, node->document_, node->parse_count_));
  }
  NanReturnNull();
}

NAN_GETTER(ASTNode::PreviousSibling) {
  NanScope();
  ASTNode *node = UnwrapValid(args.This());
  if (node) {
    TSNode sibling = ts_node_prev_sibling(node->node_);
    if (sibling.data)
      NanReturnValue(ASTNode::NewInstance(sibling, node->document_, node->parse_count_));
  }
  NanReturnNull();
}

NAN_GETTER(ASTNode::PreviousNamedSibling) {
  NanScope();
  ASTNode *node = UnwrapValid(args.This());
  if (node) {
    TSNode sibling = ts_node_prev_named_sibling(node->node_);
    if (sibling.data)
      NanReturnValue(ASTNode::NewInstance(sibling, node->document_, node->parse_count_));
  }
  NanReturnNull();
}

}  // namespace node_tree_sitter
