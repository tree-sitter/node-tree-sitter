#ifndef NODE_TREE_SITTER_LOOKAHEAD_ITERATOR_H_
#define NODE_TREE_SITTER_LOOKAHEAD_ITERATOR_H_

#include "tree_sitter/api.h"

#include <nan.h>
#include <node_object_wrap.h>
#include <v8.h>

namespace node_tree_sitter {

class LookaheadIterator final : public Nan::ObjectWrap {
public:
  static void Init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Value> NewInstance(TSLookaheadIterator *);
  static LookaheadIterator *UnwrapLookaheadIterator(const v8::Local<v8::Value> &);

  TSLookaheadIterator *iterator_;

private:
  explicit LookaheadIterator(TSLookaheadIterator *);
  ~LookaheadIterator() final;

  static void New(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void Reset(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void ResetState(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void Next(const Nan::FunctionCallbackInfo<v8::Value> &);
  static void IterNames(const Nan::FunctionCallbackInfo<v8::Value> &);

  static void CurrentSymbol(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);
  static void CurrentSymbolName(v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &);

  static Nan::Persistent<v8::Function> constructor;
  static Nan::Persistent<v8::FunctionTemplate> constructor_template;
};

} // namespace node_tree_sitter



#endif  // NODE_TREE_SITTER_LOOKAHEAD_ITERATOR_H_
