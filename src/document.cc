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
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
  tpl->SetClassName(NanNew<String>("Document"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Properties
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("children"),
      Children);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("position"),
      Position);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("size"),
      Size);
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("name"),
      Name);

  // Prototype
  tpl->PrototypeTemplate()->Set(
      NanNew("setInput"),
      NanNew<FunctionTemplate>(SetInput)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      NanNew("setLanguage"),
      NanNew<FunctionTemplate>(SetLanguage)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      NanNew("edit"),
      NanNew<FunctionTemplate>(Edit)->GetFunction());

  tpl->PrototypeTemplate()->Set(
      NanNew("toString"),
      NanNew<FunctionTemplate>(ToString)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      NanNew("parent"),
      NanNew<FunctionTemplate>(Parent)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      NanNew("next"),
      NanNew<FunctionTemplate>(Next)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      NanNew("prev"),
      NanNew<FunctionTemplate>(Prev)->GetFunction());

  NanAssignPersistent(constructor, tpl->GetFunction());
  exports->Set(NanNew("Document"), NanNew(constructor));
}

Document::Document() : value_(ts_document_make()) {}

Document::~Document() {
  ts_document_free(value_);
}

NAN_METHOD(Document::New) {
  NanScope();
  if (args.IsConstructCall()) {
    Document *document = new Document();
    document->Wrap(args.This());
    NanReturnValue(args.This());
  } else {
    NanReturnValue(NanNew(constructor)->NewInstance(0, NULL));
  }
}

NAN_METHOD(Document::ToString) {
  NanScope();
  Document *document = ObjectWrap::Unwrap<Document>(args.This()->ToObject());
  const char *result = ts_node_string(ts_document_root_node(document->value_));
  NanReturnValue(NanNew<String>(result));
}

NAN_METHOD(Document::SetInput) {
  NanScope();
  Document *document = ObjectWrap::Unwrap<Document>(args.This()->ToObject());
  ts_document_set_input(document->value_, InputReaderMake(Local<Object>::Cast(args[0])));
  NanReturnValue(args.This());
}

NAN_METHOD(Document::SetLanguage) {
  NanScope();
  Handle<Object> arg = Handle<Object>::Cast(args[0]);

  if (arg->InternalFieldCount() != 1) {
    NanThrowTypeError("Invalid language object");
    NanReturnUndefined();
  }

  Document *document = ObjectWrap::Unwrap<Document>(args.This()->ToObject());
  TSLanguage *lang = (TSLanguage *)NanGetInternalFieldPointer(arg, 0);
  ts_document_set_language(document->value_, lang);

  NanReturnValue(args.This());
}

NAN_METHOD(Document::Edit) {
  NanScope();

  Handle<Object> arg = Handle<Object>::Cast(args[0]);
  Document *document = ObjectWrap::Unwrap<Document>(args.This()->ToObject());

  TSInputEdit edit = { 0, 0, 0};
  Handle<Number> position = Handle<Number>::Cast(arg->Get(NanNew("position")));
  if (position->IsNumber())
    edit.position = position->Int32Value();
  Handle<Number> bytes_removed = Handle<Number>::Cast(arg->Get(NanNew("bytesRemoved")));
  if (bytes_removed->IsNumber())
    edit.bytes_removed = bytes_removed->Int32Value();
  Handle<Number> bytes_inserted = Handle<Number>::Cast(arg->Get(NanNew("bytesInserted")));
  if (bytes_inserted->IsNumber())
    edit.bytes_inserted = bytes_inserted->Int32Value();

  ts_document_edit(document->value_, edit);
  NanReturnValue(args.This());
}

NAN_METHOD(Document::Parent) {
  NanScope();
  NanReturnNull();
}

NAN_METHOD(Document::Next) {
  NanScope();
  NanReturnNull();
}

NAN_METHOD(Document::Prev) {
  NanScope();
  NanReturnNull();
}

NAN_GETTER(Document::Name) {
  NanScope();
  Document *doc = ObjectWrap::Unwrap<Document>(args.This()->ToObject());
  TSNode *node = ts_document_root_node(doc->value_);
  if (node)
    NanReturnValue(NanNew<String>(ts_node_name(node)));
  else
    NanReturnNull();
}

NAN_GETTER(Document::Size) {
  NanScope();
  Document *doc = ObjectWrap::Unwrap<Document>(args.This()->ToObject());
  TSNode *node = ts_document_root_node(doc->value_);
  if (node)
    NanReturnValue(NanNew<Integer>(ts_node_size(node)));
  else
    NanReturnNull();
}

NAN_GETTER(Document::Position) {
  NanScope();
  Document *doc = ObjectWrap::Unwrap<Document>(args.This()->ToObject());
  TSNode *node = ts_document_root_node(doc->value_);
  if (node)
    NanReturnValue(NanNew<Integer>(ts_node_pos(node)));
  else
    NanReturnNull();
}

NAN_GETTER(Document::Children) {
  NanScope();
  Document *doc = ObjectWrap::Unwrap<Document>(args.This()->ToObject());
  TSNode *node = ts_document_root_node(doc->value_);
  if (node)
    NanReturnValue(ASTNodeArray::NewInstance(node));
  else
    NanReturnNull();
}

}  // namespace node_tree_sitter
