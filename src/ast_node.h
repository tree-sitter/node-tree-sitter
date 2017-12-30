#ifndef NODE_TREE_SITTER_AST_NODE_H_
#define NODE_TREE_SITTER_AST_NODE_H_

#include <nan.h>
#include <v8.h>
#include <node_object_wrap.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

class ASTNode : public Nan::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Value> NewInstance(TSNode, TSDocument *, size_t);

 private:
  explicit ASTNode(TSNode, TSDocument *, size_t);

  static void New(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void ToString(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void DescendantForIndex(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void NamedDescendantForIndex(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void DescendantForPosition(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void NamedDescendantForPosition(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void IsValid(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void IsMissing(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void HasChanges(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void HasError(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void StartPosition(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void EndPosition(const Nan::FunctionCallbackInfo<v8::Value> &);

  static void Type(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void StartIndex(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void EndIndex(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void IsNamed(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void Id(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);

  static void Parent(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void Children(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void NamedChildren(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void FirstChild(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void FirstNamedChild(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void LastChild(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void LastNamedChild(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void NextSibling(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void NextNamedSibling(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void PreviousSibling(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void PreviousNamedSibling(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);

  static ASTNode *Unwrap(const v8::Local<v8::Object> &);
  static ASTNode *UnwrapValid(const v8::Local<v8::Object> &);

  TSNode node_;
  TSDocument *document_;
  size_t parse_count_;

  static Nan::Persistent<v8::Function> constructor;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_AST_NODE_H_
