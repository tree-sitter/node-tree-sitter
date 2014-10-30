#include "./input_reader.h"
#include <string>
#include <v8.h>
#include <nan.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

using namespace v8;
using std::string;

struct Debugger {
  Persistent<Function> func;
};

static void Release(void *data) {
  delete (Debugger *)data;
}

static void Debug(void *data, TSDebugType type, const char *message_str) {
  Debugger *debugger = (Debugger *)data;
  Handle<Function> fn = NanNew(debugger->func);
  if (!fn->IsFunction())
    return;

  string message(message_str);
  string param_sep = " ";
  size_t param_sep_pos = message.find(param_sep, 0);

  Local<String> type_name = NanNew((type == TSDebugTypeParse) ? "parse" : "lex");
  Local<String> name = NanNew<String>(message.substr(0, param_sep_pos));
  Local<Object> params = NanNew<Object>();


  while (param_sep_pos != string::npos) {
    size_t key_pos = param_sep_pos + param_sep.size();
    size_t value_sep_pos = message.find(":", key_pos);

    if (value_sep_pos == string::npos)
      break;

    size_t val_pos = value_sep_pos + 1;
    param_sep = ", ";
    param_sep_pos = message.find(param_sep, value_sep_pos);

    string key = message.substr(key_pos, (value_sep_pos - key_pos));
    string value = message.substr(val_pos, (param_sep_pos - val_pos));
    params->Set(NanNew(key), NanNew(value));
  }

  Handle<Value> argv[3] = { name, params, type_name };
  fn->Call(fn->CreationContext()->Global(), 3, argv);
}

TSDebugger DebuggerMake(Handle<Function> func) {
  TSDebugger result;
  Debugger *debugger = new Debugger();
  NanAssignPersistent(debugger->func, func);
  result.data = (void *)debugger;
  result.debug_fn = Debug;
  result.release_fn = Release;
  return result;
}

}  // namespace node_tree_sitter
