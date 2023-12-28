#include "./node.h"
#include "./addon_data.h"
#include <nan.h>
#include <tree_sitter/api.h>
#include <v8.h>
#include "./conversions.h"
#include <cmath>

namespace node_tree_sitter {

using namespace v8;

static const unsigned BYTES_PER_CHARACTER = 2;

void InitConversions(Local<Object> exports, Local<External> data_ext) {
  AddonData* data = static_cast<AddonData*>(data_ext->Value());

  data->row_key.Reset(Nan::Persistent<String>(Nan::New("row").ToLocalChecked()));
  data->column_key.Reset(Nan::Persistent<String>(Nan::New("column").ToLocalChecked()));
  data->start_index_key.Reset(Nan::Persistent<String>(Nan::New("startIndex").ToLocalChecked()));
  data->start_position_key.Reset(Nan::Persistent<String>(Nan::New("startPosition").ToLocalChecked()));
  data->end_index_key.Reset(Nan::Persistent<String>(Nan::New("endIndex").ToLocalChecked()));
  data->end_position_key.Reset(Nan::Persistent<String>(Nan::New("endPosition").ToLocalChecked()));

  #if defined(_MSC_VER) && NODE_RUNTIME_ELECTRON && NODE_MODULE_VERSION >= 89
    auto js_point_transfer_buffer = ArrayBuffer::New(Isolate::GetCurrent(), 2 * sizeof(uint32_t));
    point_transfer_buffer = (uint32_t *)(js_point_transfer_buffer->Data());
  #elif V8_MAJOR_VERSION < 8 || (V8_MAJOR_VERSION == 8 && V8_MINOR_VERSION < 4) || (defined(_MSC_VER) && NODE_RUNTIME_ELECTRON)
    point_transfer_buffer = static_cast<uint32_t *>(malloc(2 * sizeof(uint32_t)));
    auto js_point_transfer_buffer = ArrayBuffer::New(Isolate::GetCurrent(), point_transfer_buffer, 2 * sizeof(uint32_t));
  #else
    data->point_transfer_buffer = static_cast<uint32_t *>(malloc(2 * sizeof(uint32_t)));
    auto backing_store = ArrayBuffer::NewBackingStore(data->point_transfer_buffer, 2 * sizeof(uint32_t), BackingStore::EmptyDeleter, nullptr);
    auto js_point_transfer_buffer = ArrayBuffer::New(Isolate::GetCurrent(), std::move(backing_store));
  #endif

  Nan::Set(exports, Nan::New("pointTransferArray").ToLocalChecked(), Uint32Array::New(js_point_transfer_buffer, 0, 2));
}

void TransferPoint(AddonData* data, const TSPoint &point) {
  data->point_transfer_buffer[0] = point.row;
  data->point_transfer_buffer[1] = point.column / 2;
}

Local<Object> RangeToJS(AddonData* data, const TSRange &range) {
  Local<Object> result = Nan::New<Object>();
  Nan::Set(result, Nan::New(data->start_position_key), PointToJS(data, range.start_point));
  Nan::Set(result, Nan::New(data->start_index_key), ByteCountToJS(data, range.start_byte));
  Nan::Set(result, Nan::New(data->end_position_key), PointToJS(data, range.end_point));
  Nan::Set(result, Nan::New(data->end_index_key), ByteCountToJS(data, range.end_byte));
  return result;
}

Nan::Maybe<TSRange> RangeFromJS(AddonData* data, const Local<Value> &arg) {
  if (!arg->IsObject()) {
    Nan::ThrowTypeError("Range must be a {startPosition, endPosition, startIndex, endIndex} object");
    return Nan::Nothing<TSRange>();
  }

  TSRange result;

  Local<Object> js_range = Local<Object>::Cast(arg);

  #define INIT(field, key, Convert) { \
    auto value = Nan::Get(js_range, Nan::New(key)); \
    if (value.IsEmpty()) { \
      Nan::ThrowTypeError("Range must be a {startPosition, endPosition, startIndex, endIndex} object"); \
      return Nan::Nothing<TSRange>(); \
    } \
    auto field = Convert(data, value.ToLocalChecked()); \
    if (field.IsJust()) { \
      result.field = field.FromJust(); \
    } else { \
      return Nan::Nothing<TSRange>(); \
    } \
  }

  INIT(start_point, data->start_position_key, PointFromJS);
  INIT(end_point, data->end_position_key, PointFromJS);
  INIT(start_byte, data->start_index_key, ByteCountFromJS);
  INIT(end_byte, data->end_index_key, ByteCountFromJS);

  #undef INIT

  return Nan::Just(result);
}

Local<Object> PointToJS(AddonData* data, const TSPoint &point) {
  Local<Object> result = Nan::New<Object>();
  Nan::Set(result, Nan::New(data->row_key), Nan::New<Number>(point.row));
  Nan::Set(result, Nan::New(data->column_key), ByteCountToJS(data, point.column));
  return result;
}

Nan::Maybe<TSPoint> PointFromJS(AddonData* data, const Local<Value> &arg) {
  Local<Object> js_point;
  if (!arg->IsObject() || !Nan::To<Object>(arg).ToLocal(&js_point)) {
    Nan::ThrowTypeError("Point must be a {row, column} object");
    return Nan::Nothing<TSPoint>();
  }

  Local<Value> js_row;
  if (!Nan::Get(js_point, Nan::New(data->row_key)).ToLocal(&js_row)) {
    Nan::ThrowTypeError("Point must be a {row, column} object");
    return Nan::Nothing<TSPoint>();
  }

  Local<Value> js_column;
  if (!Nan::Get(js_point, Nan::New(data->column_key)).ToLocal(&js_column)) {
    Nan::ThrowTypeError("Point must be a {row, column} object");
    return Nan::Nothing<TSPoint>();
  }

  uint32_t row;
  if (!std::isfinite(Nan::To<double>(js_row).FromMaybe(0))) {
    row = UINT32_MAX;
  } else if (js_row->IsNumber()) {
    row = Nan::To<uint32_t>(js_row).FromJust();
  } else {
    Nan::ThrowTypeError("Point.row must be a number");
    return Nan::Nothing<TSPoint>();
  }

  uint32_t column;
  if (!std::isfinite(Nan::To<double>(js_column).FromMaybe(0))) {
    column = UINT32_MAX;
  } else if (js_column->IsNumber()) {
    column = Nan::To<uint32_t>(js_column).FromMaybe(0) * BYTES_PER_CHARACTER;
  } else {
    Nan::ThrowTypeError("Point.column must be a number");
    return Nan::Nothing<TSPoint>();
  }

  return Nan::Just<TSPoint>({row, column});
}

Local<Number> ByteCountToJS(AddonData* data, uint32_t byte_count) {
  return Nan::New<Number>(byte_count / BYTES_PER_CHARACTER);
}

Nan::Maybe<uint32_t> ByteCountFromJS(AddonData* data, const v8::Local<v8::Value> &arg) {
  auto result = Nan::To<uint32_t>(arg);
  if (!arg->IsNumber()) {
    Nan::ThrowTypeError("Character index must be a number");
    return Nan::Nothing<uint32_t>();
  }

  return Nan::Just<uint32_t>(result.FromJust() * BYTES_PER_CHARACTER);
}

}  // namespace node_tree_sitter
