#ifndef NODE_TREE_SITTER_QUERY_H_
#define NODE_TREE_SITTER_QUERY_H_

#include <v8.h>
#include <napi.h>
#include <uv.h>
#include <node_object_wrap.h>
#include <unordered_map>
#include <tree_sitter/api.h>

namespace node_tree_sitter {

class Query : public Napi::ObjectWrap<Query> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  static Napi::Value NewInstance(TSQuery *);
  static Query *UnwrapQuery(const Napi::Value &);

  TSQuery *query_;

 private:
  explicit Query(TSQuery *);
  ~Query();

  static void New(const Napi::CallbackInfo&);
  static void Matches(const Napi::CallbackInfo&);
  static void Captures(const Napi::CallbackInfo&);
  static void GetPredicates(const Napi::CallbackInfo&);

  static TSQueryCursor *ts_query_cursor;
  static Napi::FunctionReference constructor;
  static Napi::FunctionReference constructor;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_QUERY_H_
