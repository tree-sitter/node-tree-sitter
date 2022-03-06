#include "./parser.h"
#include <string>
#include <vector>
#include <climits>
#include <napi.h>
#include "./conversions.h"
#include "./language.h"
#include "./logger.h"
#include "./node.h"
#include "./tree.h"
#include "./util.h"
#include "text-buffer-snapshot-wrapper.h"
#include <cmath>

namespace node_tree_sitter {

using namespace Napi;
using std::vector;
using std::pair;

class Parser : public ObjectWrap<Parser> {
 public:
  static void Init(Object &exports) {
    Napi::Env env = exports.Env();

    Function ctor = DefineClass(env, "Parser", {
      InstanceMethod("getLogger", &Parser::GetLogger, napi_writable),
      InstanceMethod("setLogger", &Parser::SetLogger, napi_writable),
      InstanceMethod("setLanguage", &Parser::SetLanguage, napi_writable),
      InstanceMethod("printDotGraphs", &Parser::PrintDotGraphs, napi_writable),
      InstanceMethod("parse", &Parser::Parse, napi_writable),
      InstanceMethod("parseTextBuffer", &Parser::ParseTextBuffer, napi_writable),
      InstanceMethod("parseTextBufferSync", &Parser::ParseTextBufferSync, napi_writable),
    });

    String s = String::New(env, "");
    if (env.IsExceptionPending()) {
      return;
    }

    napi_value value;
    napi_valuetype type;
    napi_status status = napi_get_property(
      env,
      s,
      String::New(env, "slice"),
      &value
    );
    assert(status == napi_ok);
    status = napi_typeof(env, value, &type);
    assert(status == napi_ok);

    constructor.Reset(ctor, 1);
    constructor.SuppressDestruct(); // statics should not destruct
    // string_slice.Reset(string_slice_fn.As<Function>(), 1);
    exports["Parser"] = ctor;
    exports["LANGUAGE_VERSION"] = Number::New(env, TREE_SITTER_LANGUAGE_VERSION);
  }

  TSParser *parser_;
  bool is_parsing_async_;

  Parser(const CallbackInfo &info)
    : Napi::ObjectWrap<Parser>(info),
      parser_(ts_parser_new()),
      is_parsing_async_(false)
      {}

  ~Parser() { ts_parser_delete(parser_); }

 private:
  class CallbackInput {
   public:
    CallbackInput(Function callback, Napi::Value js_buffer_size)
      : byte_offset(0) {
      this->callback.Reset(callback, 1);
      if (js_buffer_size.IsNumber()) {
        buffer.resize(js_buffer_size.As<Number>().Uint32Value());
      } else {
        buffer.resize(32 * 1024);
      }
    }

    TSInput Input() {
      TSInput result;
      result.payload = (void *)this;
      result.encoding = TSInputEncodingUTF16;
      result.read = Read;
      return result;
    }

   private:
    static String slice(String s, uint32_t offset) {
      return string_slice.Call(s, {Number::New(s.Env(), offset)}).As<String>();
    }

    static const char * Read(void *payload, uint32_t byte, TSPoint position, uint32_t *bytes_read) {
      CallbackInput *reader = (CallbackInput *)payload;
      Napi::Env env = reader->callback.Env();

      if (byte != reader->byte_offset) {
        reader->byte_offset = byte;
        reader->partial_string.Reset();
      }

      *bytes_read = 0;
      String result;
      if (!reader->partial_string.IsEmpty()) {
        result = reader->partial_string.Value().As<String>();
      } else {
        Function callback = reader->callback.Value();
        Napi::Value result_value = callback({
          ByteCountToJS(env, byte),
          PointToJS(env, position),
        });
        if (env.IsExceptionPending()) return nullptr;
        if (!result_value.IsString()) return nullptr;
        result = result_value.As<String>();
      }

      size_t length = 0;
      size_t utf16_units_read = 0;
      napi_status status;
      status = napi_get_value_string_utf16(
        env, result, nullptr, 0, &length
      );
      if (status != napi_ok) return nullptr;
      status = napi_get_value_string_utf16(
        env, result, (char16_t *)&reader->buffer[0], reader->buffer.size(), &utf16_units_read
      );
      if (status != napi_ok) return nullptr;

      *bytes_read = 2 * utf16_units_read;
      reader->byte_offset += *bytes_read;

      if (utf16_units_read < length) {
        reader->partial_string.Reset(slice(result, utf16_units_read));
      } else {
        reader->partial_string.Reset();
      }

      return (const char *)reader->buffer.data();
    }

