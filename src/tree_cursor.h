#ifndef NODE_TREE_SITTER_TREE_CURSOR_H_
#define NODE_TREE_SITTER_TREE_CURSOR_H_

#include <v8.h>
#include <nan.h>
#include <node_object_wrap.h>
#include "tree_sitter/api.h"

namespace node_tree_sitter {

class TreeCursor final : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Value> NewInstance(TSTreeCursor);
  static TreeCursor *UnwrapTreeCursor(const v8::Local<v8::Value> &);

  TSTreeCursor cursor_;

 private:
  explicit TreeCursor(TSTreeCursor);
  ~TreeCursor() final;

  static void New(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void GotoFirstChild(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void GotoLastChild(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void GotoParent(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void GotoNextSibling(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void GotoPreviousSibling(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void GotoDescendant(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void GotoFirstChildForIndex(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void GotoFirstChildForPosition(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void StartPosition(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void EndPosition(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void CurrentNode(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void Reset(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void ResetTo(const Nan::FunctionCallbackInfo<v8::Value> &);

  static void NodeType(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void NodeIsNamed(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void NodeIsMissing(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void CurrentFieldId(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void CurrentFieldName(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void CurrentDepth(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void CurrentDescendantIndex(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void StartIndex(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void EndIndex(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);

  static Nan::Persistent<v8::Function> constructor;
  static Nan::Persistent<v8::FunctionTemplate> constructor_template;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_TREE_CURSOR_H_
