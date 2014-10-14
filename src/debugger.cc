#include "./input_reader.h"
#include <v8.h>
#include <nan.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

using namespace v8;

struct Debugger {
  Persistent<Function> func;
};

static void Release(void *data) {
  delete (Debugger *)data;
}

static void Debug(void *data, const char *message) {
  Debugger *debugger = (Debugger *)data;
  Handle<Function> fn = NanNew(debugger->func);
  if (!fn->IsFunction())
    return;

  Handle<Value> argv[1] = { NanNew(message) };
  fn->Call(NanUndefined(), 1, argv);
}

TSDebugger DebuggerMake(Local<Function> func) {
  TSDebugger result;
  Debugger *debugger = new Debugger();
  NanAssignPersistent(debugger->func, func);
  result.data = (void *)debugger;
  result.debug_fn = Debug;
  result.release_fn = Release;
  return result;
}

}  // namespace node_tree_sitter

