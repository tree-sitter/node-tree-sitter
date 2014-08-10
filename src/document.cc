#include "./document.h"
#include "./ast_node.h"
#include "./ast_node_array.h"
#include "./input_reader.h"
#include <node.h>

namespace node_tree_sitter {

using namespace v8;

Persistent<Function> Document::constructor;

void Document::Init(Handle<Object> exports) {
  // Constructor
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Document"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Properties
  tpl->InstanceTemplate()->SetAccessor(
      String::NewSymbol("children"),
      AccessorGetter(Children));
  tpl->InstanceTemplate()->SetAccessor(
      String::NewSymbol("position"),
      AccessorGetter(Position));
  tpl->InstanceTemplate()->SetAccessor(
      String::NewSymbol("size"),
      AccessorGetter(Size));
  tpl->InstanceTemplate()->SetAccessor(
      String::NewSymbol("name"),
      AccessorGetter(Name));

  // Prototype
  tpl->PrototypeTemplate()->Set(
      String::NewSymbol("toString"),
      FunctionTemplate::New(ToString)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewSymbol("setInput"),
      FunctionTemplate::New(SetInput)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewSymbol("setLanguage"),
      FunctionTemplate::New(SetLanguage)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      String::NewSymbol("edit"),
      FunctionTemplate::New(Edit)->GetFunction());

  constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("Document"), constructor);
}

Document::Document() : value_(ts_document_make()) {}

Document::~Document() {
  ts_document_free(value_);
}

Handle<Value> Document::New(const Arguments& args) {
  HandleScope scope;
  if (args.IsConstructCall()) {
    Document *document = new Document();
    document->Wrap(args.This());
    return args.This();
  } else {
    const int argc = 1;
    Local<Value> argv[argc] = { args[0] };
    return scope.Close(constructor->NewInstance(argc, argv));
  }
}

Handle<Value> Document::ToString(const Arguments& args) {
  HandleScope scope;
  Document *document = ObjectWrap::Unwrap<Document>(args.This());
  const char *result = ts_node_string(ts_document_root_node(document->value_));
  return scope.Close(String::New(result));
}

Handle<Value> Document::SetInput(const Arguments& args) {
  HandleScope scope;
  Document *document = ObjectWrap::Unwrap<Document>(args.This());
  Persistent<Object> arg = Persistent<Object>::New(Handle<Object>::Cast(args[0]));
  ts_document_set_input(document->value_, InputReaderMake(arg));
  return scope.Close(args.This());
}

Handle<Value> Document::SetLanguage(const Arguments& args) {
  HandleScope scope;
  Handle<Object> arg = Handle<Object>::Cast(args[0]);

  if (arg->InternalFieldCount() != 1) {
    ThrowException(Exception::TypeError(String::New("Invalid language object")));
    return scope.Close(Undefined());
  }

  Document *document = ObjectWrap::Unwrap<Document>(args.This());
  Local<External> field = Local<External>::Cast(arg->GetInternalField(0));
  ts_document_set_language(document->value_, (TSLanguage *)field->Value());

  return scope.Close(args.This());
}

Handle<Value> Document::Edit(const Arguments& args) {
  HandleScope scope;

  Handle<Object> arg = Handle<Object>::Cast(args[0]);
  Document *document = ObjectWrap::Unwrap<Document>(args.This());

  TSInputEdit edit = { 0, 0, 0};
  Handle<Number> position = Handle<Number>::Cast(arg->Get(String::NewSymbol("position")));
  if (position->IsNumber())
    edit.position = position->Int32Value();
  Handle<Number> bytes_removed = Handle<Number>::Cast(arg->Get(String::NewSymbol("bytesRemoved")));
  if (bytes_removed->IsNumber())
    edit.bytes_removed = bytes_removed->Int32Value();
  Handle<Number> bytes_inserted = Handle<Number>::Cast(arg->Get(String::NewSymbol("bytesInserted")));
  if (bytes_inserted->IsNumber())
    edit.bytes_inserted = bytes_inserted->Int32Value();

  ts_document_edit(document->value_, edit);
  return scope.Close(args.This());
}

Handle<Value> Document::Name(Local<String>, const AccessorInfo &info) {
  HandleScope scope;
  Document *doc = ObjectWrap::Unwrap<Document>(info.This());
  TSNode *node = ts_document_root_node(doc->value_);
  if (node)
    return scope.Close(String::New(ts_node_name(node)));
  else
    return scope.Close(Null());
}

Handle<Value> Document::Size(Local<String>, const AccessorInfo &info) {
  HandleScope scope;
  Document *doc = ObjectWrap::Unwrap<Document>(info.This());
  TSNode *node = ts_document_root_node(doc->value_);
  if (node)
    return scope.Close(Integer::New(ts_node_size(node)));
  else
    return scope.Close(Null());
}

Handle<Value> Document::Position(Local<String>, const AccessorInfo &info) {
  HandleScope scope;
  Document *doc = ObjectWrap::Unwrap<Document>(info.This());
  TSNode *node = ts_document_root_node(doc->value_);
  if (node)
    return scope.Close(Integer::New(ts_node_pos(node)));
  else
    return scope.Close(Null());
}

Handle<Value> Document::Children(Local<String>, const AccessorInfo &info) {
  HandleScope scope;
  Document *doc = ObjectWrap::Unwrap<Document>(info.This());
  TSNode *node = ts_document_root_node(doc->value_);
  if (node)
    return scope.Close(ASTNodeArray::NewInstance(node));
  else
    return scope.Close(Null());
}

}  // namespace node_tree_sitter
