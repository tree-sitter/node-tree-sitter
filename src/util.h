#ifndef NODE_TREE_SITTER_UTIL_H_
#define NODE_TREE_SITTER_UTIL_H_

#include <napi.h>

namespace node_tree_sitter {

#define length_of_array(a) (sizeof(a) / sizeof(a[0]))

struct FunctionPair {
  const char *name;
  Napi::Function::Callback callback;
};

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_UTIL_H_
