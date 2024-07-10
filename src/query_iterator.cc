#include "query_iterator.h"
#include "query.h"
#include "tree.h"
#include "node.h"

#include <vector>

using namespace Napi;

namespace node_tree_sitter {

void QueryIterator::Init(Napi::Env env, Napi::Object exports) {
  Function ctor = DefineClass(env, "QueryIterator", {
    InstanceMethod<&QueryIterator::Iterator>(Symbol::WellKnown(env, "iterator"), napi_default_method),
    InstanceMethod<&QueryIterator::Next>("_next", napi_default_method),
    InstanceAccessor<&QueryIterator::GetCaptures>("captures", static_cast<napi_property_attributes>(napi_enumerable | napi_configurable)),
    InstanceAccessor<&QueryIterator::GetQuery>("query", static_cast<napi_property_attributes>(napi_enumerable | napi_configurable)),
    InstanceAccessor<&QueryIterator::GetTree>("tree", static_cast<napi_property_attributes>(napi_enumerable | napi_configurable)),
  });

  auto *data = env.GetInstanceData<AddonData>();
  data->query_iterator_constructor = Napi::Persistent(ctor);
  exports["QueryIterator"] = ctor;
}

QueryIterator::QueryIterator(const Napi::CallbackInfo &info) : Napi::ObjectWrap<QueryIterator>(info) {
  Napi::Env env = info.Env();

  if (!info[0].IsBoolean()) {
    throw Error::New(env, "Missing argument captures");
  }
  captures_ = info[0].As<Boolean>().Value();

  const Query *query = Query::UnwrapQuery(info[1]);
  if (query == nullptr) {
    throw Error::New(env, "Missing argument query");
  }
  query_ = Napi::Persistent(info[1]);

  const Tree *tree = Tree::UnwrapTree(info[2]);
  if (tree == nullptr) {
    throw Error::New(env, "Missing argument tree");
  }
  tree_ = Napi::Persistent(info[2]);

  uint32_t start_row = 0, start_column = 0, end_row = 0, end_column = 0, start_index = 0, end_index = 0,
             match_limit = UINT32_MAX, max_start_depth = UINT32_MAX;

  if (info.Length() > 3 && info[3].IsNumber()) {
    start_row = info[3].As<Number>().Uint32Value();
  }
  if (info.Length() > 4 && info[4].IsNumber()) {
    start_column = info[4].As<Number>().Uint32Value() << 1;
  }
  if (info.Length() > 5 && info[5].IsNumber()) {
    end_row = info[5].As<Number>().Uint32Value();
  }
  if (info.Length() > 6 && info[6].IsNumber()) {
    end_column = info[6].As<Number>().Uint32Value() << 1;
  }
  if (info.Length() > 7 && info[7].IsNumber()) {
    start_index = info[7].As<Number>().Uint32Value();
  }
  if (info.Length() > 8 && info[8].IsNumber()) {
    end_index = info[8].As<Number>().Uint32Value() << 1;
  }
  if (info.Length() > 9 && info[9].IsNumber()) {
    match_limit = info[9].As<Number>().Uint32Value();
  }
  if (info.Length() > 10 && info[10].IsNumber()) {
    max_start_depth = info[10].As<Number>().Uint32Value();
  }

  query_cursor_ = ts_query_cursor_new();
  const TSQuery *ts_query = query->Get();
  TSNode root_node = node_methods::UnmarshalNode(env, tree);
  TSPoint start_point = {start_row, start_column};
  TSPoint end_point = {end_row, end_column};
  ts_query_cursor_set_point_range(query_cursor_, start_point, end_point);
  ts_query_cursor_set_byte_range(query_cursor_, start_index, end_index);
  ts_query_cursor_set_match_limit(query_cursor_, match_limit);
  ts_query_cursor_set_max_start_depth(query_cursor_, max_start_depth);
  ts_query_cursor_exec(query_cursor_, ts_query, root_node);
}

QueryIterator::~QueryIterator() {
  ts_query_cursor_delete(query_cursor_);
}

Napi::Value QueryIterator::Iterator(const Napi::CallbackInfo &info) {
  return info.This();
}

Napi::Value QueryIterator::Next(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  auto result = Object::New(env);
  const Query *query = Query::UnwrapQuery(query_.Value());
  const Tree *tree = Tree::UnwrapTree(tree_.Value());

  Array js_match = Array::New(env);
  unsigned index = 0;
  std::vector<TSNode> nodes;
  TSQueryMatch match;

  if (captures_) {
    uint32_t capture_index;
    if (!ts_query_cursor_next_capture(
      query_cursor_,
      &match,
      &capture_index
    )) {
      result["done"] = true;
      return result;
    }

    js_match[index++] = Number::New(env, match.pattern_index);
    js_match[index++] = Number::New(env, capture_index);

    for (uint16_t i = 0; i < match.capture_count; i++) {
      const TSQueryCapture &capture = match.captures[i];

      uint32_t capture_name_len = 0;
      const char *capture_name = ts_query_capture_name_for_id(
          query->Get(), capture.index, &capture_name_len);

      TSNode node = capture.node;
      nodes.push_back(node);

      String js_capture = String::New(env, capture_name);;
      js_match[index++] = js_capture;
    }
  } else {
    if (!ts_query_cursor_next_match(query_cursor_, &match)) {
      result["done"] = true;
      return result;
    }

    js_match[index++] = Number::New(env, match.pattern_index);

    for (uint16_t i = 0; i < match.capture_count; i++) {
      const TSQueryCapture &capture = match.captures[i];

      uint32_t capture_name_len = 0;
      const char *capture_name = ts_query_capture_name_for_id(
          query->Get(), capture.index, &capture_name_len);

      TSNode node = capture.node;
      nodes.push_back(node);

      String js_capture = String::New(env, capture_name);;
      js_match[index++] = js_capture;
    }
  }

  auto js_nodes = node_methods::GetMarshalNodes(info, tree, nodes.data(), nodes.size());

  auto value = Array::New(env);
  value[0U] = js_match;
  value[1] = js_nodes;
  result["value"] = value;
  return result;
}

Napi::Value QueryIterator::GetCaptures(const Napi::CallbackInfo &info) {
  return Boolean::New(info.Env(), captures_);
}

Napi::Value QueryIterator::GetQuery(const Napi::CallbackInfo &info) {
  return query_.Value();
}

Napi::Value QueryIterator::GetTree(const Napi::CallbackInfo &info) {
  return tree_.Value();
}

} // namespace node_tree_sitter
