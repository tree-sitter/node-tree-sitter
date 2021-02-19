#include "./query.h"
#include <string>
#include <vector>
#include <v8.h>
#include <string.h>
#include <napi.h>
#include <uv.h>
#include "./binding.h"
#include "./node.h"
#include "./language.h"
#include "./logger.h"
#include "./util.h"
#include "./tree.h"
#include "./conversions.h"

namespace node_tree_sitter {

using std::vector;

const char *query_error_names[] = {
  "TSQueryErrorNone",
  "TSQueryErrorSyntax",
  "TSQueryErrorNodeType",
  "TSQueryErrorField",
  "TSQueryErrorCapture",
};

void Query::Init(Napi::Object exports, InstanceData *instance) {
  Napi::Env env = exports.Env();

  Napi::Function ctor = DefineClass(env, "Query", {
    InstanceMethod("_matches", &Query::Matches),
    InstanceMethod("_captures", &Query::Captures),
    InstanceMethod("_getPredicates", &Query::GetPredicates),
  });

  instance->query_constructor = new Napi::FunctionReference();
  (*instance->query_constructor) = Napi::Persistent(ctor);
  exports["Query"] = ctor;
}

Query::Query(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Query>(info) {
  query_ = nullptr;
  ts_query_cursor = nullptr;
  Napi::Env env = info.Env();

  if (info.Length() != 2) {
    Napi::TypeError::New(env, "Missing language and source arguments").ThrowAsJavaScriptException();
    return;
  }
  const char *source;
  uint32_t source_len;
  uint32_t error_offset = 0;
  TSQueryError error_type = TSQueryErrorNone;
  const TSLanguage *language = UnwrapLanguage(env, info[0]);

  if (env.IsExceptionPending()) {
    return;
  }
  if (language == nullptr) {
    Napi::TypeError::New(env, "language argument is not a tree-sitter language").ThrowAsJavaScriptException();
    return;
  }

  if (info[1].IsString()) {
    Napi::String string = info[1].As<Napi::String>();
    std::string utf8 = string.Utf8Value();
    source = utf8.c_str();
    printf("before \"%s\"\n", source);
    source_len = strlen(source);
  }
  else if (info[1].IsBuffer()) {
    source = info[1].As<Napi::Buffer<char> >().Data();
    source_len = info[1].As<Napi::Buffer<char> >().Length();
  }
  else {
    Napi::TypeError::New(env, "source argument must be a string or buffer").ThrowAsJavaScriptException();
    return;
  }

  TSQuery *query = ts_query_new(
    language,
    source,
    source_len,
    &error_offset,
    &error_type
  );
  printf("after \"%s\" (this should be the same as first, but is overwritten in some cases)\n", source);
  query_ = query;

  if (error_type > 0) {
    const char *error_name = query_error_names[error_type];
    std::string message = "Query error of type ";
    message += error_name;
    message += " at position ";
    message += std::to_string(error_offset);
    Napi::Error::New(env, message.c_str()).ThrowAsJavaScriptException();
    return;
  }

  Napi::Object self = info.This().As<Napi::Object>();
  Napi::Function _init = self.Get("_init").As<Napi::Function>();
  if (env.IsExceptionPending()) {
    return;
  }
  ts_query_cursor = ts_query_cursor_new();
  _init.Call(self, {});
}

Query::~Query() {
  if (query_) {
    ts_query_delete(query_);
  }
  if (ts_query_cursor) {
    ts_query_cursor_delete(ts_query_cursor);
  }
}

Napi::Value Query::GetPredicates(const Napi::CallbackInfo&info) {
  Napi::Env env = info.Env();
  Napi::Array js_predicates = Napi::Array::New(env);
  TSQuery *ts_query = query_;

  if (!ts_query) {
    return js_predicates;
  }

  uint32_t pattern_len = ts_query_pattern_count(ts_query);


  for (size_t pattern_index = 0; pattern_index < pattern_len; pattern_index++) {
    uint32_t predicates_len;
    const TSQueryPredicateStep *predicates = ts_query_predicates_for_pattern(
        ts_query, pattern_index, &predicates_len);

    Napi::Array js_pattern_predicates = Napi::Array::New(env);

    if (predicates_len > 0) {
      Napi::Array js_predicate = Napi::Array::New(env);

      size_t a_index = 0;
      size_t p_index = 0;
      for (size_t i = 0; i < predicates_len; i++) {
        const TSQueryPredicateStep predicate = predicates[i];
        uint32_t len;
        switch (predicate.type) {
          case TSQueryPredicateStepTypeCapture:
            js_predicate.Set(p_index++, Napi::Number::New(env, TSQueryPredicateStepTypeCapture));
            js_predicate.Set(p_index++,
                Napi::String::New(env, 
                  ts_query_capture_name_for_id(ts_query, predicate.value_id, &len)
                ));
            break;
          case TSQueryPredicateStepTypeString:
            js_predicate.Set(p_index++, Napi::Number::New(env, TSQueryPredicateStepTypeString));
            js_predicate.Set(p_index++,
                Napi::String::New(env, 
                  ts_query_string_value_for_id(ts_query, predicate.value_id, &len)
                ));
            break;
          case TSQueryPredicateStepTypeDone:
            js_pattern_predicates.Set(a_index++, js_predicate);
            js_predicate = Napi::Array::New(env);
            p_index = 0;
            break;
        }
      }
    }

    js_predicates.Set(pattern_index, js_pattern_predicates);
  }

  return js_predicates;
}

Napi::Value Query::Matches(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  const Tree *tree = nullptr;
  if (info[0].IsObject()) {
    tree = Napi::ObjectWrap<Tree>::Unwrap(info[0].As<Napi::Object>());
  }
  if (tree == nullptr) {
    Napi::TypeError::New(env, "Missing argument tree").ThrowAsJavaScriptException();
    return env.Null();
  }
  uint32_t start_row    = info[1].As<Napi::Number>().Uint32Value();
  uint32_t start_column = info[2].As<Napi::Number>().Uint32Value() << 1;
  uint32_t end_row      = info[3].As<Napi::Number>().Uint32Value();
  uint32_t end_column   = info[4].As<Napi::Number>().Uint32Value() << 1;

  TSQuery *ts_query = query_;

  if (!ts_query) {
    Napi::TypeError::New(env, "Invalid query object passed to matches").ThrowAsJavaScriptException();
    return env.Null();
  }
  TSNode rootNode = UnmarshalNode(env, tree);
  if (env.IsExceptionPending()) {
    return env.Null();
  }
  TSPoint start_point = {start_row, start_column};
  TSPoint end_point = {end_row, end_column};
  ts_query_cursor_set_point_range(ts_query_cursor, start_point, end_point);
  ts_query_cursor_exec(ts_query_cursor, ts_query, rootNode);

  Napi::Array js_matches = Napi::Array::New(env);
  unsigned index = 0;
  vector<TSNode> nodes;
  TSQueryMatch match;

  while (ts_query_cursor_next_match(ts_query_cursor, &match)) {
    js_matches.Set(index++, Napi::Number::New(env, match.pattern_index));

    for (uint16_t i = 0; i < match.capture_count; i++) {
      const TSQueryCapture &capture = match.captures[i];

      uint32_t capture_name_len = 0;
      const char *capture_name = ts_query_capture_name_for_id(
          ts_query, capture.index, &capture_name_len);

      TSNode node = capture.node;
      nodes.push_back(node);

      Napi::Value js_capture = Napi::String::New(env, capture_name);
      js_matches.Set(index++, js_capture);
    }
  }

  Napi::Value js_nodes = MarshalNodes(env, tree, nodes.data(), nodes.size());
  if (env.IsExceptionPending()) {
    return env.Null();
  }

  Napi::Array result = Napi::Array::New(env);
  result.Set(uint32_t(0), js_matches);
  result.Set(uint32_t(1), js_nodes);
  return result;
}

Napi::Value Query::Captures(const Napi::CallbackInfo&info) {
  Napi::Env env = info.Env();
  const Tree *tree = nullptr;
  if (info[0].IsObject()) {
    tree = Napi::ObjectWrap<Tree>::Unwrap(info[0].As<Napi::Object>());
  }
  if (tree == nullptr) {
    Napi::TypeError::New(env, "Missing argument tree").ThrowAsJavaScriptException();
    return env.Null();
  }
  uint32_t start_row    = info[1].As<Napi::Number>().Uint32Value();
  uint32_t start_column = info[2].As<Napi::Number>().Uint32Value() << 1;
  uint32_t end_row      = info[3].As<Napi::Number>().Uint32Value();
  uint32_t end_column   = info[4].As<Napi::Number>().Uint32Value() << 1;

  TSQuery *ts_query = query_;

  if (!ts_query) {
    Napi::TypeError::New(env, "Invalid query object passed to captures").ThrowAsJavaScriptException();
    return env.Null();
  }
  TSNode rootNode = UnmarshalNode(env, tree);
  if (env.IsExceptionPending()) {
    return env.Null();
  }
  TSPoint start_point = {start_row, start_column};
  TSPoint end_point = {end_row, end_column};
  ts_query_cursor_set_point_range(ts_query_cursor, start_point, end_point);
  ts_query_cursor_exec(ts_query_cursor, ts_query, rootNode);

  Napi::Array js_matches = Napi::Array::New(env);
  unsigned index = 0;
  vector<TSNode> nodes;
  TSQueryMatch match;
  uint32_t capture_index;

  if (!query_) {
    Napi::TypeError::New(env, "Invalid query object passed to captures, missing setup").ThrowAsJavaScriptException();
    return js_matches;
  }

  while (ts_query_cursor_next_capture(
    ts_query_cursor,
    &match,
    &capture_index
  )) {

    js_matches.Set(index++, Napi::Number::New(env, match.pattern_index));
    js_matches.Set(index++, Napi::Number::New(env, capture_index));

    for (uint16_t i = 0; i < match.capture_count; i++) {
      const TSQueryCapture &capture = match.captures[i];

      uint32_t capture_name_len = 0;
      const char *capture_name = ts_query_capture_name_for_id(
          ts_query, capture.index, &capture_name_len);

      TSNode node = capture.node;
      nodes.push_back(node);

      Napi::Value js_capture = Napi::String::New(env, capture_name);
      js_matches.Set(index++, js_capture);
    }
  }

  Napi::Value js_nodes = MarshalNodes(env, tree, nodes.data(), nodes.size());

  Napi::Array result = Napi::Array::New(env);
  result.Set((uint32_t) 0, js_matches);
  result.Set((uint32_t) 1, js_nodes);
  return result;
}


}  /* namespace node_tree_sitter */
