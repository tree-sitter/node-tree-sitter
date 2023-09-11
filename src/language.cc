#include "./language.h"
#include "tree_sitter/api.h"

#include <nan.h>
#include <string>
#include <v8.h>
#include <vector>

namespace node_tree_sitter::language_methods {

using namespace v8;

const TSLanguage *UnwrapLanguage(const v8::Local<v8::Value> &value) {
  if (value->IsObject()) {
    Local<Object> arg = Local<Object>::Cast(value);
    if (arg->InternalFieldCount() == 1) {
      const auto *language = static_cast<const TSLanguage *>(Nan::GetInternalFieldPointer(arg, 0));
      if (language != nullptr) {
        uint16_t version = ts_language_version(language);
        if (
          version < TREE_SITTER_MIN_COMPATIBLE_LANGUAGE_VERSION ||
          version > TREE_SITTER_LANGUAGE_VERSION
        ) {
          std::string message =
            "Incompatible language version. Compatible range: " +
            std::to_string(TREE_SITTER_MIN_COMPATIBLE_LANGUAGE_VERSION) + " - " +
            std::to_string(TREE_SITTER_LANGUAGE_VERSION) + ". Got: " +
            std::to_string(ts_language_version(language));
          Nan::ThrowError(Nan::RangeError(message.c_str()));
          return nullptr;
        }
        return language;
      }
    }
  }
  Nan::ThrowTypeError("Invalid language object");
  return nullptr;
}

namespace {
void GetNodeTypeNamesById(const Nan::FunctionCallbackInfo<Value> &info) {
  const TSLanguage *language = UnwrapLanguage(info[0]);
  if (language == nullptr) {
    return;
  }

  auto result = Nan::New<Array>();
  uint32_t length = ts_language_symbol_count(language);
  for (uint32_t i = 0; i < length; i++) {
    const char *name = ts_language_symbol_name(language, i);
    TSSymbolType type = ts_language_symbol_type(language, i);
    if (type == TSSymbolTypeRegular) {
      Nan::Set(result, i, Nan::New(name).ToLocalChecked());
    } else {
      Nan::Set(result, i, Nan::Null());
    }
  }

  info.GetReturnValue().Set(result);
}

void GetNodeFieldNamesById(const Nan::FunctionCallbackInfo<Value> &info) {
  const TSLanguage *language = UnwrapLanguage(info[0]);
  if (language == nullptr) {
    return;
  }

  auto result = Nan::New<Array>();
  uint32_t length = ts_language_field_count(language);
  for (uint32_t i = 0; i < length + 1; i++) {
    const char *name = ts_language_field_name_for_id(language, i);
    if (name != nullptr) {
      Nan::Set(result, i, Nan::New(name).ToLocalChecked());
    } else {
      Nan::Set(result, i, Nan::Null());
    }
  }
  info.GetReturnValue().Set(result);
}
} // namespace

void Init(Local<Object> exports) {
  Nan::Set(
    exports,
    Nan::New("getNodeTypeNamesById").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(GetNodeTypeNamesById)).ToLocalChecked()
  );

  Nan::Set(
    exports,
    Nan::New("getNodeFieldNamesById").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(GetNodeFieldNamesById)).ToLocalChecked()
  );
}

} // namespace node_tree_sitter::language_methods
