#include "./parser.h"
#include <string>
#include <v8.h>
#include <nan.h>
#include "./conversions.h"
#include "./input_reader.h"
#include "./logger.h"
#include "./tree.h"
#include "./util.h"

namespace node_tree_sitter {

using namespace v8;

Nan::Persistent<Function> Parser::constructor;

void Parser::Init(Local<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Local<String> class_name = Nan::New("Parser").ToLocalChecked();
  tpl->SetClassName(class_name);

  FunctionPair methods[] = {
    {"getLogger", GetLogger},
    {"setLogger", SetLogger},
    {"setLanguage", SetLanguage},
    {"printDotGraphs", PrintDotGraphs},
    {"parse", Parse},
  };

  for (size_t i = 0; i < length_of_array(methods); i++) {
    Nan::SetPrototypeMethod(tpl, methods[i].name, methods[i].callback);
  }

  constructor.Reset(Nan::Persistent<Function>(tpl->GetFunction()));
  exports->Set(class_name, Nan::New(constructor));
  exports->Set(Nan::New("LANGUAGE_VERSION").ToLocalChecked(), Nan::New<Number>(TREE_SITTER_LANGUAGE_VERSION));
}

Parser::Parser() : parser_(ts_parser_new()) {}

Parser::~Parser() { ts_parser_delete(parser_); }

void Parser::New(const Nan::FunctionCallbackInfo<Value> &info) {
  if (info.IsConstructCall()) {
    Parser *parser = new Parser();
    parser->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    Local<Object> self;
    MaybeLocal<Object> maybe_self = Nan::New(constructor)->NewInstance(Nan::GetCurrentContext());
    if (maybe_self.ToLocal(&self)) {
      info.GetReturnValue().Set(self);
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
}

void Parser::SetLanguage(const Nan::FunctionCallbackInfo<Value> &info) {
  Local<Object> arg = Local<Object>::Cast(info[0]);

  Parser *parser = ObjectWrap::Unwrap<Parser>(info.This());
  if (arg->InternalFieldCount() != 1) {
    Nan::ThrowTypeError("Invalid language object");
    return;
  }

  TSLanguage *language = (TSLanguage *)Nan::GetInternalFieldPointer(arg, 0);
  if (!language) {
    Nan::ThrowTypeError("Invalid language object");
    return;
  }

  if (ts_language_version(language) != TREE_SITTER_LANGUAGE_VERSION) {
    std::string message = "Incompatible language version. Expected " +
      std::to_string(TREE_SITTER_LANGUAGE_VERSION) + ". Got " +
      std::to_string(ts_language_version(language));
    Nan::ThrowError(Nan::RangeError(message.c_str()));
    return;
  }

  ts_parser_set_language(parser->parser_, language);

  info.GetReturnValue().Set(info.This());
}

void Parser::Parse(const Nan::FunctionCallbackInfo<Value> &info) {
  Parser *parser = ObjectWrap::Unwrap<Parser>(info.This());

  if (!info[0]->IsObject()) {
    Nan::ThrowTypeError("Input must be an object");
    return;
  }

  Local<Object> input = Local<Object>::Cast(info[0]);

  if (!input->Get(Nan::New("seek").ToLocalChecked())->IsFunction()) {
    Nan::ThrowTypeError("Input must implement seek(n)");
    return;
  }

  if (!input->Get(Nan::New("read").ToLocalChecked())->IsFunction()) {
    Nan::ThrowTypeError("Input must implement read(n)");
    return;
  }

  InputReader input_reader(input);
  TSTree *tree = ts_parser_parse(parser->parser_, nullptr, input_reader.Input());

  Local<Value> result = Tree::NewInstance(tree);
  info.GetReturnValue().Set(result);
}

void Parser::GetLogger(const Nan::FunctionCallbackInfo<Value> &info) {
  Parser *parser = ObjectWrap::Unwrap<Parser>(info.This());

  TSLogger current_logger = ts_parser_logger(parser->parser_);
  if (current_logger.payload && current_logger.log == Logger::Log) {
    Logger *logger = (Logger *)current_logger.payload;
    info.GetReturnValue().Set(Nan::New(logger->func));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

void Parser::SetLogger(const Nan::FunctionCallbackInfo<Value> &info) {
  Parser *parser = ObjectWrap::Unwrap<Parser>(info.This());
  Local<Function> func = Local<Function>::Cast(info[0]);

  TSLogger current_logger = ts_parser_logger(parser->parser_);
  if (current_logger.payload)
    delete (Logger *)current_logger.payload;

  if (func->IsFunction()) {
    ts_parser_set_logger(parser->parser_, Logger::Make(func));
  } else {
    ts_parser_set_logger(parser->parser_, { 0, 0 });
    if (!(func->IsNull() || func->IsFalse() || func->IsUndefined())) {
      Nan::ThrowTypeError("Debug callback must either be a function or a falsy value");
      return;
    }
  }

  info.GetReturnValue().Set(info.This());
}

void Parser::PrintDotGraphs(const Nan::FunctionCallbackInfo<Value> &info) {
  Parser *parser = ObjectWrap::Unwrap<Parser>(info.This());
  Local<Boolean> value = Local<Boolean>::Cast(info[0]);

  if (value->IsBoolean() && value->BooleanValue()) {
    ts_parser_print_dot_graphs(parser->parser_, stderr);
  } else {
    ts_parser_print_dot_graphs(parser->parser_, nullptr);
  }

  info.GetReturnValue().Set(info.This());
}

}  // namespace node_tree_sitter
