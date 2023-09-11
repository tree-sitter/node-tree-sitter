#include "./util.h"

#include <nan.h>
#include <v8.h>

namespace node_tree_sitter {

v8::Local<v8::Object> GetGlobal(v8::Local<v8::Function>& callback) {
  #if (V8_MAJOR_VERSION > 9 || (V8_MAJOR_VERSION == 9 && V8_MINOR_VERSION > 4))
    return callback->GetCreationContext().ToLocalChecked()->Global();
  #else
    return callback->CreationContext()->Global();
  #endif
}

}  // namespace node_tree_sitter
