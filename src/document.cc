#include "./document.h"
#include <string>
#include <v8.h>
#include <nan.h>
#include "./input_reader.h"
#include "./ast_node.h"
#include "./logger.h"
#include "./util.h"
#include "./conversions.h"

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
    {"getLogger", GetLogger},
    {"setLogger", SetLogger},
    {"getInput", GetInput},
    {"setInput", SetInput},
    {"setLanguage", SetLanguage},
    {"edit", Edit},
    {"invalidate", Invalidate},
    {"_printDebuggingGraphs", PrintDebuggingGraphs},
    {"parse", Parse},
  };

  for (size_t i = 0; i < length_of_array(methods); i++)
    Nan::SetPrototypeMethod(tpl, methods[i].name, methods[i].callback);

  constructor.Reset(Nan::Persistent<Function>(tpl->GetFunction()));
  exports->Set(class_name, Nan::New(constructor));
  exports->Set(Nan::New("LANGUAGE_VERSION").ToLocalChecked(), Nan::New<Number>(TREE_SITTER_LANGUAGE_VERSION));
}

Document::Document() : document_(ts_document_new()) {}

Document::~Document() {
  TSInput input = ts_document_input(document_);
  if (input.payload)
    delete (InputReader *)input.payload;
  ts_document_free(document_);
}

NAN_METHOD(Document::New) {
  if (info.IsConstructCall()) {
    Document *document = new Document();
    document->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    Local<Object> self;
    MaybeLocal<Object> maybe_self = Nan::New(constructor)->NewInstance(Nan::GetCurrentContext());
    if (maybe_self.ToLocal(&self)) {
      info.GetReturnValue().Set(self);
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
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
  if (current_input.payload && current_input.seek == InputReader::Seek) {
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
    ts_document_set_input(document->document_, {0, 0, 0, TSInputEncodingUTF16, true});
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
  InputReader *input_reader = new InputReader(input);
  ts_document_set_input(document->document_, input_reader->Input());

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

  TSLanguage *language = (TSLanguage *)Nan::GetInternalFieldPointer(arg, 0);
  if (!language) {
    Nan::ThrowTypeError("Invalid language object");
    return;
  }

  if (ts_language_version(language) != TREE_SITTER_LANGUAGE_VERSION) {
    std::string message = "Incompatible language version. Expected " +
      std::to_string(TREE_SITTER_LANGUAGE_VERSION) + ". Got " +
      std::to_string(ts_language_version(language));
    Nan::ThrowError(Nan::RangeError(message.c_str()));
    return;
  }

  ts_document_set_language(document->document_, language);

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Document::Edit) {
  Local<Object> arg = Local<Object>::Cast(info[0]);
  Document *document = ObjectWrap::Unwrap<Document>(info.This());

  auto start_byte = ByteCountFromJS(arg->Get(Nan::New("startIndex").ToLocalChecked()));
  if (start_byte.IsNothing()) return;

  auto bytes_removed = ByteCountFromJS(arg->Get(Nan::New("lengthRemoved").ToLocalChecked()));
  if (bytes_removed.IsNothing()) return;

  auto bytes_added = ByteCountFromJS(arg->Get(Nan::New("lengthAdded").ToLocalChecked()));
  if (bytes_added.IsNothing()) return;

  auto start_position = PointFromJS(arg->Get(Nan::New("startPosition").ToLocalChecked()));
  if (start_position.IsNothing()) return;

  auto extent_removed = PointFromJS(arg->Get(Nan::New("extentRemoved").ToLocalChecked()));
  if (extent_removed.IsNothing()) return;

  auto extent_added = PointFromJS(arg->Get(Nan::New("extentAdded").ToLocalChecked()));
  if (extent_added.IsNothing()) return;

  TSInputEdit edit;
  edit.start_byte = start_byte.FromJust();
  edit.bytes_removed = bytes_removed.FromJust();
  edit.bytes_added = bytes_added.FromJust();
  edit.start_point = start_position.FromJust();
  edit.extent_removed = extent_removed.FromJust();
  edit.extent_added = extent_added.FromJust();
  ts_document_edit(document->document_, edit);
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Document::Parse) {
  Document *document = ObjectWrap::Unwrap<Document>(info.This());
  TSRange *ranges;
  uint32_t range_count;
  ts_document_parse_and_get_changed_ranges(document->document_, &ranges, &range_count);

  Local<Array> result = Nan::New<Array>();
  for (size_t i = 0; i < range_count; i++)
    result->Set(i, RangeToJS(ranges[i]));

  info.GetReturnValue().Set(result);
}

NAN_METHOD(Document::Invalidate) {
  Document *document = ObjectWrap::Unwrap<Document>(info.This());
  ts_document_invalidate(document->document_);
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Document::GetLogger) {
  Document *document = ObjectWrap::Unwrap<Document>(info.This());

  TSLogger current_logger = ts_document_logger(document->document_);
  if (current_logger.payload && current_logger.log == Logger::Log) {
    Logger *logger = (Logger *)current_logger.payload;
    info.GetReturnValue().Set(Nan::New(logger->func));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_METHOD(Document::SetLogger) {
  Document *document = ObjectWrap::Unwrap<Document>(info.This());
  Local<Function> func = Local<Function>::Cast(info[0]);

  TSLogger current_logger = ts_document_logger(document->document_);
  if (current_logger.payload)
    delete (Logger *)current_logger.payload;

  if (func->IsFunction()) {
    ts_document_set_logger(document->document_, Logger::Make(func));
  } else {
    ts_document_set_logger(document->document_, { 0, 0 });
    if (!(func->IsNull() || func->IsFalse() || func->IsUndefined())) {
      Nan::ThrowTypeError("Debug callback must either be a function or a falsy value");
      return;
    }
  }

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(Document::PrintDebuggingGraphs) {
  Document *document = ObjectWrap::Unwrap<Document>(info.This());
  Local<Boolean> value = Local<Boolean>::Cast(info[0]);

  if (value->IsBoolean()) {
    ts_document_print_debugging_graphs(document->document_, value->BooleanValue());
  }

  info.GetReturnValue().Set(info.This());
}

}  // namespace node_tree_sitter
