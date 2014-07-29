#ifndef NODE_TREE_SITTER_INPUT_READER_H_
#define NODE_TREE_SITTER_INPUT_READER_H_

#include <v8.h>

class InputReader {
  v8::Handle<v8::Object> object;
  char *buffer;

 public:
  InputReader(v8::Handle<v8::Object>, char *);
  ~InputReader();

  static const char * Read(void *data, size_t *bytes_read);
  static int Seek(void *data, size_t position);
  static void Release(void *data);
};


#endif  // NODE_TREE_SITTER_INPUT_READER_H_
