#include <nan.h>

namespace node_tree_sitter {

struct GetterPair {
  const char *name;
  Nan::GetterCallback callback;
};

struct FunctionPair {
  const char *name;
  Nan::FunctionCallback callback;
};

}  // namespace node_tree_sitter
