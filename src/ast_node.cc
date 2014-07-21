#include "./ast_node.h"
#include "./ast_node_array.h"
#include <node.h>
#include <v8.h>

using namespace v8;

Persistent<Function> ASTNode::constructor;

void ASTNode::Init(Handle<Object> exports) {
  // Constructor
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("ASTNode"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

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
  exports->Set(String::NewSymbol("ASTNode"), constructor);
}

ASTNode::ASTNode(TSNode *node) : value_(node) {
    ts_node_retain(node);
}

ASTNode::~ASTNode() {
    ts_node_release(value_);
}

Handle<Value> ASTNode::New(const Arguments& args) {
  HandleScope scope;
  if (args.IsConstructCall()) {
    return args.This();
  } else {
    const int argc = 1;
    Local<Value> argv[argc] = { args[0] };
    return scope.Close(constructor->NewInstance(argc, argv));
  }
}

Handle<Value> ASTNode::NewInstance(TSNode *node) {
  HandleScope scope;
  Local<Object> instance = constructor->NewInstance(0, NULL);
  ASTNode *ast_node = new ASTNode(node);
  ast_node->Wrap(instance);
  return scope.Close(instance);
}

Handle<Value> ASTNode::Name(const Arguments& args) {
  HandleScope scope;
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(args.This());
  const char *result = ts_node_name(node->value_);
  return scope.Close(String::New(result));
}

Handle<Value> ASTNode::Size(const Arguments& args) {
  HandleScope scope;
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(args.This());
  size_t result = ts_node_size(node->value_);
  return scope.Close(Integer::New(result));
}

Handle<Value> ASTNode::Position(const Arguments& args) {
  HandleScope scope;
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(args.This());
  size_t result = ts_node_pos(node->value_);
  return scope.Close(Integer::New(result));
}

Handle<Value> ASTNode::ToString(const Arguments& args) {
  HandleScope scope;
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(args.This());
  const char *result = ts_node_string(node->value_);
  return scope.Close(String::Concat(
    String::New("ASTNode: "),
    String::New(result)
  ));
}

Handle<Value> ASTNode::Children(Local<String> name, const AccessorInfo &info) {
  HandleScope scope;
  ASTNode *node = ObjectWrap::Unwrap<ASTNode>(info.This());
  return scope.Close(ASTNodeArray::NewInstance(node->value_));
}
