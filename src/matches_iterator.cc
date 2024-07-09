#include "matches_iterator.h"
#include "query.h"
#include "tree.h"
#include "node.h"

#include <vector>

using namespace Napi;

namespace node_tree_sitter {

void MatchesIterator::Init(Napi::Env env, Napi::Object exports) {
  Function ctor = DefineClass(env, "MatchesIterator", {
    InstanceMethod<&MatchesIterator::Iterator>(Symbol::WellKnown(env, "iterator"), napi_default_method),
    InstanceMethod<&MatchesIterator::Next>("_next", napi_default_method),
    InstanceAccessor<&MatchesIterator::GetQuery>("query", static_cast<napi_property_attributes>(napi_enumerable | napi_configurable)),
    InstanceAccessor<&MatchesIterator::GetTree>("tree", static_cast<napi_property_attributes>(napi_enumerable | napi_configurable)),
  });

  auto *data = env.GetInstanceData<AddonData>();
  data->matches_iterator_constructor = Napi::Persistent(ctor);
  exports["MatchesIterator"] = ctor;
}

MatchesIterator::MatchesIterator(const Napi::CallbackInfo &info) : Napi::ObjectWrap<MatchesIterator>(info) {
  Napi::Env env = info.Env();

  const Query *query = Query::UnwrapQuery(info[0]);
  if (query == nullptr) {
    throw Error::New(env, "Missing argument query");
  }
  query_ = Napi::Persistent(info[0]);

  const Tree *tree = Tree::UnwrapTree(info[1]);
  if (tree == nullptr) {
    throw Error::New(env, "Missing argument tree");
  }
  tree_ = Napi::Persistent(info[1]);

  uint32_t start_row = 0, start_column = 0, end_row = 0, end_column = 0, start_index = 0, end_index = 0,
             match_limit = UINT32_MAX, max_start_depth = UINT32_MAX;

  if (info.Length() > 2 && info[2].IsNumber()) {
    start_row = info[2].As<Number>().Uint32Value();
  }
  if (info.Length() > 3 && info[3].IsNumber()) {
    start_column = info[3].As<Number>().Uint32Value() << 1;
  }
  if (info.Length() > 4 && info[4].IsNumber()) {
    end_row = info[4].As<Number>().Uint32Value();
  }
  if (info.Length() > 5 && info[5].IsNumber()) {
    end_column = info[5].As<Number>().Uint32Value() << 1;
  }
  if (info.Length() > 6 && info[6].IsNumber()) {
    start_index = info[6].As<Number>().Uint32Value();
  }
  if (info.Length() > 7 && info[7].IsNumber()) {
    end_index = info[7].As<Number>().Uint32Value() << 1;
  }
  if (info.Length() > 8 && info[8].IsNumber()) {
    match_limit = info[8].As<Number>().Uint32Value();
  }
  if (info.Length() > 9 && info[9].IsNumber()) {
    max_start_depth = info[9].As<Number>().Uint32Value();
  }

  query_cursor_ = ts_query_cursor_new();
  const TSQuery *ts_query = query->get();
  TSNode root_node = node_methods::UnmarshalNode(env, tree);
  TSPoint start_point = {start_row, start_column};
  TSPoint end_point = {end_row, end_column};
  ts_query_cursor_set_point_range(query_cursor_, start_point, end_point);
  ts_query_cursor_set_byte_range(query_cursor_, start_index, end_index);
  ts_query_cursor_set_match_limit(query_cursor_, match_limit);
  ts_query_cursor_set_max_start_depth(query_cursor_, max_start_depth);
  ts_query_cursor_exec(query_cursor_, ts_query, root_node);
}

MatchesIterator::~MatchesIterator() {
  ts_query_cursor_delete(query_cursor_);
}

Napi::Value MatchesIterator::Iterator(const Napi::CallbackInfo &info) {
  return info.This();
}

Napi::Value MatchesIterator::Next(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  auto result = Object::New(env);
  const Query *query = Query::UnwrapQuery(query_.Value());
  const Tree *tree = Tree::UnwrapTree(tree_.Value());

  TSQueryMatch match;
  if (!ts_query_cursor_next_match(query_cursor_, &match)) {
    result["done"] = true;
    return result;
  }

  Array js_match = Array::New(env);
  unsigned index = 0;
  std::vector<TSNode> nodes;

  js_match[index++] = Number::New(env, match.pattern_index);

  for (uint16_t i = 0; i < match.capture_count; i++) {
    const TSQueryCapture &capture = match.captures[i];

    uint32_t capture_name_len = 0;
    const char *capture_name = ts_query_capture_name_for_id(
        query->get(), capture.index, &capture_name_len);

    TSNode node = capture.node;
    nodes.push_back(node);

    String js_capture = String::New(env, capture_name);;
    js_match[index++] = js_capture;
  }

  auto js_nodes = node_methods::GetMarshalNodes(info, tree, nodes.data(), nodes.size());

  auto value = Array::New(env);
  value[0U] = js_match;
  value[1] = js_nodes;
  result["value"] = value;
  return result;
}

Napi::Value MatchesIterator::GetQuery(const Napi::CallbackInfo &info) {
  return query_.Value();
}

Napi::Value MatchesIterator::GetTree(const Napi::CallbackInfo &info) {
  return tree_.Value();
}

} // namespace node_tree_sitter
