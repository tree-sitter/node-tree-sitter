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

  static NAN_GETTER(RootNode);
  static NAN_METHOD(New);
  static NAN_METHOD(GetInput);
  static NAN_METHOD(SetInput);
  static NAN_METHOD(SetLanguage);
  static NAN_METHOD(Edit);
  static NAN_METHOD(Parse);
  static NAN_METHOD(Invalidate);
  static NAN_METHOD(GetDebugger);
  static NAN_METHOD(SetDebugger);

  TSDocument *document_;
  static Nan::Persistent<v8::Function> constructor;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_DOCUMENT_H_