    FunctionReference callback;
    std::vector<uint16_t> buffer;
    size_t byte_offset;
    Reference<String> partial_string;
  };

  class TextBufferInput {
  public:
    TextBufferInput(const vector<pair<const char16_t *, uint32_t>> *slices)
      : slices_(slices),
        byte_offset(0),
        slice_index_(0),
        slice_offset_(0) {}

    TSInput input() {
      return TSInput{this, Read, TSInputEncodingUTF16};
    }

  private:
    void seek(uint32_t byte_offset) {
      this->byte_offset = byte_offset;

      uint32_t total_length = 0;
      uint32_t goal_index = byte_offset / 2;
      for (unsigned i = 0, n = this->slices_->size(); i < n; i++) {
        uint32_t next_total_length = total_length + this->slices_->at(i).second;
        if (next_total_length > goal_index) {
          this->slice_index_ = i;
          this->slice_offset_ = goal_index - total_length;
          return;
        }
        total_length = next_total_length;
      }

      this->slice_index_ = this->slices_->size();
      this->slice_offset_ = 0;
    }

    static const char *Read(void *payload, uint32_t byte, TSPoint position, uint32_t *length) {
      auto self = static_cast<TextBufferInput *>(payload);

      if (byte != self->byte_offset) self->seek(byte);

      if (self->slice_index_ == self->slices_->size()) {
        *length = 0;
        return "";
      }

      auto &slice = self->slices_->at(self->slice_index_);
      const char16_t *result = slice.first + self->slice_offset_;
      *length = 2 * (slice.second - self->slice_offset_);
      self->byte_offset += *length;
      self->slice_index_++;
      self->slice_offset_ = 0;
      return reinterpret_cast<const char *>(result);
    }

    const vector<pair<const char16_t *, uint32_t>> *slices_;
    uint32_t byte_offset;
    uint32_t slice_index_;
    uint32_t slice_offset_;
  };

  bool handle_included_ranges(class Value arg) {
    Napi::Env env = arg.Env();
    uint32_t last_included_range_end = 0;
    if (arg.IsArray()) {
      Array js_included_ranges = arg.As<Array>();
      vector<TSRange> included_ranges;
      for (unsigned i = 0; i < js_included_ranges.Length(); i++) {
        Napi::Value range_value = js_included_ranges[i];
        if (!range_value.IsObject()) return false;
        optional<TSRange> range = RangeFromJS(range_value);
        if (!range) return false;
        if (range->start_byte < last_included_range_end) {
          RangeError::New(env, "Overlapping ranges").ThrowAsJavaScriptException();
          return false;
        }
        last_included_range_end = range->end_byte;
        included_ranges.push_back(*range);
      }
      ts_parser_set_included_ranges(parser_, included_ranges.data(), included_ranges.size());
    } else {
      ts_parser_set_included_ranges(parser_, nullptr, 0);
    }
    return true;
  }

