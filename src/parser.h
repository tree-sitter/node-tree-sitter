#ifndef NODE_TREE_SITTER_PARSER_H_
#define NODE_TREE_SITTER_PARSER_H_

#include <napi.h>
#include <node_object_wrap.h>
#include <tree_sitter/api.h>

namespace node_tree_sitter {

class Parser : public Napi::ObjectWrap<Parser> {
 public:
  static void Init(Napi::Env env, Napi::Object exports);
  explicit Parser(const Napi::CallbackInfo &info);
  ~Parser();

private:
  TSParser *parser_;

  Napi::Value SetLanguage(const Napi::CallbackInfo &);
  Napi::Value GetLogger(const Napi::CallbackInfo &);
  Napi::Value SetLogger(const Napi::CallbackInfo &);
  Napi::Value Parse(const Napi::CallbackInfo &);
  Napi::Value PrintDotGraphs(const Napi::CallbackInfo &);
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_PARSER_H_
