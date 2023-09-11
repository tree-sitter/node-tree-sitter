#ifndef NODE_TREE_SITTER_QUERY_H_
#define NODE_TREE_SITTER_QUERY_H_

#include "tree_sitter/api.h"

#include <nan.h>
#include <node_object_wrap.h>
#include <v8.h>

namespace node_tree_sitter {

class Query final : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Value> NewInstance(TSQuery *);
  static Query *UnwrapQuery(const v8::Local<v8::Value> &);

  TSQuery *query_;

 private:
  explicit Query(TSQuery *);
  ~Query() final;

  static void New(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void Matches(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void Captures(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void GetPredicates(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void DisableCapture(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void DisablePattern(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void IsPatternGuaranteedAtStep(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void IsPatternRooted(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void IsPatternNonLocal(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void StartIndexForPattern(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void DidExceedMatchLimit(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void MatchLimit(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);

  static TSQueryCursor *ts_query_cursor;
  static Nan::Persistent<v8::Function> constructor;
  static Nan::Persistent<v8::FunctionTemplate> constructor_template;
};

} // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_QUERY_H_
