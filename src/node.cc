#include "./node.h"
#include <tree_sitter/api.h>
#include <napi.h>
#include <vector>
#include "./util.h"
#include "./conversions.h"
#include "./tree.h"
#include "./tree_cursor.h"

namespace node_tree_sitter {

using std::vector;
using namespace Napi;

static const uint32_t FIELD_COUNT_PER_NODE = 6;

static uint32_t *transfer_buffer = nullptr;
static uint32_t transfer_buffer_length = 0;
static ObjectReference module_exports;
static TSTreeCursor scratch_cursor = {nullptr, nullptr, {0, 0}};

static inline void setup_transfer_buffer(Env env, uint32_t node_count) {
  uint32_t new_length = node_count * FIELD_COUNT_PER_NODE;
  if (new_length > transfer_buffer_length) {
    if (transfer_buffer) {
      free(transfer_buffer);
    }
    transfer_buffer_length = new_length;
    transfer_buffer = static_cast<uint32_t *>(malloc(transfer_buffer_length * sizeof(uint32_t)));
    auto js_transfer_buffer = ArrayBuffer::New(
      env,
      transfer_buffer,
      transfer_buffer_length * sizeof(uint32_t)
    );
    module_exports.Value()["nodeTransferArray"] = Uint32Array::New(
      env,
      transfer_buffer_length,
      js_transfer_buffer,
      0
    );
  }
}

static inline bool operator<=(const TSPoint &left, const TSPoint &right) {
  if (left.row < right.row) return true;
  if (left.row > right.row) return false;
  return left.column <= right.column;
}


static Value MarshalNodes(
  Env env,
  const Tree *tree,
  const TSNode *nodes,
  uint32_t node_count
) {
  Array result = Array::New(env);
  setup_transfer_buffer(env, node_count);
  uint32_t *p = transfer_buffer;
  for (unsigned i = 0; i < node_count; i++) {
    TSNode node = nodes[i];
    const auto &cache_entry = tree->cached_nodes_.find(node.id);
    if (cache_entry == tree->cached_nodes_.end()) {
      MarshalPointer(node.id, p);
      p += 2;
      *(p++) = node.context[0];
      *(p++) = node.context[1];
      *(p++) = node.context[2];
      *(p++) = node.context[3];
      if (node.id) {
        result[i] = Number::New(env, ts_node_symbol(node));
      } else {
        result[i] = env.Null();
      }
    } else {
      result[i] = cache_entry->second->node.Value();
    }
  }
  return result;
}

Value MarshalNode(
  Env env,
  const Tree *tree,
  TSNode node
) {
  const auto &cache_entry = tree->cached_nodes_.find(node.id);
  if (cache_entry == tree->cached_nodes_.end()) {
    setup_transfer_buffer(env, 1);
    uint32_t *p = transfer_buffer;
    MarshalPointer(node.id, p);
    p += 2;
    *(p++) = node.context[0];
    *(p++) = node.context[1];
    *(p++) = node.context[2];
    *(p++) = node.context[3];
    if (node.id) {
      return Number::New(env, ts_node_symbol(node));
    } else {
      return env.Null();
    }
  } else {
    return cache_entry->second->node.Value();
  }
  return env.Null();
}

Value MarshalNullNode(Env env) {
  memset(transfer_buffer, 0, FIELD_COUNT_PER_NODE * sizeof(transfer_buffer[0]));
  return env.Null();
}

TSNode UnmarshalNode(Env env, const Tree *tree) {
  TSNode result = {{0, 0, 0, 0}, nullptr, nullptr};
  if (!tree) {
    TypeError::New(env, "Argument must be a tree").ThrowAsJavaScriptException();
    return result;
  }

  result.tree = tree->tree_;
  result.id = UnmarshalPointer(&transfer_buffer[0]);
  result.context[0] = transfer_buffer[2];
  result.context[1] = transfer_buffer[3];
  result.context[2] = transfer_buffer[4];
  result.context[3] = transfer_buffer[5];
  return result;
}

static Value ToString(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    char *string = ts_node_string(node);
    String result = String::New(env, string);
    free(string);
    return result;
  }
  return env.Undefined();
}

