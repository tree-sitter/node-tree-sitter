#include "./node.h"
#include <nan.h>
#include <tree_sitter/runtime.h>
#include <vector>
#include <v8.h>
#include "./util.h"
#include "./conversions.h"
#include "./tree.h"
#include "./tree_cursor.h"

namespace node_tree_sitter {
namespace node_methods {

using std::vector;
using namespace v8;

static const uint32_t FIELD_COUNT_PER_NODE = 6;

static uint32_t *transfer_buffer = NULL;
static uint32_t transfer_buffer_length = 0;
static Nan::Persistent<Object> module_exports;

static inline void setup_transfer_buffer(uint32_t node_count) {
  uint32_t new_length = node_count * FIELD_COUNT_PER_NODE;
  if (new_length > transfer_buffer_length) {
    transfer_buffer_length = new_length;
    transfer_buffer = static_cast<uint32_t *>(malloc(transfer_buffer_length * sizeof(uint32_t)));
    auto js_transfer_buffer = ArrayBuffer::New(Isolate::GetCurrent(), transfer_buffer, transfer_buffer_length * sizeof(uint32_t));
    Nan::New(module_exports)->Set(
      Nan::New("nodeTransferArray").ToLocalChecked(),
      Uint32Array::New(js_transfer_buffer, 0, transfer_buffer_length)
    );
  }
}

static inline bool operator<=(const TSPoint &left, const TSPoint &right) {
  if (left.row < right.row) return true;
  if (left.row > right.row) return false;
  return left.column <= right.column;
}

static void MarshalNodes(const Nan::FunctionCallbackInfo<Value> &info,
                         const Tree *tree, const TSNode *nodes, uint32_t node_count) {
  auto result = Nan::New<Array>();
  setup_transfer_buffer(node_count);
  uint32_t *p = transfer_buffer;
  for (unsigned i = 0; i < node_count; i++) {
    TSNode node = nodes[i];
    const auto &cache_entry = tree->cached_nodes_.find(node.id);
    if (cache_entry == tree->cached_nodes_.end()) {
      MarshalNodeId(node.id, p);
      p += 2;
      *(p++) = node.context[0];
      *(p++) = node.context[1];
      *(p++) = node.context[2];
      *(p++) = node.context[3];
      result->Set(i, Nan::Null());
    } else {
      assert(!cache_entry->second->node.IsNearDeath());
      result->Set(i, Nan::New(cache_entry->second->node));
    }
  }
  info.GetReturnValue().Set(result);
}

void MarshalNode(const Nan::FunctionCallbackInfo<Value> &info, const Tree *tree, TSNode node) {
  const auto &cache_entry = tree->cached_nodes_.find(node.id);
  if (cache_entry == tree->cached_nodes_.end()) {
    setup_transfer_buffer(1);
    uint32_t *p = transfer_buffer;
    MarshalNodeId(node.id, p);
    p += 2;
    *(p++) = node.context[0];
    *(p++) = node.context[1];
    *(p++) = node.context[2];
    *(p++) = node.context[3];
  } else {
    assert(!cache_entry->second->node.IsNearDeath());
    info.GetReturnValue().Set(Nan::New(cache_entry->second->node));
  }
}

static TSNode UnmarshalNode(const Tree *tree) {
  TSNode result = {{0, 0, 0, 0}, nullptr, nullptr};
  result.tree = tree->tree_;
  if (!result.tree) {
    Nan::ThrowTypeError("Argument must be a tree");
    return result;
  }

  result.id = UnmarshalNodeId(&transfer_buffer[0]);
  result.context[0] = transfer_buffer[2];
  result.context[1] = transfer_buffer[3];
  result.context[2] = transfer_buffer[4];
  result.context[3] = transfer_buffer[5];
  return result;
}

static void ToString(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);
  if (node.id) {
    const char *string = ts_node_string(node);
    info.GetReturnValue().Set(Nan::New(string).ToLocalChecked());
    free((char *)string);
  }
}

static void IsMissing(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);
  if (node.id) {
    bool result = ts_node_is_missing(node);
    info.GetReturnValue().Set(Nan::New<Boolean>(result));
  }
}

