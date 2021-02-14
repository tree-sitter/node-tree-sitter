#include <napi.h>
#include <tree_sitter/api.h>
#include "./node.h"
#include "./conversions.h"
#include <cmath>

namespace node_tree_sitter {

using namespace Napi;

static unsigned BYTES_PER_CHARACTER = 2;
static uint32_t *point_transfer_buffer;

void InitConversions(Napi::Object &exports) {
  auto env = exports.Env();
  point_transfer_buffer = static_cast<uint32_t *>(malloc(2 * sizeof(uint32_t)));
  auto js_point_transfer_buffer = ArrayBuffer::New(
    env,
    static_cast<void *>(point_transfer_buffer),
    2 * sizeof(uint32_t)
  );
  exports.Set("pointTransferArray", Uint32Array::New(
    env,
    2,
    js_point_transfer_buffer,
    0
  ));
}

void TransferPoint(const TSPoint &point) {
  point_transfer_buffer[0] = point.row;
  point_transfer_buffer[1] = point.column / 2;
}

Napi::Object RangeToJS(Env env, const TSRange &range) {
  Napi::Object result = Napi::Object::New(env);
  result.Set("startPosition", PointToJS(env, range.start_point));
  result.Set("startIndex", ByteCountToJS(env, range.start_byte));
  result.Set("endPosition", PointToJS(env, range.end_point));
  result.Set("endIndex", ByteCountToJS(env, range.end_byte));
  return result;
}

optional<TSRange> RangeFromJS(const Value &arg) {
  Env env = arg.Env();

  if (!arg.IsObject()) {
    TypeError::New(env, "Range must be a {startPosition, endPosition, startIndex, endIndex} object").ThrowAsJavaScriptException();
    return optional<TSRange>();
  }

  TSRange result;
  Napi::Object js_range = arg.ToObject();

  #define INIT(field, key, Convert) { \
    auto value = js_range.Get(key); \
    if (value.IsEmpty()) { \
      TypeError::New(env, "Range must be a {startPosition, endPosition, startIndex, endIndex} object").ThrowAsJavaScriptException(); \
      return optional<TSRange>(); \
    } \
    auto field = Convert(value); \
    if (field) { \
      result.field = *field; \
    } else { \
      return optional<TSRange>(); \
    } \
  }

  INIT(start_point, "startPosition", PointFromJS);
  INIT(end_point, "endPosition", PointFromJS);
  INIT(start_byte, "startIndex", ByteCountFromJS);
  INIT(end_byte, "endIndex", ByteCountFromJS);

  #undef INIT

  return result;
}

Napi::Object PointToJS(Env env, const TSPoint &point) {
  Napi::Object result = Object::New(env);
  result["row"] = Number::New(env, point.row);
  result["column"] = ByteCountToJS(env, point.column);
  return result;
}

Number ByteCountToJS(Env env, uint32_t byte_count) {
  return Number::New(env, byte_count / BYTES_PER_CHARACTER);
}

optional<TSPoint> PointFromJS(const Value &arg) {
  Env env = arg.Env();

  if (!arg.IsObject()) {
    TypeError::New(env, "Point must be a {row, column} object").ThrowAsJavaScriptException();
    return optional<TSPoint>();
  }

  Napi::Object js_point = arg.ToObject();

  Number js_row = js_point.Get("row").As<Napi::Number>();
  if (!js_row.IsNumber()) {
    TypeError::New(env, "Point must be a {row, column} object").ThrowAsJavaScriptException();
    return optional<TSPoint>();
  }

  Number js_column = js_point.Get("column").As<Napi::Number>();
  if (!js_column.IsNumber()) {
    TypeError::New(env, "Point must be a {row, column} object").ThrowAsJavaScriptException();
    return optional<TSPoint>();
  }

  uint32_t row;
  if (!std::isfinite(js_row.DoubleValue())) {
    row = UINT32_MAX;
  } else {
    row = js_row.Uint32Value();
  }

  uint32_t column;
  if (!std::isfinite(js_column.DoubleValue())) {
    column = UINT32_MAX;
  } else {
    column = js_column.Uint32Value() * BYTES_PER_CHARACTER;
  }

  return TSPoint{row, column};
}

optional<uint32_t> ByteCountFromJS(const Value &arg) {
  Env env = arg.Env();

  if (!arg.IsNumber()) {
    if (!env.IsExceptionPending()) {
      TypeError::New(env, "Character index must be a number").ThrowAsJavaScriptException();
    }
    return optional<uint32_t>();
  }

  Number js_number = arg.ToNumber();
  if (!std::isfinite(js_number.DoubleValue())) {
    return UINT32_MAX;
  } else {
    return js_number.Uint32Value() * BYTES_PER_CHARACTER;
  }
}

}  // namespace node_tree_sitter
