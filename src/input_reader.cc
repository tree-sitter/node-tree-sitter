#include "./input_reader.h"
#include <v8.h>
#include <nan.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

using namespace v8;

Nan::Persistent<v8::String> InputReader::read_key;
Nan::Persistent<v8::String> InputReader::seek_key;

void InputReader::Init(v8::Local<v8::Object> exports) {
  read_key.Reset(Nan::Persistent<String>(Nan::New("read").ToLocalChecked()));
  seek_key.Reset(Nan::Persistent<String>(Nan::New("seek").ToLocalChecked()));
}

int InputReader::Seek(void *payload, size_t character, size_t byte) {
  InputReader *reader = (InputReader *)payload;
  Local<Function> fn = Local<Function>::Cast(Nan::New(reader->object)->Get(Nan::New(seek_key)));
  if (!fn->IsFunction())
    return 0;

  Local<Value> argv[1] = { Nan::New<Number>(byte) };
  Local<Value> result = fn->Call(Nan::New(reader->object), 1, argv);
  return result->NumberValue();
}

const char * InputReader::Read(void *payload, size_t *bytes_read) {
  InputReader *reader = (InputReader *)payload;
  Local<Function> read_fn = Local<Function>::Cast(Nan::New(reader->object)->Get(Nan::New(read_key)));
  if (!read_fn->IsFunction()) {
    *bytes_read = 0;
    return "";
  }

  Local<String> result = Local<String>::Cast(read_fn->Call(Nan::New(reader->object), 0, NULL));
  if (!result->IsString()) {
    *bytes_read = 0;
    return "";
  }

  *bytes_read = result->WriteUtf8(reader->buffer, 1024, NULL, 2);
  return reader->buffer;
}

TSInput InputReader::Make(Local<Object> object) {
  TSInput result;
  InputReader *reader = new InputReader(new char[1024]);
  reader->object.Reset(object);
  result.payload = (void *)reader;
  result.seek_fn = Seek;
  result.read_fn = Read;
  return result;
}

}  // namespace node_tree_sitter
