#include "./input_reader.h"

namespace node_tree_sitter {

using namespace v8;

struct InputReader {
  v8::Handle<v8::Object> object;
  char *buffer;

 public:
  InputReader(v8::Handle<v8::Object> object, char *buffer) :
    object(object),
    buffer(buffer)
    {}
};

static int Seek(void *data, size_t position) {
  InputReader *reader = (InputReader *)data;
  Handle<Function> fn = Handle<Function>::Cast(reader->object->Get(String::NewSymbol("seek")));
  if (!fn->IsFunction())
    return 0;

  Handle<Value> argv[1] = { Number::New(position) };
  Handle<Number> result = Handle<Number>::Cast(fn->Call(reader->object, 1, argv));
  return result->NumberValue();
}

static void Release(void *data) {
  InputReader *reader = (InputReader *)data;
  delete reader->buffer;
}

static const char * Read(void *data, size_t *bytes_read) {
  InputReader *reader = (InputReader *)data;
  Handle<Function> read_fn = Handle<Function>::Cast(reader->object->Get(String::NewSymbol("read")));
  if (!read_fn->IsFunction())
    return "";

  Handle<String> result = Handle<String>::Cast(read_fn->Call(reader->object, 0, NULL));
  if (!result->IsString())
    return "";
  *bytes_read = result->WriteUtf8(reader->buffer, 1024, NULL, 2);
  return reader->buffer;
}

TSInput InputReaderMake(Handle<Object> object) {
  TSInput result;
  result.data = (void *)new InputReader(object, new char[1024]);
  result.seek_fn = Seek;
  result.read_fn = Read;
  result.release_fn = Release;
  return result;
}

}  // namespace node_tree_sitter
