#include "./language.h"
#include <napi.h>
#include <tree_sitter/api.h>
#include <vector>
#include <string>

using namespace Napi;
using std::vector;

namespace node_tree_sitter {
namespace language_methods {

const TSLanguage *UnwrapLanguage(Napi::Value value) {
  Napi::Env env = value.Env();

  if (value.IsObject()) {
    value = value.As<Object>()["language"];
  }

  if (value.IsExternal()) {
    External arg = value.As<External<const TSLanguage>>();
    const TSLanguage *language = arg.Data();
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
        RangeError::New(env, message.c_str());
        return nullptr;
      }
      return language;
    }
  }
  throw TypeError::New(env, "Invalid language object");
}

static Napi::Value GetNodeTypeNamesById(const Napi::CallbackInfo &info) {
  Env env = info.Env();

  const TSLanguage *language = UnwrapLanguage(info[0]);
  if (!language) return env.Undefined();

  auto result = Array::New(env);
  uint32_t length = ts_language_symbol_count(language);
  for (uint32_t i = 0; i < length; i++) {
    const char *name = ts_language_symbol_name(language, i);
    TSSymbolType type = ts_language_symbol_type(language, i);
    if (type == TSSymbolTypeRegular) {
      result[i] = String::New(env, name);
    } else {
      result[i] = env.Null();
    }
  }

  return result;
}

static Napi::Value GetNodeFieldNamesById(const Napi::CallbackInfo &info) {
  Env env = info.Env();

  const TSLanguage *language = UnwrapLanguage(info[0]);
  if (!language) return env.Undefined();

  auto result = Array::New(env);
  uint32_t length = ts_language_field_count(language);
  for (uint32_t i = 0; i < length + 1; i++) {
    const char *name = ts_language_field_name_for_id(language, i);
    if (name) {
      result[i] = String::New(env, name);
    } else {
      result[i] = env.Null();
    }
  }

  return result;
}

void Init(Napi::Env env, Napi::Object exports) {
  exports["getNodeTypeNamesById"] = Function::New(env, GetNodeTypeNamesById);
  exports["getNodeFieldNamesById"] = Function::New(env, GetNodeFieldNamesById);
}

}  // namespace language_methods
}  // namespace node_tree_sitter
