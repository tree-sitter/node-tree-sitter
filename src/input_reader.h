#ifndef NODE_TREE_SITTER_INPUT_READER_H_
#define NODE_TREE_SITTER_INPUT_READER_H_

#include <v8.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

TSInput InputReaderMake(v8::Local<v8::Object>);

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_INPUT_READER_H_
