#include "./document.h"
#include <v8.h>
#include <nan.h>
#include "./input_reader.h"
#include "./ast_node.h"
#include "./debugger.h"

namespace node_tree_sitter {

using namespace v8;

Persistent<Function> Document::constructor;

void Document::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(New);
  tpl->SetClassName(NanNew("Document"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Properties
  tpl->InstanceTemplate()->SetAccessor(
      NanNew("rootNode"),
      RootNode);

  // Prototype methods
  tpl->PrototypeTemplate()->Set(
      NanNew("setDebugger"),
      NanNew<FunctionTemplate>(SetDebugger)->GetFunction());
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
      NanNew("parse"),
      NanNew<FunctionTemplate>(Parse)->GetFunction());
  tpl->PrototypeTemplate()->Set(
      NanNew("invalidate"),
      NanNew<FunctionTemplate>(Invalidate)->GetFunction());

  NanAssignPersistent(constructor, tpl->GetFunction());
  exports->Set(NanNew("Document"), NanNew(constructor));
}

Document::Document() : document_(ts_document_make()) {}

Document::~Document() {
  ts_document_free(document_);
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

NAN_GETTER(Document::RootNode) {
  NanScope();
  Document *document = ObjectWrap::Unwrap<Document>(args.This()->ToObject());
  TSNode node = ts_document_root_node(document->document_);
  size_t parse_count = ts_document_parse_count(document->document_);
  if (node.data)
    NanReturnValue(ASTNode::NewInstance(node, document->document_, parse_count));
  else
    NanReturnNull();
}

NAN_METHOD(Document::SetInput) {
  NanScope();
  Document *document = ObjectWrap::Unwrap<Document>(args.This()->ToObject());
  Handle<Object> input = Handle<Object>::Cast(args[0]);

  if (!input->Get(NanNew("seek"))->IsFunction())
    NanThrowTypeError("Input must implement seek(n)");

  if (!input->Get(NanNew("read"))->IsFunction())
    NanThrowTypeError("Input must implement read(n)");

  TSInput current_input = ts_document_input(document->document_);
  ts_document_set_input(document->document_, InputReaderMake(input));
  if (current_input.payload)
    delete (InputReader *)current_input.payload;

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
  ts_document_set_language(document->document_, lang);

  NanReturnValue(args.This());
}

NAN_METHOD(Document::Edit) {
  NanScope();

  Handle<Object> arg = Handle<Object>::Cast(args[0]);
  Document *document = ObjectWrap::Unwrap<Document>(args.This()->ToObject());

  TSInputEdit edit = { 0, 0, 0 };

  Handle<Number> position = Handle<Number>::Cast(arg->Get(NanNew("position")));
  if (position->IsNumber())
    edit.position = position->Int32Value();

  Handle<Number> chars_removed = Handle<Number>::Cast(arg->Get(NanNew("charsRemoved")));
  if (chars_removed->IsNumber())
    edit.chars_removed = chars_removed->Int32Value();

  Handle<Number> chars_inserted = Handle<Number>::Cast(arg->Get(NanNew("charsInserted")));
  if (chars_inserted->IsNumber())
    edit.chars_inserted = chars_inserted->Int32Value();

  ts_document_edit(document->document_, edit);
  NanReturnValue(args.This());
}

NAN_METHOD(Document::Parse) {
  NanScope();
  Document *document = ObjectWrap::Unwrap<Document>(args.This()->ToObject());
  ts_document_parse(document->document_);
  NanReturnValue(args.This());
}

NAN_METHOD(Document::Invalidate) {
  NanScope();
  Document *document = ObjectWrap::Unwrap<Document>(args.This()->ToObject());
  ts_document_invalidate(document->document_);
  NanReturnValue(args.This());
}

NAN_METHOD(Document::SetDebugger) {
  NanScope();

  Document *document = ObjectWrap::Unwrap<Document>(args.This()->ToObject());
  Handle<Function> func = Handle<Function>::Cast(args[0]);

  TSDebugger current_debugger = ts_document_debugger(document->document_);
  if (current_debugger.payload)
    delete (Debugger *)current_debugger.payload;

  if (func->IsFunction()) {
    ts_document_set_debugger(document->document_, DebuggerMake(func));
  } else {
    ts_document_set_debugger(document->document_, { 0, 0 });
    if (!(func->IsNull() || func->IsFalse() || func->IsUndefined()))
      NanThrowTypeError("Debug callback must either be a function or a falsy value");
  }

  NanReturnValue(args.This());
}

}  // namespace node_tree_sitter
