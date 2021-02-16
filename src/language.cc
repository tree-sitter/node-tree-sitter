#include "./language.h"
#include <napi.h>
#include <tree_sitter/api.h>
#include <vector>
#include <string>

namespace node_tree_sitter {

using std::vector;

const TSLanguage *UnwrapLanguage(Napi::Env env, const Napi::Value &value) {
  if (value.IsObject()) {
    Napi::Object arg = value.As<Napi::Object>();
    Napi::Value name = arg.Get("name");
    if (!env.IsExceptionPending()) {
      if (name.IsString()) {
        Napi::Value parser = arg.Get("__parser");
        if (!env.IsExceptionPending()) {
          if (parser.IsExternal()) {
            Napi::External<TSLanguage> parserInfo = parser.As<Napi::External<TSLanguage> >();
            const TSLanguage *language = (const TSLanguage *) parserInfo.Data();
            if (language) {
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
                Napi::RangeError::New(env, message.c_str()).ThrowAsJavaScriptException();
                return nullptr;
              }
              return language;
            }
          }
        }
      }
    }
  }
  Napi::TypeError::New(env, "Invalid language object").ThrowAsJavaScriptException();

  return nullptr;
}

Napi::Value GetNodeTypeNamesById(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  const TSLanguage *language = UnwrapLanguage(env, info[0]);
  if (!language) return env.Undefined();

  Napi::Array result = Napi::Array::New(env);
  uint32_t length = ts_language_symbol_count(language);
  for (uint32_t i = 0; i < length; i++) {
    const char *name = ts_language_symbol_name(language, i);
    TSSymbolType type = ts_language_symbol_type(language, i);
    if (type == TSSymbolTypeRegular) {
      (result).Set(i, Napi::String::New(env, name));
    } else {
      (result).Set(i, env.Null());
    }
  }

  return result;
}

Napi::Value GetNodeFieldNamesById(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  const TSLanguage *language = UnwrapLanguage(env, info[0]);
  if (!language) return env.Undefined();

  Napi::Array result = Napi::Array::New(env);
  uint32_t length = ts_language_field_count(language);
  for (uint32_t i = 0; i < length + 1; i++) {
    const char *name = ts_language_field_name_for_id(language, i);
    if (name) {
      (result).Set(i, Napi::String::New(env, name));
    } else {
      (result).Set(i, env.Null());
    }
  }
  return result;
}

void InitLanguage(Napi::Object &exports) {
  Napi::Env env = exports.Env();
  exports.Set(Napi::String::New(env, "getNodeTypeNamesById"), Napi::Function::New<GetNodeTypeNamesById>(env));
  exports.Set(Napi::String::New(env, "getNodeFieldNamesById"), Napi::Function::New<GetNodeFieldNamesById>(env));
}

}  // namespace node_tree_sitter
