#include "./ast_node.h"
#include "./ast_node_array.h"
#include <node.h>
#include <v8.h>

namespace node_tree_sitter {

using namespace v8;

Persistent<Function> ASTNode::constructor;

void ASTNode::Init(Handle<Object> exports) {
  // Constructor
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("ASTNode"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Properties
  tpl->InstanceTemplate()->SetAccessor(
      String::NewSymbol("children"),
      AccessorGetter(Children));

  // Prototype
  tpl->PrototypeTemplate()->Set(
      String::NewSymbol("position"),
      FunctionTemplate::New(Position)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewSymbol("size"),
      FunctionTemplate::New(Size)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewSymbol("name"),
      FunctionTemplate::New(Name)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewSymbol("toString"),
      FunctionTemplate::New(ToString)->GetFunction());

  constructor = Persistent<Function>::New(tpl->GetFunction());
}

ASTNode::ASTNode(TSNode *node) : node_(node) {
  ts_node_retain(node_);
}

ASTNode::~ASTNode() {
  ts_node_release(node_);
}

Handle<Value> ASTNode::NewInstance(TSNode *node) {
  HandleScope scope;
  Local<Object> instance = constructor->NewInstance(0, NULL);
  ASTNode *ast_node = new ASTNode(node);
  ast_node->Wrap(instance);
  return scope.Close(instance);
}

Handle<Value> ASTNode::New(const Arguments& args) {
  HandleScope scope;
  return scope.Close(Undefined());
}

Handle<Value> ASTNode::Name(const Arguments& args) {
  HandleScope scope;
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(args.This());
  const char *result = ts_node_name(node->node_);
  return scope.Close(String::New(result));
}

Handle<Value> ASTNode::Size(const Arguments& args) {
  HandleScope scope;
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(args.This());
  size_t result = ts_node_size(node->node_);
  return scope.Close(Integer::New(result));
}

Handle<Value> ASTNode::Position(const Arguments& args) {
  HandleScope scope;
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(args.This());
  size_t result = ts_node_pos(node->node_);
  return scope.Close(Integer::New(result));
}

Handle<Value> ASTNode::ToString(const Arguments& args) {
  HandleScope scope;
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(args.This());
  const char *result = ts_node_string(node->node_);
  return scope.Close(String::Concat(
      String::New("ASTNode: "),
      String::New(result)));
}

Handle<Value> ASTNode::Children(Local<String> name, const AccessorInfo &info) {
  HandleScope scope;
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(info.This());
  return scope.Close(ASTNodeArray::NewInstance(node->node_));
}

}  // namespace node_tree_sitter
