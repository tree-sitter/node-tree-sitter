#include <node.h>
#include "./compile.h"
#include "./runtime.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
  Document::Init(exports);
  ParseConfig::Init(exports);
  exports->Set(
    String::NewSymbol("compile"),
    FunctionTemplate::New(Compile)->GetFunction());
  exports->Set(
    String::NewSymbol("loadParserLib"),
    FunctionTemplate::New(LoadParserLib)->GetFunction());
}

NODE_MODULE(tree_sitter_binding, InitAll)
