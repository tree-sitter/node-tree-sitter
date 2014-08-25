#include "./ast_node.h"
#include <nan.h>
#include <tree_sitter/runtime.h>
#include <v8.h>

namespace node_tree_sitter {

using namespace v8;

Persistent<Function> ASTNode::constructor;

void ASTNode::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
  tpl->SetClassName(NanNew<String>("ASTNode"));
  IASTNode::SetUp(tpl);
  NanAssignPersistent(constructor, tpl->GetFunction());
}

ASTNode::ASTNode(TSNode *node) : node_(node) {
  ts_node_retain(node_);
}

ASTNode::~ASTNode() {
  ts_node_release(node_);
}

TSNode * ASTNode::node() {
  return node_;
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

}  // namespace node_tree_sitter
