#include "./lookaheaditerator.h"
#include "./language.h"
#include "./util.h"

#include <nan.h>
#include <tree_sitter/api.h>
#include <v8.h>

namespace node_tree_sitter {

using namespace v8;

Nan::Persistent<Function> LookaheadIterator::constructor;
Nan::Persistent<FunctionTemplate> LookaheadIterator::constructor_template;

void LookaheadIterator::Init(Local<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Local<String> class_name = Nan::New("LookaheadIterator").ToLocalChecked();
  tpl->SetClassName(class_name);

  GetterPair getters[] = {
    {"currentSymbol", CurrentSymbol},
    {"currentSymbolName", CurrentSymbolName},
  };

  FunctionPair methods[] = {
    {"reset", Reset},
    {"resetState", ResetState},
    {"next", Next},
    {"iterNames", IterNames},
  };

  for (auto &getter : getters) {
    Nan::SetAccessor(
      tpl->InstanceTemplate(),
      Nan::New(getter.name).ToLocalChecked(),
      getter.callback
    );
  }

  for (auto &method : methods) {
    Nan::SetPrototypeMethod(tpl, method.name, method.callback);
  }

  Local<Function> ctor = Nan::GetFunction(tpl).ToLocalChecked();

  constructor_template.Reset(tpl);
  constructor.Reset(ctor);
  Nan::Set(exports, class_name, ctor);
}

LookaheadIterator::LookaheadIterator(TSLookaheadIterator *iterator) : iterator_(iterator) {}

LookaheadIterator::~LookaheadIterator() {
  ts_lookahead_iterator_delete(iterator_);
}

Local<Value> LookaheadIterator::NewInstance(TSLookaheadIterator *iterator) {
  if (iterator != nullptr) {
    Local<Object> self;
    MaybeLocal<Object> maybe_self = Nan::NewInstance(Nan::New(constructor));
    if (maybe_self.ToLocal(&self)) {
      (new LookaheadIterator(iterator))->Wrap(self);
      return self;
    }
  }
  return Nan::Null();
}

LookaheadIterator *LookaheadIterator::UnwrapLookaheadIterator(const v8::Local<v8::Value> &value) {
  if (!value->IsObject()) {
    return nullptr;
  }
  Local<Object> js_iterator = Local<Object>::Cast(value);
  if (!Nan::New(constructor_template)->HasInstance(js_iterator)) {
    return nullptr;
  }
  return ObjectWrap::Unwrap<LookaheadIterator>(js_iterator);
}

void LookaheadIterator::New(const Nan::FunctionCallbackInfo<v8::Value> &info) {
  if (!info.IsConstructCall()) {
    Local<Object> self;
    MaybeLocal<Object> maybe_self = Nan::NewInstance(Nan::New(constructor));
    if (maybe_self.ToLocal(&self)) {
      info.GetReturnValue().Set(self);
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
    return;
  }

  const TSLanguage *language = language_methods::UnwrapLanguage(info[0]);

  if (language == nullptr) {
    Nan::ThrowError("Missing language argument");
    return;
  }

  if (!info[1]->IsNumber()) {
    Nan::ThrowError("Missing state argument");
    return;
  }
  TSStateId state = Nan::To<uint32_t>(info[1]).ToChecked();

  TSLookaheadIterator *iterator = ts_lookahead_iterator_new(language, state);
  if (iterator == nullptr) {
    Nan::ThrowError("Invalid state argument");
    return;
  }
  auto self = info.This();
  auto *lookahead_iterator_wrapper = new LookaheadIterator(ts_lookahead_iterator_new(language, state));
  lookahead_iterator_wrapper->Wrap(self);

  info.GetReturnValue().Set(self);
}

void LookaheadIterator::CurrentSymbolName(Local<String> /*prop*/, const Nan::PropertyCallbackInfo<Value> &info) {
  LookaheadIterator *iterator = UnwrapLookaheadIterator(info.This());
  const char *name = ts_lookahead_iterator_current_symbol_name(iterator->iterator_);
  info.GetReturnValue().Set(Nan::New(name).ToLocalChecked());
}

void LookaheadIterator::CurrentSymbol(Local<String>  /*prop*/, const Nan::PropertyCallbackInfo<Value> &info) {
  LookaheadIterator *iterator = UnwrapLookaheadIterator(info.This());
  TSSymbol symbol = ts_lookahead_iterator_current_symbol(iterator->iterator_);
  info.GetReturnValue().Set(Nan::New(symbol));
}

void LookaheadIterator::Reset(const Nan::FunctionCallbackInfo<Value> &info) {
  LookaheadIterator *iterator = UnwrapLookaheadIterator(info.This());
  const TSLanguage *language = language_methods::UnwrapLanguage(info[0]);
  if (language == nullptr) {
    Nan::ThrowError("Invalid language argument");
    return;
  }
  if (!info[1]->IsNumber()) {
    Nan::ThrowError("Missing state argument");
    return;
  }
  TSStateId state = Nan::To<uint32_t>(info[1]).ToChecked();
  info.GetReturnValue().Set(Nan::New(ts_lookahead_iterator_reset(iterator->iterator_, language, state)));
}

void LookaheadIterator::ResetState(const Nan::FunctionCallbackInfo<Value> &info) {
  LookaheadIterator *iterator = UnwrapLookaheadIterator(info.This());
  if (!info[0]->IsNumber()) {
    Nan::ThrowError("Missing state argument");
    return;
  }
  TSStateId state = Nan::To<uint32_t>(info[0]).ToChecked();
  info.GetReturnValue().Set(Nan::New(ts_lookahead_iterator_reset_state(iterator->iterator_, state)));
}

void LookaheadIterator::Next(const Nan::FunctionCallbackInfo<Value> &info) {
  LookaheadIterator *iterator = UnwrapLookaheadIterator(info.This());
  bool result = ts_lookahead_iterator_next(iterator->iterator_);
  if (!result) {
    info.GetReturnValue().Set(Nan::Null());
    return;
  }
  info.GetReturnValue().Set(Nan::New(ts_lookahead_iterator_current_symbol(iterator->iterator_)));
}

void LookaheadIterator::IterNames(const Nan::FunctionCallbackInfo<Value> &info) {
  LookaheadIterator *iterator = UnwrapLookaheadIterator(info.This());
  auto result = Nan::New<Array>();
  while (ts_lookahead_iterator_next(iterator->iterator_)) {
    const char *name = ts_lookahead_iterator_current_symbol_name(iterator->iterator_);
    Nan::Set(result, result->Length(), Nan::New(name).ToLocalChecked());
  }
  info.GetReturnValue().Set(result);
}

} // namespace node_tree_sitter
