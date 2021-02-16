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
  static void Init(Napi::Object exports);

  Query(const Napi::CallbackInfo &info);
  ~Query();

 private:
  TSQuery *query_;
  TSQueryCursor *ts_query_cursor;

  Napi::Value Matches(const Napi::CallbackInfo&);
  Napi::Value Captures(const Napi::CallbackInfo&);
  Napi::Value GetPredicates(const Napi::CallbackInfo&);
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_QUERY_H_
