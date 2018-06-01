#include "./node.h"
#include <nan.h>
#include <tree_sitter/runtime.h>
#include <v8.h>
#include "./util.h"
#include "./conversions.h"
#include "./tree.h"

namespace node_tree_sitter {

using namespace v8;

static uint32_t *transfer_buffer;

void Node::Init(Local<Object> exports) {
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
  };

  for (size_t i = 0; i < length_of_array(methods); i++) {
    result->Set(
      Nan::New(methods[i].name).ToLocalChecked(),
      Nan::New<FunctionTemplate>(methods[i].callback)->GetFunction()
    );
  }

  uint32_t transfer_buffer_length = 6;
  transfer_buffer = static_cast<uint32_t *>(malloc(transfer_buffer_length * sizeof(uint32_t)));
  auto js_transfer_buffer = ArrayBuffer::New(Isolate::GetCurrent(), transfer_buffer, transfer_buffer_length * sizeof(uint32_t));
  exports->Set(
    Nan::New("nodeTransferArray").ToLocalChecked(),
    Uint32Array::New(js_transfer_buffer, 0, transfer_buffer_length)
  );

  exports->Set(Nan::New("NodeMethods").ToLocalChecked(), result);
}

void Node::MarshalNode(TSNode node) {
  transfer_buffer[0] = 0;
  transfer_buffer[1] = 0;
  memcpy(&transfer_buffer[0], &node.id, sizeof(node.id));
  transfer_buffer[2] = node.context[0];
  transfer_buffer[3] = node.context[1];
  transfer_buffer[4] = node.context[2];
  transfer_buffer[5] = node.context[3];
}

TSNode Node::UnmarshalNode(const v8::Local<v8::Value> &tree) {
  TSNode result = {{0, 0, 0, 0}, nullptr, nullptr};
  result.tree = Tree::UnwrapTree(tree);
  if (!result.tree) {
    Nan::ThrowTypeError("Argument must be a tree");
    return result;
  }

  memcpy(&result.id, &transfer_buffer[0], sizeof(result.id));
  result.context[0] = transfer_buffer[2];
  result.context[1] = transfer_buffer[3];
  result.context[2] = transfer_buffer[4];
  result.context[3] = transfer_buffer[5];
  return result;
}

void Node::ToString(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    const char *string = ts_node_string(node);
    info.GetReturnValue().Set(Nan::New(string).ToLocalChecked());
    free((char *)string);
  }
}

void Node::IsMissing(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    bool result = ts_node_is_missing(node);
    info.GetReturnValue().Set(Nan::New<Boolean>(result));
  }
}

void Node::HasChanges(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    bool result = ts_node_has_changes(node);
    info.GetReturnValue().Set(Nan::New<Boolean>(result));
  }
}

void Node::HasError(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    bool result = ts_node_has_error(node);
    info.GetReturnValue().Set(Nan::New<Boolean>(result));
  }
}

void Node::FirstNamedChildForIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    Nan::Maybe<uint32_t> byte = ByteCountFromJS(info[1]);
    if (byte.IsJust()) {
      MarshalNode(ts_node_first_named_child_for_byte(node, byte.FromJust()));
    }
  }
}

void Node::FirstChildForIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id && info.Length() > 1) {
    Nan::Maybe<uint32_t> byte = ByteCountFromJS(info[1]);
    if (byte.IsJust()) {
      MarshalNode(ts_node_first_child_for_byte(node, byte.FromJust()));
    }
  }
}

void Node::NamedDescendantForIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    Nan::Maybe<uint32_t> maybe_min = ByteCountFromJS(info[1]);
    Nan::Maybe<uint32_t> maybe_max = ByteCountFromJS(info[2]);
    if (maybe_min.IsJust() && maybe_max.IsJust()) {
      uint32_t min = maybe_min.FromJust();
      uint32_t max = maybe_max.FromJust();
      MarshalNode(ts_node_named_descendant_for_byte_range(node, min, max));
    }
  }
}

void Node::DescendantForIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    Nan::Maybe<uint32_t> maybe_min = ByteCountFromJS(info[1]);
    Nan::Maybe<uint32_t> maybe_max = ByteCountFromJS(info[2]);
    if (maybe_min.IsJust() && maybe_max.IsJust()) {
      uint32_t min = maybe_min.FromJust();
      uint32_t max = maybe_max.FromJust();
      MarshalNode(ts_node_descendant_for_byte_range(node, min, max));
    }
  }
}

