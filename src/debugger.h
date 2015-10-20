#ifndef NODE_TREE_SITTER_DEBUGGER_H_
#define NODE_TREE_SITTER_DEBUGGER_H_

#include <v8.h>
#include <nan.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

class Debugger {
 public:
  static TSDebugger Make(v8::Local<v8::Function>);

 private:
  static void Debug(void *, TSDebugType, const char *);

  Nan::Persistent<v8::Function> func;
};


}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_DEBUGGER_H_
