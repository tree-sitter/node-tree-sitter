#include "./input_reader.h"
#include <v8.h>
#include <nan.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

using namespace v8;

size_t DEFAULT_BUFFER_SIZE = 32 * 1024;

Nan::Persistent<v8::String> InputReader::read_key;
Nan::Persistent<v8::String> InputReader::seek_key;
Nan::Persistent<v8::String> InputReader::buffer_size_key;

void InputReader::Init() {
  read_key.Reset(Nan::Persistent<String>(Nan::New("read").ToLocalChecked()));
  seek_key.Reset(Nan::Persistent<String>(Nan::New("seek").ToLocalChecked()));
  buffer_size_key.Reset(Nan::Persistent<String>(Nan::New("bufferSize").ToLocalChecked()));
}

InputReader::InputReader(Local<Object> object) : object(object), partial_string_offset(0) {
  Local<Integer> js_buffer_size = Local<Integer>::Cast(object->Get(Nan::New(buffer_size_key)));
  size_t buffer_size = DEFAULT_BUFFER_SIZE;
  if (js_buffer_size->IsNumber())
    buffer_size = js_buffer_size->Int32Value();
  buffer.resize(buffer_size);
}

int InputReader::Seek(void *payload, uint32_t character, uint32_t byte) {
  InputReader *reader = (InputReader *)payload;
  Local<Function> fn = Local<Function>::Cast(Nan::New(reader->object)->Get(Nan::New(seek_key)));
  if (!fn->IsFunction())
    return 0;

  uint32_t utf16_unit = byte / 2;
  Local<Value> argv[1] = { Nan::New<Number>(utf16_unit) };
  Local<Value> result = fn->Call(Nan::New(reader->object), 1, argv);
  reader->partial_string_offset = 0;
  reader->partial_string.Reset();
  return result->NumberValue();
}

const char * InputReader::Read(void *payload, uint32_t *bytes_read) {
  InputReader *reader = (InputReader *)payload;

  Local<String> result;
  uint32_t start;
  if (reader->partial_string_offset) {
    result = Nan::New(reader->partial_string);
    start = reader->partial_string_offset;
  } else {
    Local<Function> read_fn = Local<Function>::Cast(Nan::New(reader->object)->Get(Nan::New(read_key)));
    if (!read_fn->IsFunction()) {
      *bytes_read = 0;
      return "";
    }

    result = Local<String>::Cast(read_fn->Call(Nan::New(reader->object), 0, NULL));
    start = 0;
    if (!result->IsString()) {
      *bytes_read = 0;
      return "";
    }
  }

  int utf16_units_read = result->Write(reader->buffer.data(), start, reader->buffer.size(), 2);
  int end = start + utf16_units_read;
  *bytes_read = 2 * utf16_units_read;

  if (end < result->Length()) {
    reader->partial_string_offset = end;
    reader->partial_string.Reset(result);
  } else {
    reader->partial_string_offset = 0;
    reader->partial_string.Reset();
  }

  return (const char *)reader->buffer.data();
}

TSInput InputReader::Input() {
  TSInput result;
  result.payload = (void *)this;
  result.encoding = TSInputEncodingUTF16;
  result.seek = Seek;
  result.read = Read;
  result.measure_columns_in_bytes = true;
  return result;
}

}  // namespace node_tree_sitter
