#include "./query.h"
#include <string>
#include <vector>
#include <napi.h>
#include "./node.h"
#include "./language.h"
#include "./logger.h"
#include "./util.h"
#include "./conversions.h"

using std::vector;
using namespace Napi;

namespace node_tree_sitter {

using node_methods::UnmarshalNodeId;

const char *query_error_names[] = {
  "TSQueryErrorNone",
  "TSQueryErrorSyntax",
  "TSQueryErrorNodeType",
  "TSQueryErrorField",
  "TSQueryErrorCapture",
  "TSQueryErrorStructure",
};

void Query::Init(Napi::Env env, Napi::Object exports) {
  auto data = env.GetInstanceData<AddonData>();
  data->ts_query_cursor = ts_query_cursor_new();

  Function ctor = DefineClass(env, "Query", {
    InstanceMethod("_matches", &Query::Matches, napi_default_method),
    InstanceMethod("_captures", &Query::Captures, napi_default_method),
    InstanceMethod("_getPredicates", &Query::GetPredicates, napi_default_method),
  });

  data->query_constructor = Napi::Persistent(ctor);
  exports["Query"] = ctor;
}

Query::Query(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Query>(info) , query_(nullptr) {
  Napi::Env env = info.Env();

  const TSLanguage *language = language_methods::UnwrapLanguage(info[0]);
  const char *source;
  uint32_t source_len;
  uint32_t error_offset = 0;
  TSQueryError error_type = TSQueryErrorNone;

  if (language == nullptr) {
    throw Error::New(env, "Missing language argument");
  }

  if (info[1].IsString()) {
    auto string = info[1].As<String>();
    std::string utf8_string = string.Utf8Value();
    source = utf8_string.data();
    source_len = utf8_string.length();
    query_ = ts_query_new(language, source, source_len, &error_offset, &error_type);
  } else if (info[1].IsBuffer()) {
    Buffer buf = info[1].As<Buffer<char>>();
    source = buf.Data();
    source_len = buf.Length();
    query_ = ts_query_new(language, source, source_len, &error_offset, &error_type);
  }
  else {
    throw Error::New(env, "Missing source argument");
  }

  if (error_offset > 0) {
    const char *error_name = query_error_names[error_type];
    std::string message = "Query error of type ";
    message += error_name;
    message += " at position ";
    message += std::to_string(error_offset);
    throw Error::New(env, message.c_str());
  }

  info.This().As<Napi::Object>().Get("_init").As<Napi::Function>().Call(info.This(), {});
}

Query::~Query() {
  ts_query_delete(query_);
}

Query *Query::UnwrapQuery(const Napi::Value &value) {
  auto data = value.Env().GetInstanceData<AddonData>();
  if (!value.IsObject()) return nullptr;
  Napi::Object js_query = value.As<Object>();
  if (!js_query.InstanceOf(data->query_constructor.Value())) return nullptr;
  return Query::Unwrap(js_query);
}

Napi::Value Query::GetPredicates(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Query *query = Query::UnwrapQuery(info.This());
  auto ts_query = query->query_;

  auto pattern_len = ts_query_pattern_count(ts_query);

  Array js_predicates = Array::New(env);

  for (size_t pattern_index = 0; pattern_index < pattern_len; pattern_index++) {
    uint32_t predicates_len;
    const TSQueryPredicateStep *predicates = ts_query_predicates_for_pattern(
        ts_query, pattern_index, &predicates_len);

    Array js_pattern_predicates = Array::New(env);

    if (predicates_len > 0) {
      Array js_predicate = Array::New(env);

      size_t a_index = 0;
      size_t p_index = 0;
      for (size_t i = 0; i < predicates_len; i++) {
        const TSQueryPredicateStep predicate = predicates[i];
        uint32_t len;
        switch (predicate.type) {
          case TSQueryPredicateStepTypeCapture:
            js_predicate[p_index++] = Number::New(env, TSQueryPredicateStepTypeCapture);
            js_predicate[p_index++] = String::New(env,
              ts_query_capture_name_for_id(ts_query, predicate.value_id, &len)
            );
            break;
          case TSQueryPredicateStepTypeString:
            js_predicate[p_index++] = Number::New(env, TSQueryPredicateStepTypeString);
            js_predicate[p_index++] = String::New(env,
              ts_query_string_value_for_id(ts_query, predicate.value_id, &len)
            );
            break;
          case TSQueryPredicateStepTypeDone:
            js_pattern_predicates[a_index++] = js_predicate;
            js_predicate = Array::New(env);
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
  Napi::Env env = info.Env();
  auto data = env.GetInstanceData<AddonData>();
  Query *query = Query::UnwrapQuery(info.This());
  const Tree *tree = Tree::UnwrapTree(info[0]);
  uint32_t start_row = 0;
  if (info.Length() > 1 && info[1].IsNumber()) {
    start_row = info[1].As<Number>().Uint32Value();
  }
  uint32_t start_column = 0;
  if (info.Length() > 2 && info[2].IsNumber()) {
    start_column = info[2].As<Number>().Uint32Value() << 1;
  }
  uint32_t end_row = 0;
  if (info.Length() > 3 && info[3].IsNumber()) {
    end_row = info[3].As<Number>().Uint32Value();
  }
  uint32_t end_column = 0;
  if (info.Length() > 4 && info[4].IsNumber()) {
    end_column = info[4].As<Number>().Uint32Value() << 1;
  }

  if (query == nullptr) {
    throw Error::New(env, "Missing argument query");
  }

  if (tree == nullptr) {
    throw Error::New(env, "Missing argument tree");
  }

  TSQuery *ts_query = query->query_;
  TSNode rootNode = node_methods::UnmarshalNode(env, tree);
  TSPoint start_point = {start_row, start_column};
  TSPoint end_point = {end_row, end_column};
  ts_query_cursor_set_point_range(data->ts_query_cursor, start_point, end_point);
  ts_query_cursor_exec(data->ts_query_cursor, ts_query, rootNode);

  Array js_matches = Array::New(env);
  unsigned index = 0;
  vector<TSNode> nodes;
  TSQueryMatch match;

  while (ts_query_cursor_next_match(data->ts_query_cursor, &match)) {
    js_matches[index++] = Number::New(env, match.pattern_index);

    for (uint16_t i = 0; i < match.capture_count; i++) {
      const TSQueryCapture &capture = match.captures[i];

      uint32_t capture_name_len = 0;
      const char *capture_name = ts_query_capture_name_for_id(
          ts_query, capture.index, &capture_name_len);

      TSNode node = capture.node;
      nodes.push_back(node);

      String js_capture = String::New(env, capture_name);;
      js_matches[index++] = js_capture;
    }
  }

  auto js_nodes = node_methods::GetMarshalNodes(info, tree, nodes.data(), nodes.size());

  auto result = Array::New(env);
  result[0u] = js_matches;
  result[1] = js_nodes;
  return result;
}

Napi::Value Query::Captures(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  auto data = env.GetInstanceData<AddonData>();
  Query *query = Query::UnwrapQuery(info.This());
  const Tree *tree = Tree::UnwrapTree(info[0]);
  uint32_t start_row = 0;
  if (info.Length() > 1 && info[1].IsNumber()) {
    start_row = info[1].As<Number>().Uint32Value();
  }
  uint32_t start_column = 0;
  if (info.Length() > 2 && info[2].IsNumber()) {
    start_column = info[2].As<Number>().Uint32Value() << 1;
  }
  uint32_t end_row = 0;
  if (info.Length() > 3 && info[3].IsNumber()) {
    end_row = info[3].As<Number>().Uint32Value();
  }
  uint32_t end_column = 0;
  if (info.Length() > 4 && info[4].IsNumber()) {
    end_column = info[4].As<Number>().Uint32Value() << 1;
  }

  if (query == nullptr) {
    throw Error::New(env, "Missing argument query");
  }

  if (tree == nullptr) {
    throw Error::New(env, "Missing argument tree");
  }

  TSQuery *ts_query = query->query_;
  TSNode rootNode = node_methods::UnmarshalNode(env, tree);
  TSPoint start_point = {start_row, start_column};
  TSPoint end_point = {end_row, end_column};
  ts_query_cursor_set_point_range(data->ts_query_cursor, start_point, end_point);
  ts_query_cursor_exec(data->ts_query_cursor, ts_query, rootNode);

  Array js_matches = Array::New(env);
  unsigned index = 0;
  vector<TSNode> nodes;
  TSQueryMatch match;
  uint32_t capture_index;

  while (ts_query_cursor_next_capture(
    data->ts_query_cursor,
    &match,
    &capture_index
  )) {

    js_matches[index++] = Number::New(env, match.pattern_index);
    js_matches[index++] = Number::New(env, capture_index);

    for (uint16_t i = 0; i < match.capture_count; i++) {
      const TSQueryCapture &capture = match.captures[i];

      uint32_t capture_name_len = 0;
      const char *capture_name = ts_query_capture_name_for_id(
          ts_query, capture.index, &capture_name_len);

      TSNode node = capture.node;
      nodes.push_back(node);

      String js_capture = String::New(env, capture_name);;
      js_matches[index++] = js_capture;
    }
  }

  auto js_nodes = node_methods::GetMarshalNodes(info, tree, nodes.data(), nodes.size());

  auto result = Array::New(env);
  result[0u] = js_matches;
  result[1] = js_nodes;
  return result;
}

}  // namespace node_tree_sitter
