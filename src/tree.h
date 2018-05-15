#ifndef NODE_TREE_SITTER_TREE_H_
#define NODE_TREE_SITTER_TREE_H_

#include <v8.h>
#include <nan.h>
#include <node_object_wrap.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

class Tree : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Value> NewInstance(TSTree *);
  static const TSTree *UnwrapTree(const v8::Local<v8::Value> &);

 private:
  explicit Tree(TSTree *);
  ~Tree();

  static void New(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void Walk(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void Edit(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void RootNode(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void PrintDotGraph(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void GetChangedRanges(const Nan::FunctionCallbackInfo<v8::Value> &);

  TSTree *tree_;
  static Nan::Persistent<v8::Function> constructor;
  static Nan::Persistent<v8::FunctionTemplate> constructor_template;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_TREE_H_
