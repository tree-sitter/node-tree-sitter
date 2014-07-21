#include "./parser.h"
#include <uv.h>
#include <string>

using namespace v8;

Persistent<Function> Parser::constructor;

void Parser::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Parser"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  constructor = Persistent<Function>::New(tpl->GetFunction());
}

Parser::Parser(TSParser *value) : value_(value) {}

TSParser * Parser::value() const {
  return value_;
}

Handle<Value> Parser::Load(const Arguments &args) {
  HandleScope scope;

  Handle<String> js_filename = Handle<String>::Cast(args[0]);
  Handle<String> js_parser_name = Handle<String>::Cast(args[1]);
  String::Utf8Value filename(js_filename);
  String::Utf8Value parser_name(js_parser_name);

  uv_lib_t parser_lib;
  int error_code = uv_dlopen(*filename, &parser_lib);
  if (error_code) {
    Handle<String> message = String::New(uv_dlerror(&parser_lib));
    ThrowException(Exception::Error(
      String::Concat(String::New("Error opening parser file - "), message)));
  }

  TSParser * (* parser_constructor)();
  error_code = uv_dlsym(&parser_lib, (std::string("ts_parser_") + *parser_name).c_str(), (void **)&parser_constructor);
  if (error_code) {
    Handle<String> message = String::New(uv_dlerror(&parser_lib));
    ThrowException(Exception::Error(
      String::Concat(String::New("Error loading parser from parser file - "), message)));
  }

  Parser *parser = new Parser(parser_constructor());
  Local<Object> instance = constructor->NewInstance(0, NULL);
  parser->Wrap(instance);
  return scope.Close(instance);
}

Handle<Value> Parser::New(const Arguments &args) {
  HandleScope scope;
  return scope.Close(Undefined());
}
