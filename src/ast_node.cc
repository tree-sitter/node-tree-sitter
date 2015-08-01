#include "./ast_node.h"
#include <nan.h>
#include <tree_sitter/runtime.h>
#include <v8.h>

namespace node_tree_sitter {

using namespace v8;

Persistent<Function> ASTNode::constructor;

void ASTNode::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
  tpl->SetClassName(NanNew("ASTNode"));
  IASTNode::SetUp(tpl);
  NanAssignPersistent(constructor, tpl->GetFunction());
}

ASTNode::ASTNode(TSNode node, TSDocument *document) :
  IASTNode(document), node_(node) {}

TSNode ASTNode::node() {
  return node_;
}

Handle<Value> ASTNode::NewInstance(TSNode node, TSDocument *document) {
  Local<Object> instance = NanNew(constructor)->NewInstance(0, NULL);
  (new ASTNode(node, document))->Wrap(instance);
  return instance;
}

NAN_METHOD(ASTNode::New) {
  NanScope();
  NanReturnUndefined();
}

}  // namespace node_tree_sitter
