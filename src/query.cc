#include "./query.h"
#include <string>
#include <vector>
#include <v8.h>
#include <napi.h>
#include <uv.h>
#include "./node.h"
#include "./language.h"
#include "./logger.h"
#include "./util.h"
#include "./conversions.h"

namespace node_tree_sitter {

using std::vector;
using namespace Napi;
using node_methods::UnmarshalNodeId;

const char *query_error_names[] = {
  "TSQueryErrorNone",
  "TSQueryErrorSyntax",
  "TSQueryErrorNodeType",
  "TSQueryErrorField",
  "TSQueryErrorCapture",
};

TSQueryCursor *Query::ts_query_cursor;
Napi::FunctionReference Query::constructor;
Napi::FunctionReference Query::constructor;

void Query::Init(Napi::Object exports) {
  ts_query_cursor = ts_query_cursor_new();

  Napi::FunctionReference tpl = Napi::Function::New(env, New);

  Napi::String class_name = Napi::String::New(env, "Query");
  tpl->SetClassName(class_name);

  FunctionPair methods[] = {
    {"_matches", Matches},
    {"_captures", Captures},
    {"_getPredicates", GetPredicates},
  };

  for (size_t i = 0; i < length_of_array(methods); i++) {
    Napi::SetPrototypeMethod(tpl, methods[i].name, methods[i].callback);
  }

  Napi::Function ctor = Napi::GetFunction(tpl);

  constructor.Reset(tpl);
  constructor.Reset(ctor);
  (exports).Set(class_name, ctor);
}

Query::Query(TSQuery *query) : query_(query) {}

Query::~Query() {
  ts_query_delete(query_);
}

Napi::Value Query::NewInstance(TSQuery *query) {
  if (query) {
    Napi::Object self;
    MaybeNapi::Object maybe_self = Napi::NewInstance(Napi::New(env, constructor));
    if (maybe_self.ToLocal(&self)) {
      (new Query(query))->Wrap(self);
      return self;
    }
  }
  return env.Null();
}

Query *Query::UnwrapQuery(const Napi::Value &value) {
  if (!value.IsObject()) return nullptr;
  Napi::Object js_query = value.As<Napi::Object>();
  if (!Napi::New(env, constructor)->HasInstance(js_query)) return nullptr;
  return ObjectWrap::Unwrap<Query>(js_query);
}

void Query::New(const Napi::CallbackInfo&info) {
  if (!info.IsConstructCall()) {
    Napi::Object self;
    MaybeNapi::Object maybe_self = Napi::New(env, constructor)->NewInstance(Napi::GetCurrentContext());
    if (maybe_self.ToLocal(&self)) {
      return self;
    } else {
      return env.Null();
    }
    return;
  }

  const TSLanguage *language = language_methods::UnwrapLanguage(info[0]);
  const char *source;
  uint32_t source_len;
  uint32_t error_offset = 0;
  TSQueryError error_type = TSQueryErrorNone;

  if (language == nullptr) {
    Napi::Error::New(env, "Missing language argument").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (info[1].IsString()) {
    auto string = Napi::To<String> (info[1]);
    source = string->As<Napi::String>().Utf8Value().c_str();
    source_len = string->Length();
  }
  else if (info[1].IsBuffer()) {
    source = info[1].As<Napi::Buffer<char>>().Data();
    source_len = info[1].As<Napi::Buffer<char>>().Length();
  }
  else {
    Napi::Error::New(env, "Missing source argument").ThrowAsJavaScriptException();
    return env.Null();
  }

  TSQuery *query = ts_query_new(
      language, source, source_len, &error_offset, &error_type);

  if (error_offset > 0) {
    const char *error_name = query_error_names[error_type];
    std::string message = "Query error of type ";
    message += error_name;
    message += " at position ";
    message += std::to_string(error_offset);
    Napi::Error::New(env, message.c_str()).ThrowAsJavaScriptException();
    return env.Null();
  }

  auto self = info.This();

  Query *query_wrapper = new Query(query);
  query_wrapper->Wrap(self);

  auto init =
    Napi::To<Function>(
      (self).Get(Napi::String::New(env, "_init"))
    );
  Napi::Call(init, self, 0, nullptr);

  return self;
}

void Query::GetPredicates(const Napi::CallbackInfo&info) {
  Query *query = Query::UnwrapQuery(info.This());
  auto ts_query = query->query_;

  auto pattern_len = ts_query_pattern_count(ts_query);

  Napi::Array js_predicates = Napi::Array::New(env);

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
            (js_predicate).Set(p_index++, Napi::New(env, TSQueryPredicateStepTypeCapture));
            (js_predicate).Set(p_index++,
                Napi::String::New(env, 
                  ts_query_capture_name_for_id(ts_query, predicate.value_id, &len)
                ));
            break;
          case TSQueryPredicateStepTypeString:
            (js_predicate).Set(p_index++, Napi::New(env, TSQueryPredicateStepTypeString));
            (js_predicate).Set(p_index++,
                Napi::String::New(env, 
                  ts_query_string_value_for_id(ts_query, predicate.value_id, &len)
                ));
            break;
          case TSQueryPredicateStepTypeDone:
            (js_pattern_predicates).Set(a_index++, js_predicate);
            js_predicate = Napi::Array::New(env);
            p_index = 0;
            break;
        }
      }
    }

    (js_predicates).Set(pattern_index, js_pattern_predicates);
  }

  return js_predicates;
}

