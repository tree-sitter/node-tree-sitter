#include "./query_cursor.h"
#include <string>
#include <v8.h>
#include <nan.h>
#include "./node.h"
#include "./query.h"
#include "./language.h"
#include "./logger.h"
#include "./util.h"
#include "./conversions.h"
#include "./tree.h"

namespace node_tree_sitter {

using namespace v8;
using node_methods::UnmarshalNodeId;

Nan::Persistent<Function> QueryCursor::constructor;
Nan::Persistent<FunctionTemplate> QueryCursor::constructor_template;

void QueryCursor::Init(Local<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Local<String> class_name = Nan::New("QueryCursor").ToLocalChecked();
  tpl->SetClassName(class_name);

  FunctionPair methods[] = {
    {"exec", Exec},
  };

  for (size_t i = 0; i < length_of_array(methods); i++) {
    Nan::SetPrototypeMethod(tpl, methods[i].name, methods[i].callback);
  }

  Local<Function> ctor = Nan::GetFunction(tpl).ToLocalChecked();

  constructor_template.Reset(tpl);
  constructor.Reset(ctor);
  Nan::Set(exports, class_name, ctor);
}

QueryCursor::QueryCursor() : query_cursor_(ts_query_cursor_new()) {}

QueryCursor::~QueryCursor() {
  ts_query_cursor_delete(query_cursor_);
}

Local<Value> QueryCursor::NewInstance() {
  Local<Object> self;
  MaybeLocal<Object> maybe_self = Nan::NewInstance(Nan::New(constructor));
  if (maybe_self.ToLocal(&self)) {
    (new QueryCursor())->Wrap(self);
    return self;
  }
  return Nan::Null();
}

const QueryCursor *QueryCursor::UnwrapQueryCursor(const Local<Value> &value) {
  if (!value->IsObject()) return nullptr;
  Local<Object> js_query_cursor = Local<Object>::Cast(value);
  if (!Nan::New(constructor_template)->HasInstance(js_query_cursor)) return nullptr;
  return ObjectWrap::Unwrap<QueryCursor>(js_query_cursor);
}

void QueryCursor::New(const Nan::FunctionCallbackInfo<Value> &info) {
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

  QueryCursor *query_cursor = new QueryCursor();
  query_cursor->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

void QueryCursor::Exec(const Nan::FunctionCallbackInfo<Value> &info) {
  QueryCursor *query_cursor = ObjectWrap::Unwrap<QueryCursor>(info.This());

  Query *query = Query::UnwrapQuery(info[0]);
  const Tree *tree = Tree::UnwrapTree(info[1]);

  if (query == nullptr) {
    Nan::ThrowError("Missing argument query");
    return;
  }

  if (tree == nullptr) {
    Nan::ThrowError("Missing argument tree");
    return;
  }

  if (!info[2]->IsFunction()) {
    Nan::ThrowError("Missing argument callback");
    return;
  }

  Local<Function> callback = Nan::To<Function>(info[2]).ToLocalChecked();

  TSQuery       *ts_query        = query->query_;
  TSQueryCursor *ts_query_cursor = query_cursor->query_cursor_;

  ts_query_cursor_exec(
      ts_query_cursor,
      ts_query,
      ts_tree_root_node(tree->tree_));

  TSQueryMatch match;

  while (ts_query_cursor_next_match(ts_query_cursor, &match)) {

    Local<Array> js_predicates = query->GetPredicates(match.pattern_index);

    printf("match { pattern_index = %i, capture_count = %i, len = %i }\n",
        match.pattern_index,
        match.capture_count,
        js_predicates->Length());

    for (uint16_t i = 0; i < match.capture_count; i++) {
      uint32_t capture_name_len = 0;
      const char *capture_name = ts_query_capture_name_for_id(ts_query, match.captures[0].index, &capture_name_len);
      const char *string = ts_node_string(match.captures[i].node);

      TSNode node = match.captures[i].node;

      Local<String> js_pattern_name = Nan::New<String>(capture_name).ToLocalChecked();
      Local<Value> js_node = node_methods::GetMarshalNode(info, tree, node);

      Local<Value> argv[] = {
        js_pattern_name,
        js_node,
        js_predicates,
      };

      Nan::Call(callback, info.This(), length_of_array(argv), argv);
    }
  }
}

}  // namespace node_tree_sitter
