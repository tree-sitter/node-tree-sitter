#ifndef NODE_TREE_SITTER_DOCUMENT_H_
#define NODE_TREE_SITTER_DOCUMENT_H_

#include <v8.h>
#include <nan.h>
#include <node_object_wrap.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

class Document : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

 private:
  explicit Document();
  ~Document();

  static void RootNode(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void New(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void GetInput(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void SetInput(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void SetLanguage(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void Edit(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void Parse(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void Invalidate(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void GetLogger(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void SetLogger(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void PrintDebuggingGraphs(const Nan::FunctionCallbackInfo<v8::Value> &);

  TSDocument *document_;
  static Nan::Persistent<v8::Function> constructor;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_DOCUMENT_H_
