#include "./node.h"
#include <nan.h>
#include <tree_sitter/runtime.h>
#include <v8.h>
#include "./node_array.h"
#include "./util.h"
#include "./conversions.h"

namespace node_tree_sitter {

using namespace v8;

Nan::Persistent<Function> Node::constructor;
static uint32_t *point_transfer_buffer;

void Node::Init(v8::Local<v8::Object> exports) {
  point_transfer_buffer = static_cast<uint32_t *>(malloc(2 * sizeof(uint32_t)));
  auto js_point_transfer_buffer = ArrayBuffer::New(Isolate::GetCurrent(), point_transfer_buffer, 2 * sizeof(uint32_t));
  exports->Set(Nan::New("pointTransferArray").ToLocalChecked(), Uint32Array::New(js_point_transfer_buffer, 0, 2));

  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("Node").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  GetterPair enum_getters[] = {
    {"startIndex", StartIndex},
    {"endIndex", EndIndex},
    {"type", Type},
    {"isNamed", IsNamed},
  };

  GetterPair non_enum_getters[] = {
    {"parent", Parent},
    {"children", Children},
    {"namedChildren", NamedChildren},
    {"firstChild", FirstChild},
    {"lastChild", LastChild},
    {"firstNamedChild", FirstNamedChild},
    {"lastNamedChild", LastNamedChild},
    {"nextSibling", NextSibling},
    {"nextNamedSibling", NextNamedSibling},
    {"previousSibling", PreviousSibling},
    {"previousNamedSibling", PreviousNamedSibling},
    {"id", Id},
  };

  FunctionPair methods[] = {
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

  for (size_t i = 0; i < length_of_array(enum_getters); i++)
    Nan::SetAccessor(
      tpl->InstanceTemplate(),
      Nan::New(enum_getters[i].name).ToLocalChecked(),
      enum_getters[i].callback);

  for (size_t i = 0; i < length_of_array(non_enum_getters); i++)
    Nan::SetAccessor(
      tpl->InstanceTemplate(),
      Nan::New(non_enum_getters[i].name).ToLocalChecked(),
      non_enum_getters[i].callback,
      0, Local<Value>(), DEFAULT, DontEnum);

  for (size_t i = 0; i < length_of_array(methods); i++)
    Nan::SetPrototypeMethod(tpl, methods[i].name, methods[i].callback);

  auto constructor_local = tpl->GetFunction();
  exports->Set(Nan::New("Node").ToLocalChecked(), constructor_local);

  constructor.Reset(Nan::Persistent<Function>(constructor_local));
}

Node *Node::Unwrap(const Local<Object> &object) {
  Node *node = Nan::ObjectWrap::Unwrap<Node>(object);
  if (node && node->node_.subtree) {
    return node;
  } else {
    return NULL;
  }
}

Node::Node(TSNode node) : node_(node) {}

Local<Value> Node::NewInstance(TSNode node) {
  Local<Object> self;
  MaybeLocal<Object> maybe_self = Nan::New(constructor)->NewInstance(Nan::GetCurrentContext());
  if (maybe_self.ToLocal(&self)) {
    (new Node(node))->Wrap(self);
    return self;
  } else {
    return Nan::Null();
  }
}

void Node::New(const Nan::FunctionCallbackInfo<Value> &info) {
  info.GetReturnValue().Set(Nan::Null());
}

void Node::ToString(const Nan::FunctionCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    const char *string = ts_node_string(node->node_);
    info.GetReturnValue().Set(Nan::New(string).ToLocalChecked());
    free((char *)string);
  }
}

void Node::IsMissing(const Nan::FunctionCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    bool result = ts_node_is_missing(node->node_);
    info.GetReturnValue().Set(Nan::New<Boolean>(result));
  }
}

void Node::HasChanges(const Nan::FunctionCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    bool result = ts_node_has_changes(node->node_);
    info.GetReturnValue().Set(Nan::New<Boolean>(result));
  }
}

void Node::HasError(const Nan::FunctionCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    bool result = ts_node_has_error(node->node_);
    info.GetReturnValue().Set(Nan::New<Boolean>(result));
  }
}

