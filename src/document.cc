#include "./document.h"
#include <v8.h>
#include <nan.h>
#include "./input_reader.h"
#include "./ast_node.h"
#include "./debugger.h"
#include "./util.h"

namespace node_tree_sitter {

using namespace v8;

Nan::Persistent<Function> Document::constructor;

void Document::Init(Local<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Local<String> class_name = Nan::New("Document").ToLocalChecked();
  tpl->SetClassName(class_name);

  Nan::SetAccessor(
    tpl->InstanceTemplate(),
    Nan::New("rootNode").ToLocalChecked(),
    RootNode);

  FunctionPair methods[] = {
    {"getDebugger", GetDebugger},
    {"setDebugger", SetDebugger},
    {"getInput", GetInput},
    {"setInput", SetInput},
    {"setLanguage", SetLanguage},
    {"edit", Edit},
    {"parse", Parse},
    {"invalidate", Invalidate},
  };

  for (size_t i = 0; i < sizeof(methods) / sizeof(methods[0]); i++)
    Nan::SetPrototypeMethod(tpl, methods[i].name, methods[i].callback);

  constructor.Reset(Nan::Persistent<Function>(tpl->GetFunction()));
  exports->Set(class_name, Nan::New(constructor));
}

Document::Document() : document_(ts_document_make()) {}

Document::~Document() {
  ts_document_free(document_);
}

NAN_METHOD(Document::New) {
  if (info.IsConstructCall()) {
    Document *document = new Document();
    document->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    info.GetReturnValue().Set(Nan::New(constructor)->NewInstance(0, NULL));
  }
}

NAN_GETTER(Document::RootNode) {
  Document *document = ObjectWrap::Unwrap<Document>(info.This());
  TSNode node = ts_document_root_node(document->document_);
  size_t parse_count = ts_document_parse_count(document->document_);
  if (node.data)
    info.GetReturnValue().Set(ASTNode::NewInstance(node, document->document_, parse_count));
  else
    info.GetReturnValue().Set(Nan::Null());
}

NAN_METHOD(Document::GetInput) {
  Document *document = ObjectWrap::Unwrap<Document>(info.This());

  TSInput current_input = ts_document_input(document->document_);
  if (current_input.payload && current_input.seek_fn == InputReader::Seek) {
    InputReader *input = (InputReader *)current_input.payload;
    info.GetReturnValue().Set(Nan::New(input->object));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_METHOD(Document::SetInput) {
  Document *document = ObjectWrap::Unwrap<Document>(info.This());
  Local<Object> input = Local<Object>::Cast(info[0]);
  info.GetReturnValue().Set(info.This());

  if (input->IsNull() || input->IsFalse() || input->IsUndefined()) {
    ts_document_set_input(document->document_, {0, 0, 0});
    return;
  }

  if (!input->IsObject()) {
    Nan::ThrowTypeError("Input must be an object");
    return;
  }

  if (!input->Get(Nan::New("seek").ToLocalChecked())->IsFunction()) {
    Nan::ThrowTypeError("Input must implement seek(n)");
    return;
  }

  if (!input->Get(Nan::New("read").ToLocalChecked())->IsFunction()) {
    Nan::ThrowTypeError("Input must implement read(n)");
    return;
  }

  TSInput current_input = ts_document_input(document->document_);
  ts_document_set_input(document->document_, InputReader::Make(input));
  if (current_input.payload)
    delete (InputReader *)current_input.payload;

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Document::SetLanguage) {
  Local<Object> arg = Local<Object>::Cast(info[0]);

  Document *document = ObjectWrap::Unwrap<Document>(info.This());
  if (arg->InternalFieldCount() != 1) {
    Nan::ThrowTypeError("Invalid language object");
    return;
  }

  TSLanguage *lang = (TSLanguage *)Nan::GetInternalFieldPointer(arg, 0);
  if (!lang) {
    Nan::ThrowTypeError("Invalid language object (null)");
    return;
  }

  ts_document_set_language(document->document_, lang);

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Document::Edit) {
  Local<Object> arg = Local<Object>::Cast(info[0]);
  Document *document = ObjectWrap::Unwrap<Document>(info.This());

  TSInputEdit edit = { 0, 0, 0 };

  Local<Number> position = Local<Number>::Cast(arg->Get(Nan::New("position").ToLocalChecked()));
  if (position->IsNumber())
    edit.position = position->Int32Value();

  Local<Number> chars_removed = Local<Number>::Cast(arg->Get(Nan::New("charsRemoved").ToLocalChecked()));
  if (chars_removed->IsNumber())
    edit.chars_removed = chars_removed->Int32Value();

  Local<Number> chars_inserted = Local<Number>::Cast(arg->Get(Nan::New("charsInserted").ToLocalChecked()));
  if (chars_inserted->IsNumber())
    edit.chars_inserted = chars_inserted->Int32Value();

  ts_document_edit(document->document_, edit);
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Document::Parse) {
  Document *document = ObjectWrap::Unwrap<Document>(info.This());
  ts_document_parse(document->document_);
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Document::Invalidate) {
  Document *document = ObjectWrap::Unwrap<Document>(info.This());
  ts_document_invalidate(document->document_);
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Document::GetDebugger) {
  Document *document = ObjectWrap::Unwrap<Document>(info.This());

  TSDebugger current_debugger = ts_document_debugger(document->document_);
  if (current_debugger.payload && current_debugger.debug_fn == Debugger::Debug) {
    Debugger *debugger = (Debugger *)current_debugger.payload;
    info.GetReturnValue().Set(Nan::New(debugger->func));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_METHOD(Document::SetDebugger) {
  Document *document = ObjectWrap::Unwrap<Document>(info.This());
  Local<Function> func = Local<Function>::Cast(info[0]);

  TSDebugger current_debugger = ts_document_debugger(document->document_);
  if (current_debugger.payload)
    delete (Debugger *)current_debugger.payload;

  if (func->IsFunction()) {
    ts_document_set_debugger(document->document_, Debugger::Make(func));
  } else {
    ts_document_set_debugger(document->document_, { 0, 0 });
    if (!(func->IsNull() || func->IsFalse() || func->IsUndefined())) {
      Nan::ThrowTypeError("Debug callback must either be a function or a falsy value");
      return;
    }
  }

  info.GetReturnValue().Set(info.This());
}

}  // namespace node_tree_sitter
