#ifndef NODE_TREE_SITTER_UTIL_H_
#define NODE_TREE_SITTER_UTIL_H_

#include <napi.h>
#include <v8.h>

namespace node_tree_sitter {

static inline const void *UnmarshalPointer(const uint32_t *buffer) {
  const void *result;
  memcpy(&result, buffer, sizeof(result));
  return result;
}

static inline void MarshalPointer(const void *id, uint32_t *buffer) {
  memset(buffer, 0, sizeof(uint64_t));
  memcpy(buffer, &id, sizeof(id));
}

//=========================================================================
// This library must be able to load parsers that were generated
// using older versions of Tree-sitter, which did not use `napi`.
// So we need to use the V8 APIs directly here.
//
// The following assertion and function were taken from Node's `napi` implementation:
// github.com/nodejs/node/blob/53ca0b9ae145c430842bf78e553e3b6cbd2823aa/src/js_native_api_v8.h

static_assert(
  sizeof(v8::Local<v8::Value>) == sizeof(napi_value),
  "Cannot convert between v8::Local<v8::Value> and napi_value"
);

static inline v8::Local<v8::Value> V8LocalValueFromJsValue(napi_value v) {
  v8::Local<v8::Value> local;
  memcpy(static_cast<void*>(&local), &v, sizeof(v));
  return local;
}

//=========================================================================

static inline void *GetInternalFieldPointer(Napi::Value value) {
  if (value.IsObject()) {
    v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast(
      V8LocalValueFromJsValue(value)
    );
    if (object->InternalFieldCount() == 1) {
      return object->GetAlignedPointerFromInternalField(0);
    }
  }
  return nullptr;
}

bool instance_of(v8::Local<v8::Value> value, v8::Local<v8::Object> object);

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_UTIL_H_
