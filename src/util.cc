#include <v8.h>
#include <nan.h>
#include "./util.h"

namespace node_tree_sitter {

bool instance_of(v8::Local<v8::Value> value, v8::Local<v8::Object> object) {
  auto maybe_bool = value->InstanceOf(Nan::GetCurrentContext(), object);
  if (maybe_bool.IsNothing())
    return false;
  return maybe_bool.FromJust();
}

}  // namespace node_tree_sitter
