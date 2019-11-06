#include "./language.h"
#include <napi.h>
#include <tree_sitter/api.h>
#include <vector>
#include <string>
#include "./util.h"

namespace node_tree_sitter {

using std::vector;
using namespace Napi;


const TSLanguage *UnwrapLanguage(const Napi::Value &value) {
  Env env = value.Env();

  const TSLanguage *language = static_cast<const TSLanguage *>(
    GetInternalFieldPointer(value)
  );

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
      RangeError::New(env, message.c_str()).ThrowAsJavaScriptException();
      return nullptr;
    }
    return language;
  }

  TypeError::New(env, "Invalid language object").ThrowAsJavaScriptException();
  return nullptr;
}

static Value GetNodeTypeNamesById(const CallbackInfo &info) {
  Env env = info.Env();

  const TSLanguage *language = UnwrapLanguage(info[0]);
  if (!language) return env.Null();

  Array result = Array::New(env);
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

static Value GetNodeFieldNamesById(const CallbackInfo &info) {
  Env env = info.Env();

  const TSLanguage *language = UnwrapLanguage(info[0]);
  if (!language) return env.Null();

  Array result = Array::New(env);
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

void InitLanguage(Object &exports) {
  Env env = exports.Env();
  exports["getNodeTypeNamesById"] = Function::New(env, GetNodeTypeNamesById);
  exports["getNodeFieldNamesById"] = Function::New(env, GetNodeFieldNamesById);
}

}  // namespace node_tree_sitter
