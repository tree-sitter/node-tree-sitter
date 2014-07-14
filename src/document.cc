#include "./binding.h"
#include <node.h>

using namespace v8;

Persistent<Function> Document::constructor;

void Document::Init(Handle<Object> exports) {
  // Constructor
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Document"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  tpl->PrototypeTemplate()->Set(
    String::NewSymbol("toString"),
    FunctionTemplate::New(ToString)->GetFunction());
  tpl->PrototypeTemplate()->Set(
    String::NewSymbol("setInput"),
    FunctionTemplate::New(SetInput)->GetFunction());
  tpl->PrototypeTemplate()->Set(
    String::NewSymbol("setParser"),
    FunctionTemplate::New(SetParser)->GetFunction());

  constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("Document"), constructor);
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
  const char *result = ts_document_string(document->value_);

  return scope.Close(String::Concat(
    String::New("Document: "),
    String::New(result)
  ));
}

struct JsInputReader {
  Persistent<Object> object;
  char *buffer;
  JsInputReader(Persistent<Object> object, char *buffer) :
    object(object),
    buffer(buffer)
    {}
};

const char * JsInputRead(void *data, size_t *bytes_read) {
  JsInputReader *reader = (JsInputReader *)data;
  Handle<Function> read_fn = Handle<Function>::Cast(reader->object->Get(String::NewSymbol("read")));
  Handle<String> result = Handle<String>::Cast(read_fn->Call(reader->object, 0, NULL));
  *bytes_read = result->WriteUtf8(reader->buffer, 1024, NULL, 2);
  return reader->buffer;
}

int JsInputSeek(void *data, size_t position) {
  JsInputReader *reader = (JsInputReader *)data;
  Handle<Function> fn = Handle<Function>::Cast(reader->object->Get(String::NewSymbol("seek")));
  Handle<Value> argv[1] = { Number::New(position) };
  Handle<Number> result = Handle<Number>::Cast(fn->Call(reader->object, 1, argv));
  return result->NumberValue();
}

void JsInputRelease(void *data) {
  JsInputReader *reader = (JsInputReader *)data;
  delete reader->buffer;
  delete reader;
}

Handle<Value> Document::SetInput(const Arguments& args) {
  HandleScope scope;
  Handle<Object> reader = Handle<Object>::Cast(args[0]);
  char *buffer = new char[1024];

  TSInput input = {
    .data = (void *)(new JsInputReader(Persistent<Object>(reader), buffer)),
    .read_fn = JsInputRead,
    .seek_fn = JsInputSeek,
    .release_fn = JsInputRelease
  };

  Document *document = ObjectWrap::Unwrap<Document>(args.This());
  ts_document_set_input(document->value_, input);

  return scope.Close(String::New(""));
}

Handle<Value> Document::SetParser(const Arguments& args) {
  HandleScope scope;
  Handle<Object> arg = Handle<Object>::Cast(args[0]);
  Document *document = ObjectWrap::Unwrap<Document>(args.This());
  Parser *parser = ObjectWrap::Unwrap<Parser>(arg);

  if (parser)
    ts_document_set_parser(document->value_, parser->value());
  else
    ThrowException(Exception::TypeError(String::New("Expected parser object")));

  return scope.Close(Undefined());
}

Document::Document() : value_(ts_document_make()) {}

Document::~Document() {
  ts_document_free(value_);
}
