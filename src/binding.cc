#include <node.h>
#include <v8.h>
#include "tree_sitter/compiler.h"

using namespace v8;

Handle<Value> Method(const Arguments& args) {
  HandleScope scope;
  return scope.Close(String::New("world"));
}

void init(Handle<Object> exports) {
  exports->Set(
    String::NewSymbol("hello"),
    FunctionTemplate::New(Method)->GetFunction());
}

NODE_MODULE(binding, init)
