#include "./query.h"
#include <string>
#include <v8.h>
#include <nan.h>
#include "./node.h"
#include "./language.h"
#include "./logger.h"
#include "./util.h"
#include "./conversions.h"

namespace node_tree_sitter {

using namespace v8;
using node_methods::UnmarshalNodeId;

Nan::Persistent<Function> Query::constructor;
Nan::Persistent<FunctionTemplate> Query::constructor_template;

void Query::Init(Local<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Local<String> class_name = Nan::New("Query").ToLocalChecked();
  tpl->SetClassName(class_name);

  Local<Function> ctor = Nan::GetFunction(tpl).ToLocalChecked();

  constructor_template.Reset(tpl);
  constructor.Reset(ctor);
  Nan::Set(exports, class_name, ctor);
}

Query::Query(TSQuery *query) : query_(query) {}

Query::~Query() {
  ts_query_delete(query_);
}

Local<Value> Query::NewInstance(TSQuery *query) {
  if (query) {
    Local<Object> self;
    MaybeLocal<Object> maybe_self = Nan::NewInstance(Nan::New(constructor));
    if (maybe_self.ToLocal(&self)) {
      (new Query(query))->Wrap(self);
      return self;
    }
  }
  return Nan::Null();
}

const Query *Query::UnwrapQuery(const Local<Value> &value) {
  if (!value->IsObject()) return nullptr;
  Local<Object> js_query = Local<Object>::Cast(value);
  if (!Nan::New(constructor_template)->HasInstance(js_query)) return nullptr;
  return ObjectWrap::Unwrap<Query>(js_query);
}

void Query::New(const Nan::FunctionCallbackInfo<Value> &info) {
  if (!info.IsConstructCall()) {
    Local<Object> self;
    MaybeLocal<Object> maybe_self = Nan::New(constructor)->NewInstance(Nan::GetCurrentContext());
    if (maybe_self.ToLocal(&self)) {
      info.GetReturnValue().Set(self);
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
    return;
  }

  const TSLanguage *language = language_methods::UnwrapLanguage(info[0]);
  const char *source;
  uint32_t source_len;
  uint32_t error_offset = 0;
  TSQueryError error_type = TSQueryErrorNone;

  if (language == nullptr) {
    Nan::ThrowError("Missing language argument");
    return;
  }

  if (info[1]->IsString()) {
    auto string = Nan::To<String> (info[1]).ToLocalChecked();
    source = *Nan::Utf8String(string);
    source_len = string->Length();
  }
  else if (node::Buffer::HasInstance(info[1])) {
    source = node::Buffer::Data(info[1]);
    source_len = node::Buffer::Length(info[1]);
  }
  else {
    Nan::ThrowError("Missing source argument");
    return;
  }

  TSQuery *query = ts_query_new(
      language, source, source_len, &error_offset, &error_type);

  if (error_offset > 0) {
    std::string message =
      "Query error of type " +
      std::to_string(error_type) +
      " at offset " +
      std::to_string(error_offset);
    Nan::ThrowError(message.c_str());
    return;
  }

  Query *query_wrapper = new Query(query);
  query_wrapper->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

}  // namespace node_tree_sitter
