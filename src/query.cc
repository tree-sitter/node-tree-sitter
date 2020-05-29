#include "./query.h"
#include <string>
#include <vector>
#include <v8.h>
#include <nan.h>
#include "./node.h"
#include "./language.h"
#include "./logger.h"
#include "./util.h"
#include "./conversions.h"

namespace node_tree_sitter {

using std::vector;
using namespace v8;
using node_methods::UnmarshalNodeId;

const char *query_error_names[] = {
  "TSQueryErrorNone",
  "TSQueryErrorSyntax",
  "TSQueryErrorNodeType",
  "TSQueryErrorField",
  "TSQueryErrorCapture",
};

TSQueryCursor *Query::ts_query_cursor;
Nan::Persistent<Function> Query::constructor;
Nan::Persistent<FunctionTemplate> Query::constructor_template;

void Query::Init(Local<Object> exports) {
  ts_query_cursor = ts_query_cursor_new();

  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Local<String> class_name = Nan::New("Query").ToLocalChecked();
  tpl->SetClassName(class_name);

  FunctionPair methods[] = {
    {"exec", Exec},
    {"matches", Matches},
    {"captures", Captures},
  };

  for (size_t i = 0; i < length_of_array(methods); i++) {
    Nan::SetPrototypeMethod(tpl, methods[i].name, methods[i].callback);
  }

  Local<Function> ctor = Nan::GetFunction(tpl).ToLocalChecked();

  constructor_template.Reset(tpl);
  constructor.Reset(ctor);
  Nan::Set(exports, class_name, ctor);
}

Query::Query(TSQuery *query) : query_(query) {}

Query::~Query() {
  ts_query_delete(query_);
  for (auto &entry : cached_predicates_) {
    entry.second->Reset();
    delete entry.second;
  }
}

Local<Array> Query::GetPredicates(uint32_t pattern_index) {
  const auto &entry = cached_predicates_.find(pattern_index);
  if (entry != cached_predicates_.end()) {
    return Nan::New(*entry->second);
  }

  uint32_t predicates_len;
  const TSQueryPredicateStep *predicates = ts_query_predicates_for_pattern(
      query_, pattern_index, &predicates_len);

  Local<Array> js_predicates = Nan::New<Array>();

  if (predicates_len > 0) {
    Local<Array> js_predicate = Nan::New<Array>();

    size_t a_index = 0;
    size_t p_index = 0;
    for (size_t i = 0; i < predicates_len; i++) {
      const TSQueryPredicateStep predicate = predicates[i];
      uint32_t len;
      switch (predicate.type) {
        case TSQueryPredicateStepTypeCapture:
          Nan::Set(js_predicate, p_index++,
              Nan::New<String>(
                ts_query_capture_name_for_id(query_, predicate.value_id, &len)
              ).ToLocalChecked());
          break;
        case TSQueryPredicateStepTypeString:
          Nan::Set(js_predicate, p_index++,
              Nan::New<String>(
                ts_query_string_value_for_id(query_, predicate.value_id, &len)
              ).ToLocalChecked());
          break;
        case TSQueryPredicateStepTypeDone:
          Nan::Set(js_predicates, a_index++, js_predicate);
          js_predicate = Nan::New<Array>();
          p_index = 0;
          break;
      }
    }

    js_predicate->SetIntegrityLevel(Nan::GetCurrentContext(), v8::IntegrityLevel::kFrozen);
  }

  Persistent<Array> *persistent = new Persistent<Array>();
  persistent->Reset(Isolate::GetCurrent(), js_predicates);

  cached_predicates_[pattern_index] = persistent;

  return js_predicates;
}

Local<Value> Query::NewInstance(TSQuery *query) {
  if (query) {
    Local<Object> self;
    MaybeLocal<Object> maybe_self = Nan::NewInstance(Nan::New(constructor));
    if (maybe_self.ToLocal(&self)) {
      (new Query(query))->Wrap(self);
      return self;
    }
  }
  return Nan::Null();
}

Query *Query::UnwrapQuery(const Local<Value> &value) {
  if (!value->IsObject()) return nullptr;
  Local<Object> js_query = Local<Object>::Cast(value);
  if (!Nan::New(constructor_template)->HasInstance(js_query)) return nullptr;
  return ObjectWrap::Unwrap<Query>(js_query);
}

void Query::New(const Nan::FunctionCallbackInfo<Value> &info) {
  if (!info.IsConstructCall()) {
    Local<Object> self;
    MaybeLocal<Object> maybe_self = Nan::New(constructor)->NewInstance(Nan::GetCurrentContext());
    if (maybe_self.ToLocal(&self)) {
      info.GetReturnValue().Set(self);
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
    return;
  }

  const TSLanguage *language = language_methods::UnwrapLanguage(info[0]);
  const char *source;
  uint32_t source_len;
  uint32_t error_offset = 0;
  TSQueryError error_type = TSQueryErrorNone;

  if (language == nullptr) {
    Nan::ThrowError("Missing language argument");
    return;
  }

  if (info[1]->IsString()) {
    auto string = Nan::To<String> (info[1]).ToLocalChecked();
    source = *Nan::Utf8String(string);
    source_len = string->Length();
  }
  else if (node::Buffer::HasInstance(info[1])) {
    source = node::Buffer::Data(info[1]);
    source_len = node::Buffer::Length(info[1]);
  }
  else {
    Nan::ThrowError("Missing source argument");
    return;
  }

  TSQuery *query = ts_query_new(
      language, source, source_len, &error_offset, &error_type);

  if (error_offset > 0) {
    const char *error_name = query_error_names[error_type];
    std::string message = "Query error of type ";
    message += error_name;
    message += " at position ";
    message += std::to_string(error_offset);
    Nan::ThrowError(message.c_str());
    return;
  }

  Query *query_wrapper = new Query(query);
  query_wrapper->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

void Query::Exec(const Nan::FunctionCallbackInfo<Value> &info) {

  Query *query = Query::UnwrapQuery(info.This());
  const Tree *tree = Tree::UnwrapTree(info[0]);

  if (query == nullptr) {
    Nan::ThrowError("Missing argument query");
    return;
  }

  if (tree == nullptr) {
    Nan::ThrowError("Missing argument tree");
    return;
  }

  if (!info[1]->IsFunction()) {
    Nan::ThrowError("Missing argument callback");
    return;
  }

  Local<Function> callback = Nan::To<Function>(info[1]).ToLocalChecked();

  TSQuery *ts_query = query->query_;

  ts_query_cursor_exec(
      ts_query_cursor,
      ts_query,
      ts_tree_root_node(tree->tree_));

  TSQueryMatch match;

  while (ts_query_cursor_next_match(ts_query_cursor, &match)) {
    Local<Array> js_predicates = query->GetPredicates(match.pattern_index);

    for (uint16_t i = 0; i < match.capture_count; i++) {
      const TSQueryCapture &capture = match.captures[i];

      uint32_t capture_name_len = 0;
      const char *capture_name = ts_query_capture_name_for_id(
          ts_query, capture.index, &capture_name_len);

      TSNode node = capture.node;

      Local<Number> js_pattern_index = Nan::New(match.pattern_index);
      Local<Number> js_capture_index = Nan::New(capture.index);
      Local<String> js_capture_name = Nan::New(capture_name).ToLocalChecked();
      Local<Value> js_node = node_methods::GetMarshalNode(info, tree, node);

      Local<Value> argv[] = {
        js_pattern_index,
        js_capture_index,
        js_capture_name,
        js_node,
        js_predicates,
      };

      Nan::Call(callback, info.This(), length_of_array(argv), argv);
    }
  }
}

void Query::Matches(const Nan::FunctionCallbackInfo<Value> &info) {
  Query *query = Query::UnwrapQuery(info.This());
  const Tree *tree = Tree::UnwrapTree(info[0]);

  if (query == nullptr) {
    Nan::ThrowError("Missing argument query");
    return;
  }

  if (tree == nullptr) {
    Nan::ThrowError("Missing argument tree");
    return;
  }

  TSNode rootNode = node_methods::UnmarshalNode(tree);
  TSQuery *ts_query = query->query_;

  ts_query_cursor_exec(ts_query_cursor, ts_query, rootNode);

  Local<String> js_matches_string = Nan::New("matches").ToLocalChecked();
  Local<String> js_nodes_string = Nan::New("nodes").ToLocalChecked();
  Local<String> js_pattern_string = Nan::New("pattern").ToLocalChecked();
  Local<String> js_captures_string = Nan::New("captures").ToLocalChecked();
  Local<String> js_predicates_string = Nan::New("predicates").ToLocalChecked();
  Local<String> js_name_string = Nan::New("name").ToLocalChecked();
  Local<String> js_node_string = Nan::New("node").ToLocalChecked();

  Local<Array> js_matches = Nan::New<Array>();
  uint32_t match_index = 0;
  TSQueryMatch match;

  vector<TSNode> nodes;

  while (ts_query_cursor_next_match(ts_query_cursor, &match)) {
    Local<Array> js_predicates = query->GetPredicates(match.pattern_index);
    Local<Array> js_captures = Nan::New<Array>();

    for (uint16_t i = 0; i < match.capture_count; i++) {
      const TSQueryCapture &capture = match.captures[i];

      uint32_t capture_name_len = 0;
      const char *capture_name = ts_query_capture_name_for_id(
          ts_query, capture.index, &capture_name_len);

      TSNode node = capture.node;
      nodes.push_back(node);

      Local<Object> js_capture = Nan::New<Object>();
      Nan::Set(js_capture, js_name_string, Nan::New(capture_name).ToLocalChecked());
      Nan::Set(js_capture, js_node_string, Nan::Null());
      Nan::Set(js_captures, i, js_capture);
    }

    Local<Object> js_match = Nan::New<Object>();
    Nan::Set(js_match, js_pattern_string,  Nan::New(match.pattern_index));
    Nan::Set(js_match, js_captures_string, js_captures);
    Nan::Set(js_match, js_predicates_string, js_predicates);
    Nan::Set(js_matches, match_index++, js_match);
  }

  auto js_nodes = node_methods::GetMarshalNodes(info, tree, nodes.data(), nodes.size());

  auto result = Nan::New<Object>();
  Nan::Set(result, js_matches_string, js_matches);
  Nan::Set(result, js_nodes_string, js_nodes);
  info.GetReturnValue().Set(result);
}

void Query::Captures(const Nan::FunctionCallbackInfo<Value> &info) {}


}  // namespace node_tree_sitter
