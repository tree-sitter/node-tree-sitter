#ifndef TREE_SITTER_INPUT_READER_H
#define TREE_SITTER_INPUT_READER_H

#include <v8.h>

struct JsInputReader {
  v8::Handle<v8::Object> object;
  char *buffer;
  JsInputReader(v8::Handle<v8::Object>, char *);
};

const char * JsInputRead(void *data, size_t *bytes_read);
int JsInputSeek(void *data, size_t position);
void JsInputRelease(void *data);

#endif  // TREE_SITTER_INPUT_READER_H
