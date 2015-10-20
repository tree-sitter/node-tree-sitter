#include "./debugger.h"
#include "./input_reader.h"
#include <string>
#include <v8.h>
#include <nan.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

using namespace v8;
using std::string;

void Debugger::Debug(void *payload, TSDebugType type, const char *message_str) {
  Debugger *debugger = (Debugger *)payload;
  Handle<Function> fn = Nan::New(debugger->func);
  if (!fn->IsFunction())
    return;

  string message(message_str);
  string param_sep = " ";
  size_t param_sep_pos = message.find(param_sep, 0);

  Local<String> type_name = Nan::New((type == TSDebugTypeParse) ? "parse" : "lex").ToLocalChecked();
  Local<String> name = Nan::New(message.substr(0, param_sep_pos)).ToLocalChecked();
  Local<Object> params = Nan::New<Object>();

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
    params->Set(Nan::New(key).ToLocalChecked(), Nan::New(value).ToLocalChecked());
  }

  Handle<Value> argv[3] = { name, params, type_name };
  TryCatch try_catch;
  fn->Call(fn->CreationContext()->Global(), 3, argv);
  if (try_catch.HasCaught()) {
    Handle<Value> log_argv[2] = {
      Nan::New("Error in debug callback:").ToLocalChecked(),
      try_catch.Exception()
    };

    Handle<Object> console = Handle<Object>::Cast(fn->CreationContext()->Global()->Get(Nan::New("console").ToLocalChecked()));
    Handle<Function> error_fn = Handle<Function>::Cast(console->Get(Nan::New("error").ToLocalChecked()));
    error_fn->Call(console, 2, log_argv);
  }
}

TSDebugger Debugger::Make(Handle<Function> func) {
  TSDebugger result;
  Debugger *debugger = new Debugger();
  debugger->func.Reset(Nan::Persistent<Function>(func));
  result.payload = (void *)debugger;
  result.debug_fn = Debug;
  return result;
}

}  // namespace node_tree_sitter
