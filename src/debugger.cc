#include "./debugger.h"
#include "./input_reader.h"
#include <string>
#include <v8.h>
#include <nan.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

using namespace v8;
using std::string;

static void Debug(void *payload, TSDebugType type, const char *message_str) {
  Debugger *debugger = (Debugger *)payload;
  Handle<Function> fn = NanNew(debugger->func);
  if (!fn->IsFunction())
    return;

  string message(message_str);
  string param_sep = " ";
  size_t param_sep_pos = message.find(param_sep, 0);

  Local<String> type_name = NanNew((type == TSDebugTypeParse) ? "parse" : "lex");
  Local<String> name = NanNew(message.substr(0, param_sep_pos));
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
  TryCatch try_catch;
  fn->Call(fn->CreationContext()->Global(), 3, argv);
  if (try_catch.HasCaught()) {
    Handle<Value> log_argv[2] = {
      NanNew("Error in debug callback:"),
      try_catch.Exception()
    };

    Handle<Object> console = Handle<Object>::Cast(fn->CreationContext()->Global()->Get(NanNew("console")));
    Handle<Function> error_fn = Handle<Function>::Cast(console->Get(NanNew("error")));
    error_fn->Call(console, 2, log_argv);
  }
}

TSDebugger DebuggerMake(Handle<Function> func) {
  TSDebugger result;
  Debugger *debugger = new Debugger();
  NanAssignPersistent(debugger->func, func);
  result.payload = (void *)debugger;
  result.debug_fn = Debug;
  return result;
}

}  // namespace node_tree_sitter