void Node::NamedDescendantForPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    Nan::Maybe<TSPoint> maybe_min = PointFromJS(info[1]);
    Nan::Maybe<TSPoint> maybe_max = PointFromJS(info[2]);
    if (maybe_min.IsJust() && maybe_max.IsJust()) {
      TSPoint min = maybe_min.FromJust();
      TSPoint max = maybe_max.FromJust();
      MarshalNode(ts_node_named_descendant_for_point_range(node, min, max));
    }
  }
}

void Node::DescendantForPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    Nan::Maybe<TSPoint> maybe_min = PointFromJS(info[1]);
    Nan::Maybe<TSPoint> maybe_max = PointFromJS(info[2]);
    if (maybe_min.IsJust() && maybe_max.IsJust()) {
      TSPoint min = maybe_min.FromJust();
      TSPoint max = maybe_max.FromJust();
      MarshalNode(ts_node_descendant_for_point_range(node, min, max));
    }
  }
}

void Node::Type(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    const char *result = ts_node_type(node);
    info.GetReturnValue().Set(Nan::New(result).ToLocalChecked());
  }
}

void Node::IsNamed(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    bool result = ts_node_is_named(node);
    info.GetReturnValue().Set(Nan::New(result));
  }
}

void Node::Id(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    uint64_t result = reinterpret_cast<uint64_t>(node.id);
    info.GetReturnValue().Set(Nan::New<Number>(result));
  }
}

void Node::StartIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    int32_t result = ts_node_start_byte(node) / 2;
    info.GetReturnValue().Set(Nan::New<Integer>(result));
  }
}

void Node::EndIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    int32_t result = ts_node_end_byte(node) / 2;
    info.GetReturnValue().Set(Nan::New<Integer>(result));
  }
}

void Node::StartPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    TransferPoint(ts_node_start_point(node));
  }
}

void Node::EndPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    TransferPoint(ts_node_end_point(node));
  }
}

void Node::Child(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    if (!info[1]->IsUint32()) {
      Nan::ThrowTypeError("Second argument must be an integer");
      return;
    }
    uint32_t index = info[1]->Uint32Value();
    MarshalNode(ts_node_child(node, index));
  }
}

void Node::NamedChild(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    if (!info[1]->IsUint32()) {
      Nan::ThrowTypeError("Second argument must be an integer");
      return;
    }
    uint32_t index = info[1]->Uint32Value();
    MarshalNode(ts_node_named_child(node, index));
  }
}

void Node::ChildCount(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    info.GetReturnValue().Set(Nan::New(ts_node_child_count(node)));
  }
}

void Node::NamedChildCount(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    info.GetReturnValue().Set(Nan::New(ts_node_named_child_count(node)));
  }
}

void Node::FirstChild(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    MarshalNode(ts_node_child(node, 0));
  }
}

void Node::FirstNamedChild(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    MarshalNode(ts_node_named_child(node, 0));
  }
}

void Node::LastChild(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    uint32_t child_count = ts_node_child_count(node);
    if (child_count > 0) {
      MarshalNode(ts_node_child(node, child_count - 1));
    }
  }
}

void Node::LastNamedChild(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    uint32_t child_count = ts_node_named_child_count(node);
    if (child_count > 0) {
      MarshalNode(ts_node_named_child(node, child_count - 1));
    }
  }
}

void Node::Parent(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    MarshalNode(ts_node_parent(node));
  }
}

void Node::NextSibling(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    MarshalNode(ts_node_next_sibling(node));
  }
}

void Node::NextNamedSibling(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    MarshalNode(ts_node_next_named_sibling(node));
  }
}

void Node::PreviousSibling(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    MarshalNode(ts_node_prev_sibling(node));
  }
}

void Node::PreviousNamedSibling(const Nan::FunctionCallbackInfo<Value> &info) {
  TSNode node = UnmarshalNode(info[0]);
  if (node.id) {
    MarshalNode(ts_node_prev_named_sibling(node));
  }
}

}  // namespace node_tree_sitter
