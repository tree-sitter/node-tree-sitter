#ifndef NODE_TREE_SITTER_DOCUMENT_H_
#define NODE_TREE_SITTER_DOCUMENT_H_

#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>
#include "nan.h"
#include "tree_sitter/runtime.h"

namespace node_tree_sitter {

class Document : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);

 private:
  explicit Document();
  ~Document();

  static NAN_METHOD(New);
  static NAN_METHOD(ToString);
  static NAN_METHOD(SetInput);
  static NAN_METHOD(SetLanguage);
  static NAN_METHOD(Edit);

  static NAN_GETTER(Name);
  static NAN_GETTER(Position);
  static NAN_GETTER(Size);
  static NAN_GETTER(Parent);
  static NAN_GETTER(Children);

  static v8::Persistent<v8::Function> constructor;

  TSDocument *value_;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_DOCUMENT_H_
