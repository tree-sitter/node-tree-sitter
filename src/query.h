#ifndef NODE_TREE_SITTER_QUERY_H_
#define NODE_TREE_SITTER_QUERY_H_

#include <v8.h>
#include <nan.h>
#include <node_object_wrap.h>
#include <unordered_map>
#include <tree_sitter/api.h>
#include "./addon_data.h"

namespace node_tree_sitter {

class Query : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports, v8::Local<v8::External> data_ext);
  static v8::Local<v8::Value> NewInstance(AddonData* data, TSQuery *);
  static Query *UnwrapQuery(AddonData* data, const v8::Local<v8::Value> &);

  TSQuery *query_;

 private:
  explicit Query(TSQuery *);
  ~Query();

  static void New(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void Matches(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void Captures(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void GetPredicates(const Nan::FunctionCallbackInfo<v8::Value> &);
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_QUERY_H_
