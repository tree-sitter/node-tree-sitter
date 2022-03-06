#ifndef NODE_TREE_SITTER_QUERY_H_
#define NODE_TREE_SITTER_QUERY_H_

#include <napi.h>
#include <tree_sitter/api.h>

namespace node_tree_sitter {

class Query : public Napi::ObjectWrap<Query> {
 public:
  static void Init(Napi::Object &);
  //static Napi::Value NewInstance(Napi::Env, TSQuery *);
  static Query *UnwrapQuery(const Napi::Value &);

  TSQuery *query_;

  explicit Query(const Napi::CallbackInfo &);
  ~Query();

 private:
  Napi::Value Matches(const Napi::CallbackInfo &);
  Napi::Value Captures(const Napi::CallbackInfo &);
  Napi::Value GetPredicates(const Napi::CallbackInfo &);

  static TSQueryCursor *ts_query_cursor;
  static Napi::FunctionReference constructor;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_QUERY_H_
