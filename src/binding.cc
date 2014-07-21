#include "./compile.h"
#include "./parser.h"
#include "./document.h"
#include "./ast_node.h"
#include "./ast_node_array.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
  Document::Init(exports);
  Parser::Init(exports);
  ASTNode::Init(exports);
  ASTNodeArray::Init(exports);

  exports->Set(
      String::NewSymbol("compile"),
      FunctionTemplate::New(Compile)->GetFunction());
  exports->Set(
      String::NewSymbol("loadParser"),
      FunctionTemplate::New(Parser::Load)->GetFunction());
}

NODE_MODULE(tree_sitter_binding, InitAll)
