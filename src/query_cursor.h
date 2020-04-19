#ifndef NODE_TREE_SITTER_QUERY_CURSOR_H_
#define NODE_TREE_SITTER_QUERY_CURSOR_H_

#include <v8.h>
#include <nan.h>
#include <node_object_wrap.h>
#include <unordered_map>
#include <tree_sitter/api.h>

namespace node_tree_sitter {

class QueryCursor : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Value> NewInstance();
  static const QueryCursor *UnwrapQueryCursor(const v8::Local<v8::Value> &);

  TSQueryCursor *query_cursor_;

 private:
  explicit QueryCursor();
  ~QueryCursor();

  static void New(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void Exec(const Nan::FunctionCallbackInfo<v8::Value> &);

  static Nan::Persistent<v8::Function> constructor;
  static Nan::Persistent<v8::FunctionTemplate> constructor_template;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_QUERY_CURSOR_H_
