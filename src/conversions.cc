#include "./node.h"
#include <nan.h>
#include <tree_sitter/runtime.h>
#include <v8.h>
#include "./conversions.h"

namespace node_tree_sitter {

using namespace v8;

Nan::Persistent<String> row_key;
Nan::Persistent<String> column_key;
Nan::Persistent<String> start_key;
Nan::Persistent<String> end_key;

static unsigned BYTES_PER_CHARACTER = 2;
static uint32_t *point_transfer_buffer;

void InitConversions(Local<Object> exports) {
  row_key.Reset(Nan::Persistent<String>(Nan::New("row").ToLocalChecked()));
  column_key.Reset(Nan::Persistent<String>(Nan::New("column").ToLocalChecked()));
  start_key.Reset(Nan::Persistent<String>(Nan::New("start").ToLocalChecked()));
  end_key.Reset(Nan::Persistent<String>(Nan::New("end").ToLocalChecked()));

  point_transfer_buffer = static_cast<uint32_t *>(malloc(2 * sizeof(uint32_t)));
  auto js_point_transfer_buffer = ArrayBuffer::New(Isolate::GetCurrent(), point_transfer_buffer, 2 * sizeof(uint32_t));
  exports->Set(Nan::New("pointTransferArray").ToLocalChecked(), Uint32Array::New(js_point_transfer_buffer, 0, 2));
}

void TransferPoint(const TSPoint &point) {
  point_transfer_buffer[0] = point.row;
  point_transfer_buffer[1] = point.column / 2;
}

Local<Object> RangeToJS(const TSRange &range) {
  Local<Object> result = Nan::New<Object>();
  result->Set(Nan::New(start_key), PointToJS(range.start));
  result->Set(Nan::New(end_key), PointToJS(range.end));
  return result;
}

Local<Object> PointToJS(const TSPoint &point) {
  Local<Object> result = Nan::New<Object>();
  result->Set(Nan::New(row_key), Nan::New<Number>(point.row));
  result->Set(Nan::New(column_key), ByteCountToJS(point.column));
  return result;
}

Nan::Maybe<TSPoint> PointFromJS(const Local<Value> &arg) {
  Local<Object> js_point = Local<Object>::Cast(arg);
  if (!js_point->IsObject()) {
    Nan::ThrowTypeError("Point must be a {row, column} object");
    return Nan::Nothing<TSPoint>();
  }

  Local<Value> js_row = js_point->Get(Nan::New(row_key));
  if (!js_row->IsNumber()) {
    Nan::ThrowTypeError("Point.row must be a number");
    return Nan::Nothing<TSPoint>();
  }

  Local<Value> js_column = js_point->Get(Nan::New(column_key));
  if (!js_column->IsNumber()) {
    Nan::ThrowTypeError("Point.column must be a number");
    return Nan::Nothing<TSPoint>();
  }

  uint32_t row = static_cast<uint32_t>(js_row->Int32Value());
  uint32_t column = static_cast<uint32_t>(js_column->Int32Value() * BYTES_PER_CHARACTER);
  return Nan::Just<TSPoint>({row, column});
}

Local<Number> ByteCountToJS(uint32_t byte_count) {
  return Nan::New<Number>(byte_count / BYTES_PER_CHARACTER);
}

Nan::Maybe<uint32_t> ByteCountFromJS(const v8::Local<v8::Value> &arg) {
  if (!arg->IsNumber()) {
    Nan::ThrowTypeError("Character index must be a number");
    return Nan::Nothing<uint32_t>();
  }

  return Nan::Just<uint32_t>(arg->Int32Value() * BYTES_PER_CHARACTER);
}

}  // namespace node_tree_sitter
