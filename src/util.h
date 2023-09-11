#ifndef NODE_TREE_SITTER_UTIL_H_
#define NODE_TREE_SITTER_UTIL_H_

#include <nan.h>
#include <v8.h>

namespace node_tree_sitter {

struct GetterPair {
  const char *name;
  Nan::GetterCallback callback;
};

struct FunctionPair {
  const char *name;
  Nan::FunctionCallback callback;
};

v8::Local<v8::Object> GetGlobal(v8::Local<v8::Function>& callback);

} // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_UTIL_H_