void Query::Matches(const Napi::CallbackInfo&info) {
  Query *query = Query::UnwrapQuery(info.This());
  const Tree *tree = Tree::UnwrapTree(info[0]);
  uint32_t start_row    = info[1].As<Napi::Number>().Uint32Value().ToChecked();
  uint32_t start_column = info[2].As<Napi::Number>().Uint32Value().ToChecked() << 1;
  uint32_t end_row      = info[3].As<Napi::Number>().Uint32Value().ToChecked();
  uint32_t end_column   = info[4].As<Napi::Number>().Uint32Value().ToChecked() << 1;

  if (query == nullptr) {
    Napi::Error::New(env, "Missing argument query").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (tree == nullptr) {
    Napi::Error::New(env, "Missing argument tree").ThrowAsJavaScriptException();
    return env.Null();
  }

  TSQuery *ts_query = query->query_;
  TSNode rootNode = node_methods::UnmarshalNode(tree);
  TSPoint start_point = {start_row, start_column};
  TSPoint end_point = {end_row, end_column};
  ts_query_cursor_set_point_range(ts_query_cursor, start_point, end_point);
  ts_query_cursor_exec(ts_query_cursor, ts_query, rootNode);

  Napi::Array js_matches = Napi::Array::New(env);
  unsigned index = 0;
  vector<TSNode> nodes;
  TSQueryMatch match;

  while (ts_query_cursor_next_match(ts_query_cursor, &match)) {
    (js_matches).Set(index++, Napi::New(env, match.pattern_index));

    for (uint16_t i = 0; i < match.capture_count; i++) {
      const TSQueryCapture &capture = match.captures[i];

      uint32_t capture_name_len = 0;
      const char *capture_name = ts_query_capture_name_for_id(
          ts_query, capture.index, &capture_name_len);

      TSNode node = capture.node;
      nodes.push_back(node);

      Napi::Value js_capture = Napi::New(env, capture_name);
      (js_matches).Set(index++, js_capture);
    }
  }

  auto js_nodes = node_methods::GetMarshalNodes(info, tree, nodes.data(), nodes.size());

  auto result = Napi::Array::New(env);
  (result).Set(0, js_matches);
  (result).Set(1, js_nodes);
  return result;
}

void Query::Captures(const Napi::CallbackInfo&info) {
  Query *query = Query::UnwrapQuery(info.This());
  const Tree *tree = Tree::UnwrapTree(info[0]);
  uint32_t start_row    = info[1].As<Napi::Number>().Uint32Value().ToChecked();
  uint32_t start_column = info[2].As<Napi::Number>().Uint32Value().ToChecked() << 1;
  uint32_t end_row      = info[3].As<Napi::Number>().Uint32Value().ToChecked();
  uint32_t end_column   = info[4].As<Napi::Number>().Uint32Value().ToChecked() << 1;

  if (query == nullptr) {
    Napi::Error::New(env, "Missing argument query").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (tree == nullptr) {
    Napi::Error::New(env, "Missing argument tree").ThrowAsJavaScriptException();
    return env.Null();
  }

  TSQuery *ts_query = query->query_;
  TSNode rootNode = node_methods::UnmarshalNode(tree);
  TSPoint start_point = {start_row, start_column};
  TSPoint end_point = {end_row, end_column};
  ts_query_cursor_set_point_range(ts_query_cursor, start_point, end_point);
  ts_query_cursor_exec(ts_query_cursor, ts_query, rootNode);

  Napi::Array js_matches = Napi::Array::New(env);
  unsigned index = 0;
  vector<TSNode> nodes;
  TSQueryMatch match;
  uint32_t capture_index;

  while (ts_query_cursor_next_capture(
    ts_query_cursor,
    &match,
    &capture_index
  )) {

    (js_matches).Set(index++, Napi::New(env, match.pattern_index));
    (js_matches).Set(index++, Napi::New(env, capture_index));

    for (uint16_t i = 0; i < match.capture_count; i++) {
      const TSQueryCapture &capture = match.captures[i];

      uint32_t capture_name_len = 0;
      const char *capture_name = ts_query_capture_name_for_id(
          ts_query, capture.index, &capture_name_len);

      TSNode node = capture.node;
      nodes.push_back(node);

      Napi::Value js_capture = Napi::New(env, capture_name);
      (js_matches).Set(index++, js_capture);
    }
  }

  auto js_nodes = node_methods::GetMarshalNodes(info, tree, nodes.data(), nodes.size());

  auto result = Napi::Array::New(env);
  (result).Set(0, js_matches);
  (result).Set(1, js_nodes);
  return result;
}


}  // namespace node_tree_sitter
