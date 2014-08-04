#include "./document.h"
#include "./ast_node.h"
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

  // Prototype
  tpl->PrototypeTemplate()->Set(
      String::NewSymbol("rootNode"),
      FunctionTemplate::New(RootNode)->GetFunction());
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

Handle<Value> Document::RootNode(const Arguments& args) {
  HandleScope scope;
  Document *document = ObjectWrap::Unwrap<Document>(args.This());
  TSNode *root_node = ts_document_root_node(document->value_);
  if (root_node)
    return scope.Close(ASTNode::NewInstance(root_node));
  else
    return scope.Close(Null());
}

Handle<Value> Document::ToString(const Arguments& args) {
  HandleScope scope;
  Document *document = ObjectWrap::Unwrap<Document>(args.This());
  const char *result = ts_document_string(document->value_);
  return scope.Close(String::Concat(
    String::New("Document: "),
    String::New(result)
  ));
}

Handle<Value> Document::SetInput(const Arguments& args) {
  HandleScope scope;
  Document *document = ObjectWrap::Unwrap<Document>(args.This());
  Persistent<Object> arg = Persistent<Object>::New(Handle<Object>::Cast(args[0]));
  ts_document_set_input(document->value_, InputReaderMake(arg));
  return scope.Close(Undefined());
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

}  // namespace node_tree_sitter
