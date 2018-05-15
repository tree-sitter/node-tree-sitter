#ifndef SUPERSTRING_TEXT_BUFFER_SNAPSHOT_WRAPPER_H
#define SUPERSTRING_TEXT_BUFFER_SNAPSHOT_WRAPPER_H

#include "nan.h"
#include <string>

// This header can be included by other native node modules, allowing them
// to access the content of a TextBuffer::Snapshot without having to call
// any superstring APIs.

class TextBufferSnapshotWrapper : public Nan::ObjectWrap {
public:
  static void init();

  static v8::Local<v8::Value> new_instance(v8::Local<v8::Object>, void *);

  inline const std::vector<std::pair<const char16_t *, uint32_t>> *slices() {
    return &slices_;
  }

private:
  TextBufferSnapshotWrapper(v8::Local<v8::Object> js_buffer, void *snapshot);
  ~TextBufferSnapshotWrapper();

  static void construct(const Nan::FunctionCallbackInfo<v8::Value> &info);
  static void destroy(const Nan::FunctionCallbackInfo<v8::Value> &info);

  v8::Persistent<v8::Object> js_text_buffer;
  void *snapshot;
  std::vector<std::pair<const char16_t *, uint32_t>> slices_;
};

#endif // SUPERSTRING_TEXT_BUFFER_SNAPSHOT_WRAPPER_H
