#include "./input_reader.h"
#include <v8.h>
#include <nan.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

using namespace v8;

static int Seek(void *payload, TSLength position) {
  InputReader *reader = (InputReader *)payload;
  Handle<Function> fn = Handle<Function>::Cast(NanNew(reader->object)->Get(NanNew("seek")));
  if (!fn->IsFunction())
    return 0;

  Handle<Value> argv[1] = { NanNew<Number>(position.chars) };
  Handle<Number> result = Handle<Number>::Cast(fn->Call(NanNew(reader->object), 1, argv));
  return result->NumberValue();
}

static const char * Read(void *payload, size_t *bytes_read) {
  InputReader *reader = (InputReader *)payload;
  Handle<Function> read_fn = Handle<Function>::Cast(NanNew(reader->object)->Get(NanNew("read")));
  if (!read_fn->IsFunction()) {
    *bytes_read = 0;
    return "";
  }

  Handle<String> result = Handle<String>::Cast(read_fn->Call(NanNew(reader->object), 0, NULL));
  if (!result->IsString()) {
    *bytes_read = 0;
    return "";
  }

  *bytes_read = result->WriteUtf8(reader->buffer, 1024, NULL, 2);
  return reader->buffer;
}

TSInput InputReaderMake(Handle<Object> object) {
  TSInput result;
  InputReader *reader = new InputReader(new char[1024]);
  NanAssignPersistent(reader->object, object);
  result.payload = (void *)reader;
  result.seek_fn = Seek;
  result.read_fn = Read;
  return result;
}

}  // namespace node_tree_sitter
