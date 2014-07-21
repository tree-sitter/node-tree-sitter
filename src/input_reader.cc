#include "./input_reader.h"

using namespace v8;

JsInputReader::JsInputReader(v8::Persistent<v8::Object> object, char *buffer) :
    object(object),
    buffer(buffer)
    {}

const char * JsInputRead(void *data, size_t *bytes_read) {
  JsInputReader *reader = (JsInputReader *)data;
  Handle<Function> read_fn = Handle<Function>::Cast(reader->object->Get(String::NewSymbol("read")));
  Handle<String> result = Handle<String>::Cast(read_fn->Call(reader->object, 0, NULL));
  *bytes_read = result->WriteUtf8(reader->buffer, 1024, NULL, 2);
  return reader->buffer;
}

int JsInputSeek(void *data, size_t position) {
  JsInputReader *reader = (JsInputReader *)data;
  Handle<Function> fn = Handle<Function>::Cast(reader->object->Get(String::NewSymbol("seek")));
  Handle<Value> argv[1] = { Number::New(position) };
  Handle<Number> result = Handle<Number>::Cast(fn->Call(reader->object, 1, argv));
  return result->NumberValue();
}

void JsInputRelease(void *data) {
  JsInputReader *reader = (JsInputReader *)data;
  delete reader->buffer;
  delete reader;
}