static void HasChanges(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);
  if (node.id) {
    bool result = ts_node_has_changes(node);
    info.GetReturnValue().Set(Nan::New<Boolean>(result));
  }
}

static void HasError(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);
  if (node.id) {
    bool result = ts_node_has_error(node);
    info.GetReturnValue().Set(Nan::New<Boolean>(result));
  }
}

static void FirstNamedChildForIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);
  if (node.id) {
    Nan::Maybe<uint32_t> byte = ByteCountFromJS(info[1]);
    if (byte.IsJust()) {
      MarshalNode(info, tree, ts_node_first_named_child_for_byte(node, byte.FromJust()));
    }
  }
}

static void FirstChildForIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);

  if (node.id && info.Length() > 1) {
    Nan::Maybe<uint32_t> byte = ByteCountFromJS(info[1]);
    if (byte.IsJust()) {
      MarshalNode(info, tree, ts_node_first_child_for_byte(node, byte.FromJust()));
    }
  }
}

static void NamedDescendantForIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);

  if (node.id) {
    Nan::Maybe<uint32_t> maybe_min = ByteCountFromJS(info[1]);
    Nan::Maybe<uint32_t> maybe_max = ByteCountFromJS(info[2]);
    if (maybe_min.IsJust() && maybe_max.IsJust()) {
      uint32_t min = maybe_min.FromJust();
      uint32_t max = maybe_max.FromJust();
      MarshalNode(info, tree, ts_node_named_descendant_for_byte_range(node, min, max));
    }
  }
}

static void DescendantForIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);

  if (node.id) {
    Nan::Maybe<uint32_t> maybe_min = ByteCountFromJS(info[1]);
    Nan::Maybe<uint32_t> maybe_max = ByteCountFromJS(info[2]);
    if (maybe_min.IsJust() && maybe_max.IsJust()) {
      uint32_t min = maybe_min.FromJust();
      uint32_t max = maybe_max.FromJust();
      MarshalNode(info, tree, ts_node_descendant_for_byte_range(node, min, max));
    }
  }
}

static void NamedDescendantForPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);

  if (node.id) {
    Nan::Maybe<TSPoint> maybe_min = PointFromJS(info[1]);
    Nan::Maybe<TSPoint> maybe_max = PointFromJS(info[2]);
    if (maybe_min.IsJust() && maybe_max.IsJust()) {
      TSPoint min = maybe_min.FromJust();
      TSPoint max = maybe_max.FromJust();
      MarshalNode(info, tree, ts_node_named_descendant_for_point_range(node, min, max));
    }
  }
}

static void DescendantForPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);

  if (node.id) {
    Nan::Maybe<TSPoint> maybe_min = PointFromJS(info[1]);
    Nan::Maybe<TSPoint> maybe_max = PointFromJS(info[2]);
    if (maybe_min.IsJust() && maybe_max.IsJust()) {
      TSPoint min = maybe_min.FromJust();
      TSPoint max = maybe_max.FromJust();
      MarshalNode(info, tree, ts_node_descendant_for_point_range(node, min, max));
    }
  }
}

static void Type(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);

  if (node.id) {
    const char *result = ts_node_type(node);
    info.GetReturnValue().Set(Nan::New(result).ToLocalChecked());
  }
}

static void IsNamed(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);

  if (node.id) {
    bool result = ts_node_is_named(node);
    info.GetReturnValue().Set(Nan::New(result));
  }
}

static void Id(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);

  if (node.id) {
    uint64_t result = reinterpret_cast<uint64_t>(node.id);
    info.GetReturnValue().Set(Nan::New<Number>(result));
  }
}