void Node::FirstNamedChildForIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node && info.Length() > 0) {
    Nan::Maybe<uint32_t> byte = ByteCountFromJS(info[0]);
    if (byte.IsNothing()) return;
    TSNode result = ts_node_first_named_child_for_byte(node->node_, byte.FromJust());
    if (result.subtree) {
      info.GetReturnValue().Set(Node::NewInstance(result));
    }
  }
}

void Node::FirstChildForIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node && info.Length() > 0) {
    Nan::Maybe<uint32_t> byte = ByteCountFromJS(info[0]);
    if (byte.IsNothing()) return;
    TSNode result = ts_node_first_child_for_byte(node->node_, byte.FromJust());
    if (result.subtree) {
      info.GetReturnValue().Set(Node::NewInstance(result));
    }
  }
}

void Node::NamedDescendantForIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    uint32_t min, max;
    switch (info.Length()) {
      case 1: {
        Nan::Maybe<uint32_t> maybe_value = ByteCountFromJS(info[0]);
        if (maybe_value.IsNothing()) return;
        min = max = maybe_value.FromJust();
        break;
      }
      case 2: {
        Nan::Maybe<uint32_t> maybe_min = ByteCountFromJS(info[0]);
        Nan::Maybe<uint32_t> maybe_max = ByteCountFromJS(info[1]);
        if (maybe_min.IsNothing()) return;
        if (maybe_max.IsNothing()) return;
        min = maybe_min.FromJust();
        max = maybe_max.FromJust();
        break;
      }
      default:
        Nan::ThrowTypeError("Must provide 1 or 2 character indices");
        return;
    }

    TSNode result = ts_node_named_descendant_for_byte_range(node->node_, min, max);
    info.GetReturnValue().Set(Node::NewInstance(result));
  }
}

void Node::DescendantForIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    uint32_t min, max;
    switch (info.Length()) {
      case 1: {
        Nan::Maybe<uint32_t> maybe_value = ByteCountFromJS(info[0]);
        if (maybe_value.IsNothing()) return;
        min = max = maybe_value.FromJust();
        break;
      }
      case 2: {
        Nan::Maybe<uint32_t> maybe_min = ByteCountFromJS(info[0]);
        Nan::Maybe<uint32_t> maybe_max = ByteCountFromJS(info[1]);
        if (maybe_min.IsNothing()) return;
        if (maybe_max.IsNothing()) return;
        min = maybe_min.FromJust();
        max = maybe_max.FromJust();
        break;
      }
      default:
        Nan::ThrowTypeError("Must provide 1 or 2 character indices");
        return;
    }

    TSNode result = ts_node_descendant_for_byte_range(node->node_, min, max);
    info.GetReturnValue().Set(Node::NewInstance(result));
  }
}

void Node::NamedDescendantForPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    TSPoint min, max;
    switch (info.Length()) {
      case 1: {
        Nan::Maybe<TSPoint> value = PointFromJS(info[0]);
        if (value.IsNothing()) return;
        min = max = value.FromJust();
        break;
      }
      case 2: {
        Nan::Maybe<TSPoint> maybe_min = PointFromJS(info[0]);
        Nan::Maybe<TSPoint> maybe_max = PointFromJS(info[1]);
        if (maybe_min.IsNothing()) return;
        if (maybe_max.IsNothing()) return;
        min = maybe_min.FromJust();
        max = maybe_max.FromJust();
        break;
      }
      default:
        Nan::ThrowTypeError("Must provide 1 or 2 points");
        return;
    }

    TSNode result = ts_node_named_descendant_for_point_range(node->node_, min, max);
    info.GetReturnValue().Set(Node::NewInstance(result));
  }
}

