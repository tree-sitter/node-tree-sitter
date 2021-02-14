#include <v8.h>
#include <napi.h>
#include <uv.h>
#include "./util.h"

namespace node_tree_sitter {

bool instance_of(Napi::Value value, Napi::Object object) {
  Napi::Env env = value.Env();
  auto maybe_bool = value->InstanceOf(Napi::GetCurrentContext(), object);
  if (maybe_bool.IsNothing())
    return false;
  return maybe_bool;
}

}  // namespace node_tree_sitter