static Value IsMissing(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    bool result = ts_node_is_missing(node);
    return Boolean::New(env, result);
  }
  return env.Undefined();
}

static Value HasChanges(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    bool result = ts_node_has_changes(node);
    return Boolean::New(env, result);
  }
  return env.Undefined();
}

static Value HasError(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    bool result = ts_node_has_error(node);
    return Boolean::New(env, result);
  }
  return env.Undefined();
}

static Value FirstNamedChildForIndex(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    auto byte = ByteCountFromJS(info[1]);
    if (byte) {
      return MarshalNode(env, tree, ts_node_first_named_child_for_byte(node, *byte));
    }
  }
  return MarshalNullNode(env);
}

static Value FirstChildForIndex(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);

  if (node.id && info.Length() > 1) {
    optional<uint32_t> byte = ByteCountFromJS(info[1]);
    if (byte) {
      return MarshalNode(env, tree, ts_node_first_child_for_byte(node, *byte));
    }
  }
  return MarshalNullNode(env);
}

static Value NamedDescendantForIndex(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);

  if (node.id) {
    optional<uint32_t> maybe_min = ByteCountFromJS(info[1]);
    if (maybe_min) {
      optional<uint32_t> maybe_max = ByteCountFromJS(info[2]);
      if (maybe_max) {
        uint32_t min = *maybe_min;
        uint32_t max = *maybe_max;
        return MarshalNode(env, tree, ts_node_named_descendant_for_byte_range(node, min, max));
      }
    }
  }
  return MarshalNullNode(env);
}

static Value DescendantForIndex(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);

  if (node.id) {
    optional<uint32_t> maybe_min = ByteCountFromJS(info[1]);
    if (maybe_min) {
      optional<uint32_t> maybe_max = ByteCountFromJS(info[2]);
      if (maybe_max) {
        uint32_t min = *maybe_min;
        uint32_t max = *maybe_max;
        return MarshalNode(env, tree, ts_node_descendant_for_byte_range(node, min, max));
      }
    }
  }
  return MarshalNullNode(env);
}

static Value NamedDescendantForPosition(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);

  if (node.id) {
    optional<TSPoint> maybe_min = PointFromJS(info[1]);
    optional<TSPoint> maybe_max = PointFromJS(info[2]);
    if (maybe_min && maybe_max) {
      TSPoint min = *maybe_min;
      TSPoint max = *maybe_max;
      return MarshalNode(env, tree, ts_node_named_descendant_for_point_range(node, min, max));
    }
  }
  return MarshalNullNode(env);
}

static Value DescendantForPosition(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);

  if (node.id) {
    optional<TSPoint> maybe_min = PointFromJS(info[1]);
    if (maybe_min) {
      optional<TSPoint> maybe_max = PointFromJS(info[2]);
      if (maybe_max) {
        TSPoint min = *maybe_min;
        TSPoint max = *maybe_max;
        return MarshalNode(env, tree, ts_node_descendant_for_point_range(node, min, max));
      }
    }
  }
  return MarshalNullNode(env);
}

static Value Type(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);

  if (node.id) {
    const char *result = ts_node_type(node);
    return String::New(env, result);
  }
  return env.Undefined();
}

static Value TypeId(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);

  if (node.id) {
    TSSymbol result = ts_node_symbol(node);
    return Number::New(env, result);
  }
  return env.Undefined();
}

static Value IsNamed(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);

  if (node.id) {
    bool result = ts_node_is_named(node);
    return Boolean::New(env, result);
  }
  return env.Undefined();
}

static Value StartIndex(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);

  if (node.id) {
    uint32_t result = ts_node_start_byte(node) / 2;
    return Number::New(env, result);
  }
  return env.Undefined();
}

static Value EndIndex(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);

  if (node.id) {
    uint32_t result = ts_node_end_byte(node) / 2;
    return Number::New(env, result);
  }
  return env.Undefined();
}

static Value StartPosition(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    TransferPoint(ts_node_start_point(node));
  }
  return env.Undefined();
}

static Value EndPosition(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    TransferPoint(ts_node_end_point(node));
  }
  return env.Undefined();
}

