#include "./input_reader.h"

using namespace v8;

InputReader::InputReader(Handle<Object> object, char *buffer) :
  object(object),
  buffer(buffer)
  {}

InputReader::~InputReader() {
  delete buffer;
}

const char * InputReader::Read(void *data, size_t *bytes_read) {
  InputReader *reader = (InputReader *)data;
  Handle<Function> read_fn = Handle<Function>::Cast(reader->object->Get(String::NewSymbol("read")));
  if (!read_fn->IsFunction())
    return "";

  Handle<String> result = Handle<String>::Cast(read_fn->Call(reader->object, 0, NULL));
  *bytes_read = result->WriteUtf8(reader->buffer, 1024, NULL, 2);
  return reader->buffer;
}

int InputReader::Seek(void *data, size_t position) {
  InputReader *reader = (InputReader *)data;
  Handle<Function> fn = Handle<Function>::Cast(reader->object->Get(String::NewSymbol("seek")));
  if (!fn->IsFunction())
    return 0;

  Handle<Value> argv[1] = { Number::New(position) };
  Handle<Number> result = Handle<Number>::Cast(fn->Call(reader->object, 1, argv));
  return result->NumberValue();
}

void InputReader::Release(void *data) {
  delete (InputReader *)data;
}