void Node::DescendantForPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    TSPoint min, max;
    switch (info.Length()) {
      case 1: {
        Nan::Maybe<TSPoint> value = PointFromJS(info[0]);
        if (value.IsNothing()) return;
        min = max = value.FromJust();
        break;
      }
      case 2: {
        Nan::Maybe<TSPoint> maybe_min = PointFromJS(info[0]);
        Nan::Maybe<TSPoint> maybe_max = PointFromJS(info[1]);
        if (maybe_min.IsNothing()) return;
        if (maybe_max.IsNothing()) return;
        min = maybe_min.FromJust();
        max = maybe_max.FromJust();
        break;
      }
      default:
        Nan::ThrowTypeError("Must provide 1 or 2 points");
        return;
    }

    TSNode result = ts_node_descendant_for_point_range(node->node_, min, max);
    info.GetReturnValue().Set(Node::NewInstance(result));
  }
}

void Node::Type(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    const char *result = ts_node_type(node->node_);
    info.GetReturnValue().Set(Nan::New(result).ToLocalChecked());
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

void Node::IsNamed(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    bool result = ts_node_is_named(node->node_);
    info.GetReturnValue().Set(Nan::New(result));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

void Node::Id(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    uint64_t result = reinterpret_cast<uint64_t>(node->node_.subtree);
    info.GetReturnValue().Set(Nan::New<Number>(result));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

void Node::StartIndex(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    int32_t result = ts_node_start_byte(node->node_) / 2;
    info.GetReturnValue().Set(Nan::New<Integer>(result));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

void Node::EndIndex(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    int32_t result = ts_node_end_byte(node->node_) / 2;
    info.GetReturnValue().Set(Nan::New<Integer>(result));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

void Node::StartPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    TSPoint result = ts_node_start_point(node->node_);
    point_transfer_buffer[0] = result.row;
    point_transfer_buffer[1] = result.column / 2;
  }
}

void Node::EndPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    TSPoint result = ts_node_end_point(node->node_);
    point_transfer_buffer[0] = result.row;
    point_transfer_buffer[1] = result.column / 2;
  }
}

void Node::Children(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node)
    info.GetReturnValue().Set(NodeArray::NewInstance(node->node_, false));
  else
    info.GetReturnValue().Set(Nan::Null());
}

void Node::NamedChildren(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node)
    info.GetReturnValue().Set(NodeArray::NewInstance(node->node_, true));
  else
    info.GetReturnValue().Set(Nan::Null());
}

void Node::FirstChild(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    TSNode child = ts_node_child(node->node_, 0);
    if (child.subtree) {
      info.GetReturnValue().Set(Node::NewInstance(child));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

void Node::FirstNamedChild(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    TSNode child = ts_node_named_child(node->node_, 0);
    if (child.subtree) {
      info.GetReturnValue().Set(Node::NewInstance(child));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

void Node::LastChild(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    uint32_t child_count = ts_node_child_count(node->node_);
    if (child_count > 0) {
      TSNode child = ts_node_child(node->node_, child_count - 1);
      info.GetReturnValue().Set(Node::NewInstance(child));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

void Node::LastNamedChild(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    uint32_t child_count = ts_node_named_child_count(node->node_);
    if (child_count > 0) {
      TSNode child = ts_node_named_child(node->node_, child_count - 1);
      info.GetReturnValue().Set(Node::NewInstance(child));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

void Node::Parent(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    TSNode parent = ts_node_parent(node->node_);
    if (parent.subtree) {
      info.GetReturnValue().Set(Node::NewInstance(parent));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

void Node::NextSibling(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    TSNode sibling = ts_node_next_sibling(node->node_);
    if (sibling.subtree) {
      info.GetReturnValue().Set(Node::NewInstance(sibling));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

void Node::NextNamedSibling(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    TSNode sibling = ts_node_next_named_sibling(node->node_);
    if (sibling.subtree) {
      info.GetReturnValue().Set(Node::NewInstance(sibling));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

void Node::PreviousSibling(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    TSNode sibling = ts_node_prev_sibling(node->node_);
    if (sibling.subtree) {
      info.GetReturnValue().Set(Node::NewInstance(sibling));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

void Node::PreviousNamedSibling(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  Node *node = Unwrap(info.This());
  if (node) {
    TSNode sibling = ts_node_prev_named_sibling(node->node_);
    if (sibling.subtree) {
      info.GetReturnValue().Set(Node::NewInstance(sibling));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

}  // namespace node_tree_sitter
