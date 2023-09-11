#ifndef NODE_TREE_SITTER_LANGUAGE_H_
#define NODE_TREE_SITTER_LANGUAGE_H_

#include "tree_sitter/api.h"
#include "./tree.h"

#include <nan.h>
#include <node_object_wrap.h>
#include <v8.h>

namespace node_tree_sitter::language_methods {

void Init(v8::Local<v8::Object>);

const TSLanguage *UnwrapLanguage(const v8::Local<v8::Value> &);

} // namespace node_tree_sitter::language_methods

#endif  // NODE_TREE_SITTER_LANGUAGE_H_