static void StartIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);

  if (node.id) {
    int32_t result = ts_node_start_byte(node) / 2;
    info.GetReturnValue().Set(Nan::New<Integer>(result));
  }
}

static void EndIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);

  if (node.id) {
    int32_t result = ts_node_end_byte(node) / 2;
    info.GetReturnValue().Set(Nan::New<Integer>(result));
  }
}

static void StartPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);

  if (node.id) {
    TransferPoint(ts_node_start_point(node));
  }
}

static void EndPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);

  if (node.id) {
    TransferPoint(ts_node_end_point(node));
  }
}

static void Child(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);

  if (node.id) {
    if (!info[1]->IsUint32()) {
      Nan::ThrowTypeError("Second argument must be an integer");
      return;
    }
    uint32_t index = info[1]->Uint32Value();
    MarshalNode(info, tree, ts_node_child(node, index));
  }
}

static void NamedChild(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);

  if (node.id) {
    if (!info[1]->IsUint32()) {
      Nan::ThrowTypeError("Second argument must be an integer");
      return;
    }
    uint32_t index = info[1]->Uint32Value();
    MarshalNode(info, tree, ts_node_named_child(node, index));
  }
}

static void ChildCount(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);

  if (node.id) {
    info.GetReturnValue().Set(Nan::New(ts_node_child_count(node)));
  }
}

static void NamedChildCount(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);

  if (node.id) {
    info.GetReturnValue().Set(Nan::New(ts_node_named_child_count(node)));
  }
}

static void FirstChild(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);
  if (node.id) {
    MarshalNode(info, tree, ts_node_child(node, 0));
  }
}

static void FirstNamedChild(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);
  if (node.id) {
    MarshalNode(info, tree, ts_node_named_child(node, 0));
  }
}

static void LastChild(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);
  if (node.id) {
    uint32_t child_count = ts_node_child_count(node);
    if (child_count > 0) {
      MarshalNode(info, tree, ts_node_child(node, child_count - 1));
    }
  }
}

static void LastNamedChild(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);
  if (node.id) {
    uint32_t child_count = ts_node_named_child_count(node);
    if (child_count > 0) {
      MarshalNode(info, tree, ts_node_named_child(node, child_count - 1));
    }
  }
}

static void Parent(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);
  if (node.id) {
    MarshalNode(info, tree, ts_node_parent(node));
  }
}

static void NextSibling(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);
  if (node.id) {
    MarshalNode(info, tree, ts_node_next_sibling(node));
  }
}

static void NextNamedSibling(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);
  if (node.id) {
    MarshalNode(info, tree, ts_node_next_named_sibling(node));
  }
}

static void PreviousSibling(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);
  if (node.id) {
    MarshalNode(info, tree, ts_node_prev_sibling(node));
  }
}

static void PreviousNamedSibling(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);
  if (node.id) {
    MarshalNode(info, tree, ts_node_prev_named_sibling(node));
  }
}

