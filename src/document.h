#ifndef NODE_TREE_SITTER_DOCUMENT_H_
#define NODE_TREE_SITTER_DOCUMENT_H_

#include <v8.h>
#include <nan.h>
#include <tree_sitter/runtime.h>
#include "./i_ast_node.h"

namespace node_tree_sitter {

class Document : public IASTNode {
 public:
  static void Init(v8::Handle<v8::Object> exports);
  TSNode * node();

 private:
  explicit Document();
  ~Document();

  static NAN_METHOD(New);
  static NAN_METHOD(SetInput);
  static NAN_METHOD(SetLanguage);
  static NAN_METHOD(Edit);

  TSDocument *document_;
  static v8::Persistent<v8::Function> constructor;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_DOCUMENT_H_
