#ifndef NODE_TREE_SITTER_CONVERSIONS_H_
#define NODE_TREE_SITTER_CONVERSIONS_H_

#include <nan.h>
#include <v8.h>
#include <tree_sitter/api.h>
#include "./addon_data.h"

namespace node_tree_sitter {

void InitConversions(v8::Local<v8::Object> exports, v8::Local<v8::External> data_ext);
v8::Local<v8::Object> RangeToJS(AddonData* data, const TSRange &);
v8::Local<v8::Object> PointToJS(AddonData* data, const TSPoint &);
void TransferPoint(AddonData* data, const TSPoint &);
v8::Local<v8::Number> ByteCountToJS(AddonData* data, uint32_t);
Nan::Maybe<TSPoint> PointFromJS(AddonData* data, const v8::Local<v8::Value> &);
Nan::Maybe<uint32_t> ByteCountFromJS(AddonData* data, const v8::Local<v8::Value> &);
Nan::Maybe<TSRange> RangeFromJS(AddonData* data, const v8::Local<v8::Value> &);

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_CONVERSIONS_H_
