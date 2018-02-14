#ifndef NODE_TREE_SITTER_INPUT_READER_H_
#define NODE_TREE_SITTER_INPUT_READER_H_

#include <v8.h>
#include <nan.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

class InputReader {
 public:
  static void Init();
  static int Seek(void *, uint32_t, TSPoint);
  static const char * Read(void *, uint32_t *);

  InputReader(v8::Local<v8::Object>);
  TSInput Input();

  Nan::Persistent<v8::Object> object;

 private:
  std::vector<uint16_t> buffer;
  Nan::Persistent<v8::String> partial_string;
  size_t partial_string_offset;

  static Nan::Persistent<v8::String> seek_key;
  static Nan::Persistent<v8::String> read_key;
  static Nan::Persistent<v8::String> buffer_size_key;
};


}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_INPUT_READER_H_
