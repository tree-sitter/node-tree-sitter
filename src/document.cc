#include "./document.h"
#include "./ast_node.h"
#include "./parser.h"
#include "./input_reader.h"
#include <node.h>

using namespace v8;

Persistent<Function> Document::constructor;

void Document::Init(Handle<Object> exports) {
  // Constructor
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Document"));

  // Properties
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

Handle<Value> Document::RootNode(const Arguments& args) {
  HandleScope scope;
  Document *document = ObjectWrap::Unwrap<Document>(args.This());
  TSNode *root_node = ts_document_root_node(document->value_);
  return scope.Close(ASTNode::NewInstance(root_node));
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
  Handle<Object> reader = Handle<Object>::Cast(args[0]);
  char *buffer = new char[1024];

  TSInput input;
  input.data = (void *)(new JsInputReader(Persistent<Object>(reader), buffer));
  input.read_fn = JsInputRead;
  input.seek_fn = JsInputSeek;
  input.release_fn = JsInputRelease;

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
