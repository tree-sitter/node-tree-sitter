#include "./ast_node.h"
#include <nan.h>
#include <tree_sitter/runtime.h>
#include <v8.h>
#include "./conversions.h"

namespace node_tree_sitter {

using namespace v8;

Nan::Persistent<String> row_key;
Nan::Persistent<String> column_key;

void InitConversions() {
  row_key.Reset(Nan::Persistent<String>(Nan::New("row").ToLocalChecked()));
  column_key.Reset(Nan::Persistent<String>(Nan::New("column").ToLocalChecked()));
}

Local<Object> PointToJS(const TSPoint &point) {
  Local<Object> result = Nan::New<Object>();
  result->Set(Nan::New(row_key), Nan::New<Number>(point.row));
  result->Set(Nan::New(column_key), Nan::New<Number>(point.column));
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

  int row = js_row->Int32Value();
  int column = js_column->Int32Value();
  return Nan::Just<TSPoint>({(size_t)row, (size_t)column});
}

Local<Number> ByteCountToJS(size_t byte_count) {
  return Nan::New<Number>(byte_count / 2);
}

Nan::Maybe<size_t> ByteCountFromJS(const v8::Local<v8::Value> &arg) {
  if (!arg->IsNumber()) {
    Nan::ThrowTypeError("Character index must be a number");
    return Nan::Nothing<size_t>();
  }

  return Nan::Just<size_t>(arg->Int32Value() * 2);
}

}  // namespace node_tree_sitter