static Value Child(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    if (info[1].IsNumber()) {
      uint32_t index = info[1].As<Number>().Uint32Value();
      return MarshalNode(env, tree, ts_node_child(node, index));
    }
    TypeError::New(env, "Second argument must be an integer").ThrowAsJavaScriptException();
  }
  return MarshalNullNode(env);
}

static Value NamedChild(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    if (info[1].IsNumber()) {
      uint32_t index = info[1].As<Number>().Uint32Value();
      return MarshalNode(env, tree, ts_node_named_child(node, index));
    }
    TypeError::New(env, "Second argument must be an integer").ThrowAsJavaScriptException();
  }
  return MarshalNullNode(env);
}

static Value ChildCount(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    return Number::New(env, ts_node_child_count(node));
  }
  return env.Undefined();
}

static Value NamedChildCount(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    return Number::New(env, ts_node_named_child_count(node));
  }
  return env.Undefined();
}

static Value FirstChild(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    return MarshalNode(env, tree, ts_node_child(node, 0));
  }
  return MarshalNullNode(env);
}

static Value FirstNamedChild(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    return MarshalNode(env, tree, ts_node_named_child(node, 0));
  }
  return MarshalNullNode(env);
}

static Value LastChild(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    uint32_t child_count = ts_node_child_count(node);
    if (child_count > 0) {
      return MarshalNode(env, tree, ts_node_child(node, child_count - 1));
    }
  }
  return MarshalNullNode(env);
}

static Value LastNamedChild(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    uint32_t child_count = ts_node_named_child_count(node);
    if (child_count > 0) {
      return MarshalNode(env, tree, ts_node_named_child(node, child_count - 1));
    }
  }
  return MarshalNullNode(env);
}

static Value Parent(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    return MarshalNode(env, tree, ts_node_parent(node));
  }
  return MarshalNullNode(env);
}

static Value NextSibling(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    return MarshalNode(env, tree, ts_node_next_sibling(node));
  }
  return MarshalNullNode(env);
}

static Value NextNamedSibling(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    return MarshalNode(env, tree, ts_node_next_named_sibling(node));
  }
  return MarshalNullNode(env);
}

static Value PreviousSibling(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    return MarshalNode(env, tree, ts_node_prev_sibling(node));
  }
  return MarshalNullNode(env);
}

static Value PreviousNamedSibling(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (node.id) {
    return MarshalNode(env, tree, ts_node_prev_named_sibling(node));
  }
  return MarshalNullNode(env);
}

struct SymbolSet {
  std::basic_string<TSSymbol> symbols;
  void add(TSSymbol symbol) { symbols += symbol; }
  bool contains(TSSymbol symbol) { return symbols.find(symbol) != symbols.npos; }
};

bool symbol_set_from_js(SymbolSet *symbols, const Value &value, const TSLanguage *language) {
  Env env = value.Env();
  if (!value.IsArray()) {
    TypeError::New(env, "Argument must be a string or array of strings").ThrowAsJavaScriptException();
    return false;
  }
  Array js_types = value.As<Array>();
  unsigned symbol_count = ts_language_symbol_count(language);
  for (uint32_t i = 0, n = js_types.Length(); i < n; i++) {
    Value js_node_type_value = js_types[i];
    if (js_node_type_value.IsString()) {
      String js_node_type = js_node_type_value.As<String>();
      std::string node_type = js_node_type.Utf8Value();
      if (node_type == "ERROR") {
        symbols->add(static_cast<TSSymbol>(-1));
      } else {
        for (TSSymbol j = 0; j < symbol_count; j++) {
          if (node_type == ts_language_symbol_name(language, j)) {
            symbols->add(j);
          }
        }
      }
    } else {
      TypeError::New(env, "Argument must be a string or array of strings").ThrowAsJavaScriptException();
      return false;
    }
  }
  return true;
}

