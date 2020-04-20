#ifndef NODE_TREE_SITTER_QUERY_H_
#define NODE_TREE_SITTER_QUERY_H_

#include <v8.h>
#include <nan.h>
#include <node_object_wrap.h>
#include <unordered_map>
#include <tree_sitter/api.h>

namespace node_tree_sitter {

class Query : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Value> NewInstance(TSQuery *);
  static Query *UnwrapQuery(const v8::Local<v8::Value> &);
  v8::Local<v8::Array> GetPredicates(uint32_t pattern_index);

  TSQuery *query_;
  std::unordered_map<uint32_t, v8::Persistent<v8::Array>*> cached_predicates_;

 private:
  explicit Query(TSQuery *);
  ~Query();

  static void New(const Nan::FunctionCallbackInfo<v8::Value> &);

  static Nan::Persistent<v8::Function> constructor;
  static Nan::Persistent<v8::FunctionTemplate> constructor_template;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_QUERY_H_
