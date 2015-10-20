#ifndef NODE_TREE_SITTER_INPUT_READER_H_
#define NODE_TREE_SITTER_INPUT_READER_H_

#include <v8.h>
#include <nan.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

class InputReader {
 public:
  static void Init(v8::Handle<v8::Object>);
  static TSInput Make(v8::Handle<v8::Object>);

 private:
  InputReader(char *buffer) :
    buffer(buffer)
    {}

  static int Seek(void *, TSLength);
  static const char * Read(void *, size_t *);

  Nan::Persistent<v8::Object> object;
  char *buffer;

  static Nan::Persistent<v8::String> seek_key;
  static Nan::Persistent<v8::String> read_key;
};


}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_INPUT_READER_H_
