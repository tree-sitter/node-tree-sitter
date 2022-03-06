#include "./query.h"
#include <string>
#include <vector>
#include <napi.h>
#include "./node.h"
#include "./language.h"
#include "./logger.h"
#include "./util.h"
#include "./conversions.h"
#include "tree_sitter/api.h"

namespace node_tree_sitter {

using std::vector;
using namespace Napi;

const char *query_error_names[] = {
  "TSQueryErrorNone",
  "TSQueryErrorSyntax",
  "TSQueryErrorNodeType",
  "TSQueryErrorField",
  "TSQueryErrorCapture",
  "TSQueryErrorStructure",
};

TSQueryCursor *Query::ts_query_cursor;
Napi::FunctionReference Query::constructor;

void Query::Init(Napi::Object &exports) {
  ts_query_cursor = ts_query_cursor_new();
  Napi::Env env = exports.Env();
  Function ctor = DefineClass(env, "Query", {
    InstanceMethod("_matches", &Query::Matches),
    InstanceMethod("_captures", &Query::Captures),
    InstanceMethod("_getPredicates", &Query::GetPredicates),
  });

	constructor.Reset(ctor, 1);
  constructor.SuppressDestruct(); // statics should not destruct
  exports["Query"] = ctor;
}

Query::~Query() {
  ts_query_delete(query_);
}

Query *Query::UnwrapQuery(const Napi::Value &value) {
  return Query::Unwrap(value.As<Napi::Object>());
}

Query::Query(const Napi::CallbackInfo& info)
  : Napi::ObjectWrap<Query>(info)
  , query_(nullptr) {

  const TSLanguage *language = UnwrapLanguage(info[0]);
  const char *source;
  uint32_t source_len;
  uint32_t error_offset = 0;
  TSQueryError error_type = TSQueryErrorNone;

  if (language == nullptr) {
    Napi::TypeError::New(info.Env(), "Missing language argument").ThrowAsJavaScriptException();
    return;
  }

  if (info[1].IsString()) {
    auto&& utf8_string = info[1].As<Napi::String>().Utf8Value();
    source = utf8_string.data();
    source_len = utf8_string.size();
    query_ = ts_query_new(language, source, source_len, &error_offset, &error_type);
  }
  else if (info[1].IsBuffer()) {
    auto&& buffer = info[1].As<Napi::Buffer<char>>();
    source = buffer.Data();
    source_len = buffer.Length();
    query_ = ts_query_new(language, source, source_len, &error_offset, &error_type);
  }
  else {
    Napi::TypeError::New(info.Env(), "Missing source argument").ThrowAsJavaScriptException();
    return;
  }

  if (error_offset > 0) {
    const char *error_name = query_error_names[error_type];
    std::string message = "Query error of type ";
    message += error_name;
    message += " at position ";
    message += std::to_string(error_offset);
    Napi::TypeError::New(info.Env(),message.c_str()).ThrowAsJavaScriptException();
    return;
  }

	info.This().As<Napi::Object>().Get("_init").As<Napi::Function>().Call(info.This(), {});
}

Napi::Value Query::GetPredicates(const Napi::CallbackInfo &info) {
  auto pattern_len = ts_query_pattern_count(query_);

  Napi::Array js_predicates = Napi::Array::New(info.Env());

  for (size_t pattern_index = 0; pattern_index < pattern_len; pattern_index++) {
    uint32_t predicates_len;
    const TSQueryPredicateStep *predicates = ts_query_predicates_for_pattern(
        query_, pattern_index, &predicates_len);

    Napi::Array js_pattern_predicates = Napi::Array::New(info.Env());

    if (predicates_len > 0) {
      Napi::Array js_predicate = Napi::Array::New(info.Env());

      size_t a_index = 0;
      size_t p_index = 0;
      for (size_t i = 0; i < predicates_len; i++) {
        const TSQueryPredicateStep predicate = predicates[i];
        uint32_t len;
        switch (predicate.type) {
          case TSQueryPredicateStepTypeCapture:
          	js_predicate[p_index++] = Napi::Number::New(info.Env(), TSQueryPredicateStepTypeCapture);
          	js_predicate[p_index++] = Napi::String::New(info.Env(), ts_query_capture_name_for_id(query_, predicate.value_id, &len));
            break;
          case TSQueryPredicateStepTypeString:
          	js_predicate[p_index++] = Napi::Number::New(info.Env(), TSQueryPredicateStepTypeString);
          	js_predicate[p_index++] = Napi::String::New(info.Env(), ts_query_string_value_for_id(query_, predicate.value_id, &len));
            break;
          case TSQueryPredicateStepTypeDone:
          	js_pattern_predicates[a_index++] = js_predicate;
            js_predicate = Napi::Array::New(info.Env());
            p_index = 0;
            break;
        }
      }
    }

    js_predicates[pattern_index] = js_pattern_predicates;
  }

  return js_predicates;
}

Napi::Value Query::Matches(const Napi::CallbackInfo &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  uint32_t start_row    = info[1].As<Napi::Number>().Uint32Value();
  uint32_t start_column = info[2].As<Napi::Number>().Uint32Value() << 1;
  uint32_t end_row      = info[3].As<Napi::Number>().Uint32Value();
  uint32_t end_column   = info[4].As<Napi::Number>().Uint32Value() << 1;

  if (tree == nullptr) {
    Napi::TypeError::New(info.Env(), "Missing argument tree").ThrowAsJavaScriptException();
    return Env().Null();
  }

  TSNode rootNode = UnmarshalNode(info.Env(), tree);
  TSPoint start_point = {start_row, start_column};
  TSPoint end_point = {end_row, end_column};
  ts_query_cursor_set_point_range(ts_query_cursor, start_point, end_point);
  ts_query_cursor_exec(ts_query_cursor, query_, rootNode);

  Napi::Array js_matches  = Napi::Array::New(info.Env());
  unsigned index = 0;
  vector<TSNode> nodes;
  TSQueryMatch match;

  while (ts_query_cursor_next_match(ts_query_cursor, &match)) {
    js_matches[index++] = Napi::Number::New(info.Env(), match.pattern_index);

    for (uint16_t i = 0; i < match.capture_count; i++) {
      const TSQueryCapture &capture = match.captures[i];

      uint32_t capture_name_len = 0;
      const char *capture_name = ts_query_capture_name_for_id(
          query_, capture.index, &capture_name_len);

      TSNode node = capture.node;
      nodes.push_back(node);

      Napi::Value js_capture = Napi::String::New(info.Env(), capture_name);
      js_matches[index++] = js_capture;
    }
  }

  auto js_nodes = GetMarshalNodes(info.Env(), tree, nodes.data(), nodes.size());

  auto result = Napi::Array::New(info.Env());
  result[0u] = js_matches;
  result[1u] = js_nodes;
  return result;
}

Napi::Value Query::Captures(const Napi::CallbackInfo &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  uint32_t start_row    = info[1].As<Napi::Number>().Uint32Value();
  uint32_t start_column = info[2].As<Napi::Number>().Uint32Value() << 1;
  uint32_t end_row      = info[3].As<Napi::Number>().Uint32Value();
  uint32_t end_column   = info[4].As<Napi::Number>().Uint32Value() << 1;

  if (tree == nullptr) {
    Napi::TypeError::New(info.Env(), "Missing argument tree").ThrowAsJavaScriptException();
    return info.Env().Null();
  }

  TSNode rootNode = UnmarshalNode(info.Env(), tree);
  TSPoint start_point = {start_row, start_column};
  TSPoint end_point = {end_row, end_column};
  ts_query_cursor_set_point_range(ts_query_cursor, start_point, end_point);
  ts_query_cursor_exec(ts_query_cursor, query_, rootNode);

  Napi::Array js_matches  = Napi::Array::New(info.Env());
  unsigned index = 0;
  vector<TSNode> nodes;
  TSQueryMatch match;
  uint32_t capture_index;

  while (ts_query_cursor_next_capture(
    ts_query_cursor,
    &match,
    &capture_index
  )) {

    js_matches[index++] = Napi::Number::New(info.Env(), match.pattern_index);
    js_matches[index++] = Napi::Number::New(info.Env(), capture_index);

    for (uint16_t i = 0; i < match.capture_count; i++) {
      const TSQueryCapture &capture = match.captures[i];

      uint32_t capture_name_len = 0;
      const char *capture_name = ts_query_capture_name_for_id(
          query_, capture.index, &capture_name_len);

      TSNode node = capture.node;
      nodes.push_back(node);

      Napi::String js_capture = Napi::String::New(info.Env(), capture_name);
      js_matches[index++] = js_capture;
    }
  }

  auto js_nodes = GetMarshalNodes(info.Env(), tree, nodes.data(), nodes.size());

  auto result = Napi::Array::New(info.Env());
  result[0u] = js_matches;
  result[1u] = js_nodes;
  return result;
}


}  // namespace node_tree_sitter
