#include "./binding.h"
#include <node.h>

using namespace v8;

void InitAll(Handle<Object> exports) {
  Document::Init(exports);
  Parser::Init(exports);
  exports->Set(
    String::NewSymbol("compile"),
    FunctionTemplate::New(Compile)->GetFunction());
  exports->Set(
    String::NewSymbol("loadParserLib"),
    FunctionTemplate::New(Parser::NewInstance)->GetFunction());
}

NODE_MODULE(tree_sitter_binding, InitAll)
