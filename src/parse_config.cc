#include "./runtime.h"
#include "./helpers.h"
#include <uv.h>

using namespace v8;
using std::string;

Persistent<Function> ParseConfig::constructor;

ParseConfig::ParseConfig(ts_parse_config value) : value_(value) {}

ts_parse_config ParseConfig::value() const {
  return value_;
}

void ParseConfig::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("ParseConfig"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  constructor = Persistent<Function>::New(tpl->GetFunction());
}

Handle<Value> ParseConfig::New(const Arguments &args) {
  HandleScope scope;

  Handle<String> js_filename = Handle<String>::Cast(args[0]);
  Handle<String> js_parser_name = Handle<String>::Cast(args[1]);
  string filename = string_from_js_string(js_filename);
  string parser_name = string_from_js_string(js_parser_name);

  uv_lib_t parser_lib;
  int error_code = uv_dlopen(filename.c_str(), &parser_lib);
  if (error_code) {
    Handle<String> message = String::New(uv_dlerror(&parser_lib));
    ThrowException(Exception::Error(
      String::Concat(String::New("Error opening parser file - "), message)));
  }

  ts_parse_config *config;
  error_code = uv_dlsym(&parser_lib, ("ts_parse_config_" + parser_name).c_str(), (void **)&config);
  if (error_code) {
    Handle<String> message = String::New(uv_dlerror(&parser_lib));
    ThrowException(Exception::Error(
      String::Concat(String::New("Error loading parser from parser file - "), message)));
  }

  ParseConfig *parse_config = new ParseConfig(*config);
  parse_config->Wrap(args.This());
  return args.This();
}

Handle<Value> ParseConfig::NewInstance(const Arguments &args) {
  HandleScope scope;
  const unsigned argc = 2;
  Handle<Value> argv[argc] = { args[0], args[1] };
  Local<Object> instance = constructor->NewInstance(argc, argv);
  return scope.Close(instance);
}

Handle<Value> LoadParserLib(const Arguments &args) {
  HandleScope scope;
  return scope.Close(ParseConfig::NewInstance(args));
}