static Value Children(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (!node.id) return env.Undefined();

  vector<TSNode> result;
  ts_tree_cursor_reset(&scratch_cursor, node);
  if (ts_tree_cursor_goto_first_child(&scratch_cursor)) {
    do {
      TSNode child = ts_tree_cursor_current_node(&scratch_cursor);
      result.push_back(child);
    } while (ts_tree_cursor_goto_next_sibling(&scratch_cursor));
  }

  return MarshalNodes(env, tree, result.data(), result.size());
}

static Value NamedChildren(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (!node.id) return env.Undefined();

  vector<TSNode> result;
  ts_tree_cursor_reset(&scratch_cursor, node);
  if (ts_tree_cursor_goto_first_child(&scratch_cursor)) {
    do {
      TSNode child = ts_tree_cursor_current_node(&scratch_cursor);
      if (ts_node_is_named(child)) {
        result.push_back(child);
      }
    } while (ts_tree_cursor_goto_next_sibling(&scratch_cursor));
  }

  return MarshalNodes(env, tree, result.data(), result.size());
}

static Value DescendantsOfType(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (!node.id) return env.Undefined();

  SymbolSet symbols;
  if (!symbol_set_from_js(&symbols, info[1], ts_tree_language(node.tree))) {
    return env.Undefined();
  }

  TSPoint start_point = {0, 0};
  TSPoint end_point = {UINT32_MAX, UINT32_MAX};
  if (info.Length() > 2 && info[2].IsObject()) {
    auto maybe_start_point = PointFromJS(info[2]);
    if (!maybe_start_point) return env.Undefined();
    start_point = *maybe_start_point;
  }

  if (info.Length() > 3 && info[3].IsObject()) {
    auto maybe_end_point = PointFromJS(info[3]);
    if (!maybe_end_point) return env.Undefined();
    end_point = *maybe_end_point;
  }

  vector<TSNode> found;
  ts_tree_cursor_reset(&scratch_cursor, node);
  auto already_visited_children = false;
  while (true) {
    TSNode descendant = ts_tree_cursor_current_node(&scratch_cursor);

    if (!already_visited_children) {
      if (ts_node_end_point(descendant) <= start_point) {
        if (ts_tree_cursor_goto_next_sibling(&scratch_cursor)) {
          already_visited_children = false;
        } else {
          if (!ts_tree_cursor_goto_parent(&scratch_cursor)) break;
          already_visited_children = true;
        }
        continue;
      }

      if (end_point <= ts_node_start_point(descendant)) break;

      if (symbols.contains(ts_node_symbol(descendant))) {
        found.push_back(descendant);
      }

      if (ts_tree_cursor_goto_first_child(&scratch_cursor)) {
        already_visited_children = false;
      } else if (ts_tree_cursor_goto_next_sibling(&scratch_cursor)) {
        already_visited_children = false;
      } else {
        if (!ts_tree_cursor_goto_parent(&scratch_cursor)) break;
        already_visited_children = true;
      }
    } else {
      if (ts_tree_cursor_goto_next_sibling(&scratch_cursor)) {
        already_visited_children = false;
      } else {
        if (!ts_tree_cursor_goto_parent(&scratch_cursor)) break;
      }
    }
  }

  return MarshalNodes(env, tree, found.data(), found.size());
}

static Value ChildNodesForFieldId(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (!node.id) return env.Undefined();

  if (!info[1].IsNumber()) {
    TypeError::New(env, "Second argument must be an integer").ThrowAsJavaScriptException();
    return env.Undefined();
  }
  uint32_t field_id = info[1].As<Number>().Uint32Value();

  vector<TSNode> result;
  ts_tree_cursor_reset(&scratch_cursor, node);
  if (ts_tree_cursor_goto_first_child(&scratch_cursor)) {
    do {
      TSNode child = ts_tree_cursor_current_node(&scratch_cursor);
      if (ts_tree_cursor_current_field_id(&scratch_cursor) == field_id) {
        result.push_back(child);
      }
    } while (ts_tree_cursor_goto_next_sibling(&scratch_cursor));
  }

  return MarshalNodes(env, tree, result.data(), result.size());
}

