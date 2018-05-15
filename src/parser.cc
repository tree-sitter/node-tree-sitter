#include "./parser.h"
#include <string>
#include <vector>
#include <v8.h>
#include <nan.h>
#include "./conversions.h"
#include "./input_reader.h"
#include "./logger.h"
#include "./tree.h"
#include "./util.h"
#include "text-buffer-snapshot-wrapper.h"

namespace node_tree_sitter {

using namespace v8;
using std::vector;
using std::pair;

Nan::Persistent<Function> Parser::constructor;

void Parser::Init(Local<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Local<String> class_name = Nan::New("Parser").ToLocalChecked();
  tpl->SetClassName(class_name);

  FunctionPair methods[] = {
    {"getLogger", GetLogger},
    {"setLogger", SetLogger},
    {"setLanguage", SetLanguage},
    {"printDotGraphs", PrintDotGraphs},
    {"parse", Parse},
    {"parseTextBuffer", ParseTextBuffer},
  };

  for (size_t i = 0; i < length_of_array(methods); i++) {
    Nan::SetPrototypeMethod(tpl, methods[i].name, methods[i].callback);
  }

  constructor.Reset(Nan::Persistent<Function>(tpl->GetFunction()));
  exports->Set(class_name, Nan::New(constructor));
  exports->Set(Nan::New("LANGUAGE_VERSION").ToLocalChecked(), Nan::New<Number>(TREE_SITTER_LANGUAGE_VERSION));
}

Parser::Parser() : parser_(ts_parser_new()), is_parsing_async_(false) {}

Parser::~Parser() { ts_parser_delete(parser_); }

void Parser::New(const Nan::FunctionCallbackInfo<Value> &info) {
  if (info.IsConstructCall()) {
    Parser *parser = new Parser();
    parser->Wrap(info.This());
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

void Parser::SetLanguage(const Nan::FunctionCallbackInfo<Value> &info) {
  Parser *parser = ObjectWrap::Unwrap<Parser>(info.This());
  if (parser->is_parsing_async_) {
    Nan::ThrowError("Parser is in use");
    return;
  }

  if (!info[0]->IsObject()) {
    Nan::ThrowTypeError("Invalid language object");
    return;
  }

  Local<Object> arg = Local<Object>::Cast(info[0]);
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

  ts_parser_set_language(parser->parser_, language);

  info.GetReturnValue().Set(info.This());
}

void Parser::Parse(const Nan::FunctionCallbackInfo<Value> &info) {
  Parser *parser = ObjectWrap::Unwrap<Parser>(info.This());
  if (parser->is_parsing_async_) {
    Nan::ThrowError("Parser is in use");
    return;
  }

  if (!info[0]->IsObject()) {
    Nan::ThrowTypeError("Input must be an object");
    return;
  }

  Local<Object> input = Local<Object>::Cast(info[0]);

  if (!input->Get(Nan::New("seek").ToLocalChecked())->IsFunction()) {
    Nan::ThrowTypeError("Input must implement seek(n)");
    return;
  }

  if (!input->Get(Nan::New("read").ToLocalChecked())->IsFunction()) {
    Nan::ThrowTypeError("Input must implement read(n)");
    return;
  }

  const TSTree *old_tree = nullptr;
  if (info.Length() > 1 && info[1]->BooleanValue()) {
    const TSTree *tree = Tree::UnwrapTree(info[1]);
    if (!tree) {
      Nan::ThrowTypeError("Second argument must be a tree");
      return;
    }
    old_tree = tree;
  }

  InputReader input_reader(input);
  TSTree *tree = ts_parser_parse(parser->parser_, old_tree, input_reader.Input());

  Local<Value> result = Tree::NewInstance(tree);
  info.GetReturnValue().Set(result);
}

class ParseWorker : public Nan::AsyncWorker {
  Parser *parser_;
  const vector<pair<const char16_t *, uint32_t>> *slices_;
  TSTree *old_tree_;
  TSTree *result_;
  uint32_t slice_index_;
  uint32_t slice_offset_;

  static int Seek(void *payload, uint32_t byte, TSPoint position) {
    ParseWorker *worker = static_cast<ParseWorker *>(payload);

    uint32_t total_length = 0;
    uint32_t goal_index = byte / 2;
    for (unsigned i = 0, n = worker->slices_->size(); i < n; i++) {
      uint32_t next_total_length = total_length + worker->slices_->at(i).second;
      if (next_total_length > goal_index) {
        worker->slice_index_ = i;
        worker->slice_offset_ = goal_index - total_length;
        return true;
      }
      total_length = next_total_length;
    }

    worker->slice_index_ = worker->slices_->size();
    worker->slice_offset_ = 0;
    return true;
  }

  static const char *Read(void *payload, uint32_t *length) {
    ParseWorker *worker = static_cast<ParseWorker *>(payload);

    if (worker->slice_index_ == worker->slices_->size()) {
      *length = 0;
      return "";
    }

    auto &slice = worker->slices_->at(worker->slice_index_);
    const char16_t *result = slice.first + worker->slice_offset_;
    *length = 2 * (slice.second - worker->slice_offset_);
    worker->slice_index_++;
    worker->slice_offset_ = 0;
    return reinterpret_cast<const char *>(result);
  }

public:
  ParseWorker(Nan::Callback *callback, Parser *parser,
              const vector<pair<const char16_t *, uint32_t>> *slices, TSTree *old_tree) :
    AsyncWorker(callback),
    parser_(parser),
    slices_(slices),
    old_tree_(old_tree),
    result_(nullptr) {}

  void Execute() {
    TSInput input = {
      this,
      Read,
      Seek,
      TSInputEncodingUTF16,
    };
    result_ = ts_parser_parse(parser_->parser_, old_tree_, input);
  }

  void HandleOKCallback() {
    parser_->is_parsing_async_ = false;
    if (old_tree_) ts_tree_delete(old_tree_);
    Local<Value> argv[] = {Tree::NewInstance(result_)};
    callback->Call(1, argv);
  }
};

void Parser::ParseTextBuffer(const Nan::FunctionCallbackInfo<Value> &info) {
  Parser *parser = ObjectWrap::Unwrap<Parser>(info.This());
  if (parser->is_parsing_async_) {
    Nan::ThrowError("Parser is in use");
    return;
  }

  auto callback = new Nan::Callback(info[0].As<Function>());
  auto snapshot = Nan::ObjectWrap::Unwrap<TextBufferSnapshotWrapper>(info[1].As<Object>());

  parser->is_parsing_async_ = true;

  TSTree *old_tree = nullptr;
  if (info.Length() > 2 && info[2]->BooleanValue()) {
    const TSTree *tree = Tree::UnwrapTree(info[2]);
    if (!tree) {
      Nan::ThrowTypeError("Second argument must be a tree");
      return;
    }
    old_tree = ts_tree_copy(tree);
  }

  Nan::AsyncQueueWorker(new ParseWorker(
    callback,
    parser,
    snapshot->slices(),
    old_tree
  ));
}

void Parser::GetLogger(const Nan::FunctionCallbackInfo<Value> &info) {
  Parser *parser = ObjectWrap::Unwrap<Parser>(info.This());

  TSLogger current_logger = ts_parser_logger(parser->parser_);
  if (current_logger.payload && current_logger.log == Logger::Log) {
    Logger *logger = (Logger *)current_logger.payload;
    info.GetReturnValue().Set(Nan::New(logger->func));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

void Parser::SetLogger(const Nan::FunctionCallbackInfo<Value> &info) {
  Parser *parser = ObjectWrap::Unwrap<Parser>(info.This());
  if (parser->is_parsing_async_) {
    Nan::ThrowError("Parser is in use");
    return;
  }

  TSLogger current_logger = ts_parser_logger(parser->parser_);

  if (info[0]->IsFunction()) {
    if (current_logger.payload) delete (Logger *)current_logger.payload;
    ts_parser_set_logger(parser->parser_, Logger::Make(Local<Function>::Cast(info[0])));
  } else if (!info[0]->BooleanValue()) {
    if (current_logger.payload) delete (Logger *)current_logger.payload;
    ts_parser_set_logger(parser->parser_, { 0, 0 });
  } else {
    Nan::ThrowTypeError("Logger callback must either be a function or a falsy value");
    return;
  }

  info.GetReturnValue().Set(info.This());
}

void Parser::PrintDotGraphs(const Nan::FunctionCallbackInfo<Value> &info) {
  Parser *parser = ObjectWrap::Unwrap<Parser>(info.This());
  if (parser->is_parsing_async_) {
    Nan::ThrowError("Parser is in use");
    return;
  }

  Local<Boolean> value = Local<Boolean>::Cast(info[0]);

  if (value->IsBoolean() && value->BooleanValue()) {
    ts_parser_print_dot_graphs(parser->parser_, stderr);
  } else {
    ts_parser_print_dot_graphs(parser->parser_, nullptr);
  }

  info.GetReturnValue().Set(info.This());
}

}  // namespace node_tree_sitter
