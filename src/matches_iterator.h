#ifndef NODE_TREE_SITTER_MATCHES_ITERATOR_H_
#define NODE_TREE_SITTER_MATCHES_ITERATOR_H_

#include <napi.h>
#include "tree_sitter/api.h"

namespace node_tree_sitter {

class MatchesIterator final : public Napi::ObjectWrap<MatchesIterator> {
 public:
  static void Init(Napi::Env env, Napi::Object exports);

  explicit MatchesIterator(const Napi::CallbackInfo &info);
  ~MatchesIterator() final;

 private:
  Napi::Reference<Napi::Value> query_;
  Napi::Reference<Napi::Value> tree_;
  TSQueryCursor *query_cursor_ = nullptr;

  Napi::Value Iterator(const Napi::CallbackInfo &info);
  Napi::Value Next(const Napi::CallbackInfo &info);
  Napi::Value GetQuery(const Napi::CallbackInfo &info);
  Napi::Value GetTree(const Napi::CallbackInfo &info);
};

} // namespace node_tree_sitter

#endif // NODE_TREE_SITTER_MATCHES_ITERATOR_H_