static Value ChildNodeForFieldId(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);

  if (node.id) {
    if (!info[1].IsNumber()) {
      TypeError::New(env, "Second argument must be an integer").ThrowAsJavaScriptException();
      return env.Undefined();
    }
    uint32_t field_id = info[1].As<Number>().Uint32Value();
    return MarshalNode(env, tree, ts_node_child_by_field_id(node, field_id));
  }
  return MarshalNullNode(env);
}

static Value Closest(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  if (!node.id) return env.Undefined();

  SymbolSet symbols;
  if (!symbol_set_from_js(&symbols, info[1], ts_tree_language(node.tree))) {
    return env.Undefined();
  }

  for (;;) {
    TSNode parent = ts_node_parent(node);
    if (!parent.id) break;
    if (symbols.contains(ts_node_symbol(parent))) {
      return MarshalNode(env, tree, parent);
    }
    node = parent;
  }

  return MarshalNullNode(env);
}

static Value Walk(const CallbackInfo &info) {
  Env env = info.Env();
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(env, tree);
  TSTreeCursor cursor = ts_tree_cursor_new(node);
  return NewTreeCursor(cursor);
}

class NodeMethods : public ObjectWrap<NodeMethods> {
  public:
  NodeMethods(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<NodeMethods>(info)
    {}

  static void Init(Napi::Env env, Object &exports) {
    exports["NodeMethods"] = DefineClass(env, "NodeMethods", {
      StaticMethod("startIndex", StartIndex, napi_writable),
      StaticMethod("endIndex", EndIndex, napi_writable),
      StaticMethod("type", Type, napi_writable),
      StaticMethod("typeId", TypeId, napi_writable),
      StaticMethod("isNamed", IsNamed, napi_writable),
      StaticMethod("parent", Parent, napi_writable),
      StaticMethod("child", Child, napi_writable),
      StaticMethod("namedChild", NamedChild, napi_writable),
      StaticMethod("children", Children, napi_writable),
      StaticMethod("namedChildren", NamedChildren, napi_writable),
      StaticMethod("childCount", ChildCount, napi_writable),
      StaticMethod("namedChildCount", NamedChildCount, napi_writable),
      StaticMethod("firstChild", FirstChild, napi_writable),
      StaticMethod("lastChild", LastChild, napi_writable),
      StaticMethod("firstNamedChild", FirstNamedChild, napi_writable),
      StaticMethod("lastNamedChild", LastNamedChild, napi_writable),
      StaticMethod("nextSibling", NextSibling, napi_writable),
      StaticMethod("nextNamedSibling", NextNamedSibling, napi_writable),
      StaticMethod("previousSibling", PreviousSibling, napi_writable),
      StaticMethod("previousNamedSibling", PreviousNamedSibling, napi_writable),
      StaticMethod("startPosition", StartPosition, napi_writable),
      StaticMethod("endPosition", EndPosition, napi_writable),
      StaticMethod("isMissing", IsMissing, napi_writable),
      StaticMethod("toString", ToString, napi_writable),
      StaticMethod("firstChildForIndex", FirstChildForIndex, napi_writable),
      StaticMethod("firstNamedChildForIndex", FirstNamedChildForIndex, napi_writable),
      StaticMethod("descendantForIndex", DescendantForIndex, napi_writable),
      StaticMethod("namedDescendantForIndex", NamedDescendantForIndex, napi_writable),
      StaticMethod("descendantForPosition", DescendantForPosition, napi_writable),
      StaticMethod("namedDescendantForPosition", NamedDescendantForPosition, napi_writable),
      StaticMethod("hasChanges", HasChanges, napi_writable),
      StaticMethod("hasError", HasError, napi_writable),
      StaticMethod("descendantsOfType", DescendantsOfType, napi_writable),
      StaticMethod("walk", Walk, napi_writable),
      StaticMethod("closest", Closest, napi_writable),
      StaticMethod("childNodeForFieldId", ChildNodeForFieldId, napi_writable),
      StaticMethod("childNodesForFieldId", ChildNodesForFieldId, napi_writable),
    });

  }
};

void InitNode(Object &exports) {
  Env env = exports.Env();
  NodeMethods::Init(env, exports);
  module_exports.Reset(exports, 1);
  setup_transfer_buffer(env, 1);
}

}  // namespace node_tree_sitter
