#ifndef NODE_TREE_SITTER_NODE_H_
#define NODE_TREE_SITTER_NODE_H_

#include <nan.h>
#include <v8.h>
#include <node_object_wrap.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

class Node {
 public:
  static void Init(v8::Local<v8::Object> exports);
  static void MarshalNode(TSNode);
  static TSNode UnmarshalNode(const v8::Local<v8::Value> &tree);

 private:
  static void ToString(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void FirstChildForIndex(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void FirstNamedChildForIndex(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void DescendantForIndex(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void NamedDescendantForIndex(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void DescendantForPosition(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void NamedDescendantForPosition(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void IsMissing(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void HasChanges(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void HasError(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void StartPosition(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void EndPosition(const Nan::FunctionCallbackInfo<v8::Value> &);

  static void Type(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void StartIndex(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void EndIndex(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void IsNamed(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void Id(const Nan::FunctionCallbackInfo<v8::Value> &);

  static void Parent(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void Child(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void ChildCount(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void NamedChild(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void NamedChildCount(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void FirstChild(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void FirstNamedChild(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void LastChild(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void LastNamedChild(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void NextSibling(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void NextNamedSibling(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void PreviousSibling(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void PreviousNamedSibling(const Nan::FunctionCallbackInfo<v8::Value> &);
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_NODE_H_
