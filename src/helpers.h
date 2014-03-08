#ifndef TREE_SITTER_BINDING_HELPERS_H
#define TREE_SITTER_BINDING_HELPERS_H

#include <string>
#include <v8.h>

static inline void throw_type_error(const char *message) {
  v8::ThrowException(v8::Exception::TypeError(v8::String::New(message)));
}

static inline std::string string_from_js_string(v8::Handle<v8::String> js_string) {
  v8::String::Utf8Value utf8_string(js_string);
  return std::string(*utf8_string);
}

template<typename T>
v8::Handle<T> js_object_get(v8::Handle<v8::Object> object, const char *key) {
  return v8::Handle<T>::Cast(object->Get(v8::String::NewSymbol(key)));
}

template<typename T>
v8::Handle<T> js_array_get(v8::Handle<v8::Array> array, uint32_t i) {
  return v8::Handle<T>::Cast(array->Get(i));
}

#endif
