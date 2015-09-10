#ifndef NODE_TREE_SITTER_INPUT_READER_H_
#define NODE_TREE_SITTER_INPUT_READER_H_

#include <v8.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

struct InputReader {
  v8::Persistent<v8::Object> object;
  char *buffer;

 public:
  InputReader(char *buffer) :
    buffer(buffer)
    {}
};


TSInput InputReaderMake(v8::Handle<v8::Object>);

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_INPUT_READER_H_
