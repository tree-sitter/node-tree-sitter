#include "./input_reader.h"
#include <v8.h>
#include <nan.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

using namespace v8;

struct InputReader {
  v8::Persistent<v8::Object> object;
  char *buffer;

 public:
  InputReader(char *buffer) :
    buffer(buffer)
    {}
};

static int Seek(void *data, size_t position) {
  InputReader *reader = (InputReader *)data;
  Handle<Function> fn = Handle<Function>::Cast(NanNew(reader->object)->Get(NanNew<String>("seek")));
  if (!fn->IsFunction())
    return 0;

  Handle<Value> argv[1] = { NanNew<Integer>(position) };
  Handle<Number> result = Handle<Number>::Cast(fn->Call(NanNew(reader->object), 1, argv));
  return result->NumberValue();
}

static void Release(void *data) {
  InputReader *reader = (InputReader *)data;
  delete reader->buffer;
}

static const char * Read(void *data, size_t *bytes_read) {
  InputReader *reader = (InputReader *)data;
  Handle<Function> read_fn = Handle<Function>::Cast(NanNew(reader->object)->Get(NanNew<String>("read")));
  if (!read_fn->IsFunction())
    return "";

  Handle<String> result = Handle<String>::Cast(read_fn->Call(NanNew(reader->object), 0, NULL));
  if (!result->IsString())
    return "";
  *bytes_read = result->WriteUtf8(reader->buffer, 1024, NULL, 2);
  return reader->buffer;
}

TSInput InputReaderMake(Local<Object> object) {
  TSInput result;
  InputReader *reader = new InputReader(new char[1024]);
  NanAssignPersistent(reader->object, object);
  result.data = (void *)reader;
  result.seek_fn = Seek;
  result.read_fn = Read;
  result.release_fn = Release;
  return result;
}

}  // namespace node_tree_sitter