  Napi::Value SetLanguage(const CallbackInfo &info) {
    auto env = info.Env();
    if (is_parsing_async_) {
      Error::New(env, "Parser is in use").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    const TSLanguage *language = UnwrapLanguage(info[0]);
    if (language) ts_parser_set_language(parser_, language);
    return info.This();
  }

  Napi::Value Parse(const CallbackInfo &info) {
    auto env = info.Env();

    if (is_parsing_async_) {
      Error::New(env, "Parser is in use").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    if (!info[0].IsFunction()) {
      TypeError::New(env, "Input must be a function").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    Function callback = info[0].As<Function>();

    const TSTree *old_tree = nullptr;
    if (info.Length() > 1 && info[1].IsObject()) {
      Object js_old_tree = info[1].As<Object>();
      const Tree *tree = Tree::UnwrapTree(js_old_tree);
      if (!tree) {
        TypeError::New(env, "Second argument must be a tree").ThrowAsJavaScriptException();
        return env.Undefined();
      }
      old_tree = tree->tree_;
    }

    auto buffer_size = env.Null();
    if (info.Length() > 2) buffer_size = info[2];
    if (!handle_included_ranges(info[3])) return env.Undefined();

    CallbackInput callback_input(callback, buffer_size);
    TSTree *tree = ts_parser_parse(parser_, old_tree, callback_input.Input());
    auto result = Tree::NewInstance(env, tree);
    return result;
  }

  class ParseWorker : public Napi::AsyncWorker {
    Parser *parser_;
    TSTree *new_tree_;
    TextBufferInput *input_;

  public:
    ParseWorker(Napi::Function &callback, Parser *parser, TextBufferInput *input) :
      Napi::AsyncWorker(callback, "tree-sitter.parseTextBuffer"),
      parser_(parser),
      new_tree_(nullptr),
      input_(input) {}

    void Execute() {
      TSLogger logger = ts_parser_logger(parser_->parser_);
      ts_parser_set_logger(parser_->parser_, TSLogger{0, 0});
      new_tree_ = ts_parser_parse(parser_->parser_, nullptr, input_->input());
      ts_parser_set_logger(parser_->parser_, logger);
    }

    void OnOK() {
      parser_->is_parsing_async_ = false;
      delete input_;
      Callback()({Tree::NewInstance(Env(), new_tree_)});
    }
  };

  const std::vector<std::pair<const char16_t *, uint32_t>> *
  TextBufferSnapshotFromJS(Napi::Value value) {
    auto env = value.Env();
    if (!value.IsObject()) {
      TypeError::New(env, "Expected a snapshot wrapper").ThrowAsJavaScriptException();
      return nullptr;
    }

    void *internal = GetInternalFieldPointer(value);
    if (internal) {
      return reinterpret_cast<TextBufferSnapshotWrapper *>(internal)->slices();
    } else {
      TypeError::New(env, "Expected a snapshot wrapper").ThrowAsJavaScriptException();
      return nullptr;
    }
  }

  Napi::Value ParseTextBuffer(const CallbackInfo &info) {
    auto env = info.Env();

    if (is_parsing_async_) {
      Error::New(env, "Parser is in use").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    auto callback = info[0].As<Function>();

    const TSTree *old_tree = nullptr;
    if (info.Length() > 2 && info[2].IsObject()) {
      Object js_old_tree = info[2].As<Object>();
      const Tree *tree = Tree::UnwrapTree(js_old_tree);
      if (!tree) {
        TypeError::New(env, "Second argument must be a tree").ThrowAsJavaScriptException();
        return env.Undefined();
      }
      old_tree = tree->tree_;
    }

    if (!handle_included_ranges(info[3])) return env.Undefined();

    auto snapshot = TextBufferSnapshotFromJS(info[1]);
    if (!snapshot) return env.Undefined();

    auto input = new TextBufferInput(snapshot);

    // If a `syncTimeoutMicros` option is passed, parse synchronously
    // for the given amount of time before queuing an async task.
    auto js_sync_timeout = info[4];
    if (!js_sync_timeout.IsEmpty() && !js_sync_timeout.IsNull() && !js_sync_timeout.IsUndefined()) {
      size_t sync_timeout;

      // If the timeout is `Infinity`, then parse synchronously with no timeout.
      if (!std::isfinite(js_sync_timeout.ToNumber().DoubleValue())) {
        sync_timeout = 0;
      } else if (js_sync_timeout.IsNumber()) {
        sync_timeout = js_sync_timeout.As<Number>().Uint32Value();
      } else {
        TypeError::New(env, "The `syncTimeoutMicros` option must be a positive integer.").ThrowAsJavaScriptException();
        return env.Undefined();
      }

      // Logging is disabled for this method, because we can't call the
      // logging callback from an async worker.
      TSLogger logger = ts_parser_logger(parser_);
      ts_parser_set_timeout_micros(parser_, sync_timeout);
      ts_parser_set_logger(parser_, TSLogger{0, 0});
      TSTree *result = ts_parser_parse(parser_, old_tree, input->input());
      ts_parser_set_timeout_micros(parser_, 0);
      ts_parser_set_logger(parser_, logger);

      if (result) {
        delete input;
        callback({Tree::NewInstance(env, result)});
        return env.Undefined();
      }
    }

    is_parsing_async_ = true;
    auto worker = new ParseWorker(callback, this, input);
    worker->Queue();
    return env.Undefined();
  }

  Napi::Value ParseTextBufferSync(const CallbackInfo &info) {
    auto env = info.Env();

    if (is_parsing_async_) {
      Error::New(env, "Parser is in use").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    const TSTree *old_tree = nullptr;
    if (info.Length() > 1 && info[1].IsObject()) {
      const Tree *tree = Tree::UnwrapTree(info[1].As<Object>());
      if (!tree) {
        TypeError::New(env, "Second argument must be a tree").ThrowAsJavaScriptException();
        return env.Undefined();
      }
      old_tree = ts_tree_copy(tree->tree_);
    }

    if (!handle_included_ranges(info[2])) return env.Undefined();

    auto snapshot = TextBufferSnapshotFromJS(info[0]);
    if (!snapshot) return env.Undefined();

    TextBufferInput input(snapshot);
    TSTree *result = ts_parser_parse(parser_, old_tree, input.input());
    return Tree::NewInstance(env, result);
  }

  Napi::Value GetLogger(const CallbackInfo &info) {
    auto env = info.Env();
    TSLogger current_logger = ts_parser_logger(parser_);
    if (current_logger.payload && current_logger.log == Logger::Log) {
      Logger *logger = (Logger *)current_logger.payload;
      return logger->func.Value();
    } else {
      return env.Null();
    }
  }

  Napi::Value SetLogger(const CallbackInfo &info) {
    auto env = info.Env();

    if (is_parsing_async_) {
      Error::New(env, "Parser is in use").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    TSLogger current_logger = ts_parser_logger(parser_);

    if (info[0].IsFunction()) {
      if (current_logger.payload) delete (Logger *)current_logger.payload;
      ts_parser_set_logger(parser_, Logger::Make(info[0].As<Function>()));
    } else if (info[0].IsEmpty() || info[0].IsNull() || (info[0].IsBoolean() && !info[0].As<Boolean>())) {
      if (current_logger.payload) delete (Logger *)current_logger.payload;
      ts_parser_set_logger(parser_, { 0, 0 });
    } else {
      TypeError::New(env, "Logger callback must either be a function or a falsy value").ThrowAsJavaScriptException();
    }

    return info.This();
  }

  Napi::Value PrintDotGraphs(const CallbackInfo &info) {
    auto env = info.Env();

    if (is_parsing_async_) {
      Error::New(env, "Parser is in use").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    if (info[0].IsBoolean() && info[0].As<Boolean>()) {
      ts_parser_print_dot_graphs(parser_, 2);
    } else {
      ts_parser_print_dot_graphs(parser_, -1);
    }

    return info.This();
  }

  static Napi::FunctionReference constructor;
  static Napi::FunctionReference string_slice;
};

void InitParser(Object &exports) {
  Parser::Init(exports);
}

Napi::FunctionReference Parser::constructor;
Napi::FunctionReference Parser::string_slice;

}  // namespace node_tree_sitter