static void DescendantsOfType(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);

  if (!node.id) return;

  if (!info[1]->IsString()) {
    Nan::ThrowTypeError("Node type must be a string");
    return;
  }

  auto js_node_type = Local<String>::Cast(info[1]);
  std::string node_type(js_node_type->Utf8Length() + 1, '\0');
  js_node_type->WriteUtf8(&node_type[0]);

  const TSLanguage *language = ts_tree_language(node.tree);
  TSSymbol symbol = ts_language_symbol_for_name(language, node_type.c_str());
  if (!symbol) {
    Nan::ThrowTypeError("Invalid node type");
    return;
  }

  TSPoint start_point = {0, 0};
  TSPoint end_point = {UINT32_MAX, UINT32_MAX};

  if (info[2]->BooleanValue()) {
    auto maybe_start_point = PointFromJS(info[2]);
    if (maybe_start_point.IsNothing()) return;
    start_point = maybe_start_point.FromJust();
  }

  if (info[3]->BooleanValue()) {
    auto maybe_end_point = PointFromJS(info[3]);
    if (maybe_end_point.IsNothing()) return;
    end_point = maybe_end_point.FromJust();
  }

  vector<TSNode> found;
  TSTreeCursor cursor = ts_tree_cursor_new(node);
  auto already_visited_children = false;
  while (true) {
    TSNode descendant = ts_tree_cursor_current_node(&cursor);

    if (!already_visited_children) {
      if (ts_node_end_point(descendant) <= start_point) {
        if (ts_tree_cursor_goto_next_sibling(&cursor)) {
          already_visited_children = false;
        } else {
          if (!ts_tree_cursor_goto_parent(&cursor)) break;
          already_visited_children = true;
        }
        continue;
      }

      if (end_point <= ts_node_start_point(descendant)) break;

      if (ts_node_symbol(descendant) == symbol) {
        found.push_back(descendant);
      }

      if (ts_tree_cursor_goto_first_child(&cursor)) {
        already_visited_children = false;
      } else if (ts_tree_cursor_goto_next_sibling(&cursor)) {
        already_visited_children = false;
      } else {
        if (!ts_tree_cursor_goto_parent(&cursor)) break;
        already_visited_children = true;
      }
    } else {
      if (ts_tree_cursor_goto_next_sibling(&cursor)) {
        already_visited_children = false;
      } else {
        if (!ts_tree_cursor_goto_parent(&cursor)) break;
      }
    }
  }

  ts_tree_cursor_delete(&cursor);
  MarshalNodes(info, tree, found.data(), found.size());
}

static void Walk(const Nan::FunctionCallbackInfo<Value> &info) {
  const Tree *tree = Tree::UnwrapTree(info[0]);
  TSNode node = UnmarshalNode(tree);
  TSTreeCursor cursor = ts_tree_cursor_new(node);
  info.GetReturnValue().Set(TreeCursor::NewInstance(cursor));
}

void Init(Local<Object> exports) {
  Local<Object> result = Nan::New<Object>();

  FunctionPair methods[] = {
    {"startIndex", StartIndex},
    {"endIndex", EndIndex},
    {"type", Type},
    {"isNamed", IsNamed},
    {"parent", Parent},
    {"child", Child},
    {"namedChild", NamedChild},
    {"childCount", ChildCount},
    {"namedChildCount", NamedChildCount},
    {"firstChild", FirstChild},
    {"lastChild", LastChild},
    {"firstNamedChild", FirstNamedChild},
    {"lastNamedChild", LastNamedChild},
    {"nextSibling", NextSibling},
    {"nextNamedSibling", NextNamedSibling},
    {"previousSibling", PreviousSibling},
    {"previousNamedSibling", PreviousNamedSibling},
    {"id", Id},
    {"startPosition", StartPosition},
    {"endPosition", EndPosition},
    {"isMissing", IsMissing},
    {"toString", ToString},
    {"firstChildForIndex", FirstChildForIndex},
    {"firstNamedChildForIndex", FirstNamedChildForIndex},
    {"descendantForIndex", DescendantForIndex},
    {"namedDescendantForIndex", NamedDescendantForIndex},
    {"descendantForPosition", DescendantForPosition},
    {"namedDescendantForPosition", NamedDescendantForPosition},
    {"hasChanges", HasChanges},
    {"hasError", HasError},
    {"descendantsOfType", DescendantsOfType},
    {"walk", Walk},
  };

  for (size_t i = 0; i < length_of_array(methods); i++) {
    result->Set(
      Nan::New(methods[i].name).ToLocalChecked(),
      Nan::New<FunctionTemplate>(methods[i].callback)->GetFunction()
    );
  }

  module_exports.Reset(exports);
  setup_transfer_buffer(1);

  exports->Set(Nan::New("NodeMethods").ToLocalChecked(), result);
}

}  // namespace node_methods
}  // namespace node_tree_sitter
