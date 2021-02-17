#ifndef NODE_TREE_SITTER_CONVERSIONS_H_
#define NODE_TREE_SITTER_CONVERSIONS_H_

#include <napi.h>
#include <tree_sitter/api.h>
#include "./binding.h"
#include "./optional.h"

namespace node_tree_sitter {

void InitConversions(Napi::Object &, InstanceData *);
void TransferPoint(Napi::Env, const TSPoint &);

Napi::Object RangeToJS(Napi::Env, const TSRange &);
Napi::Object PointToJS(Napi::Env, const TSPoint &);
Napi::Number ByteCountToJS(Napi::Env, uint32_t);

optional<TSPoint> PointFromJS(const Napi::Value &);
optional<uint32_t> ByteCountFromJS(const Napi::Value &);
optional<TSRange> RangeFromJS(const Napi::Value &);

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_CONVERSIONS_H_
