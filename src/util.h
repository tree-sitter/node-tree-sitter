#include <nan.h>

namespace node_tree_sitter {

#define length_of_array(a) (sizeof(a) / sizeof(a[0]))

struct GetterPair {
  const char *name;
  Nan::GetterCallback callback;
};

struct FunctionPair {
  const char *name;
  Nan::FunctionCallback callback;
};

}  // namespace node_tree_sitter
