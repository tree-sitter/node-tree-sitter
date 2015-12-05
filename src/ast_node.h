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

  static NAN_METHOD(New);
  static NAN_METHOD(ToString);
  static NAN_METHOD(DescendantForRange);
  static NAN_METHOD(NamedDescendantForRange);
  static NAN_METHOD(IsValid);

  static NAN_GETTER(Type);
  static NAN_GETTER(StartIndex);
  static NAN_GETTER(EndIndex);
  static NAN_GETTER(StartPosition);
  static NAN_GETTER(EndPosition);

  static NAN_GETTER(Parent);
  static NAN_GETTER(Children);
  static NAN_GETTER(NamedChildren);
  static NAN_GETTER(NextSibling);
  static NAN_GETTER(NextNamedSibling);
  static NAN_GETTER(PreviousSibling);
  static NAN_GETTER(PreviousNamedSibling);

  static ASTNode *Unwrap(const v8::Local<v8::Object> &);
  static ASTNode *UnwrapValid(const v8::Local<v8::Object> &);
  static v8::Local<v8::Object> PointToJS(const TSPoint &);

  TSNode node_;
  TSDocument *document_;
  size_t parse_count_;

  static Nan::Persistent<v8::Function> constructor;
  static Nan::Persistent<v8::String> row_key;
  static Nan::Persistent<v8::String> column_key;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_AST_NODE_H_
