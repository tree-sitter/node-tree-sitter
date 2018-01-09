#include "./ast_node.h"
#include <nan.h>
#include <tree_sitter/runtime.h>
#include <v8.h>
#include "./ast_node_array.h"
#include "./util.h"
#include "./conversions.h"

namespace node_tree_sitter {

using namespace v8;

Nan::Persistent<Function> ASTNode::constructor;
static uint32_t *point_transfer_buffer;

void ASTNode::Init(v8::Local<v8::Object> exports) {
  point_transfer_buffer = static_cast<uint32_t *>(malloc(2 * sizeof(uint32_t)));
  auto js_point_transfer_buffer = ArrayBuffer::New(Isolate::GetCurrent(), point_transfer_buffer, 2 * sizeof(uint32_t));
  exports->Set(Nan::New("pointTransferArray").ToLocalChecked(), Uint32Array::New(js_point_transfer_buffer, 0, 2));

  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("ASTNode").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  GetterPair enum_getters[] = {
    {"startIndex", StartIndex},
    {"endIndex", EndIndex},
    {"type", Type},
    {"isNamed", IsNamed},
  };

  GetterPair non_enum_getters[] = {
    {"parent", Parent},
    {"childIndex", ChildIndex},
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
    {"isValid", IsValid},
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
  exports->Set(Nan::New("ASTNode").ToLocalChecked(), constructor_local);

  constructor.Reset(Nan::Persistent<Function>(constructor_local));
}

ASTNode *ASTNode::Unwrap(const Local<Object> &object) {
  ASTNode *node = Nan::ObjectWrap::Unwrap<ASTNode>(object);
  if (node && node->node_.data)
    return node;
  else
    return NULL;
}

ASTNode *ASTNode::UnwrapValid(const Local<Object> &object) {
  ASTNode *node = Unwrap(object);
  if (node && node->parse_count_ == ts_document_parse_count(node->document_))
    return node;
  else
    return NULL;
}

ASTNode::ASTNode(TSNode node, TSDocument *document, size_t parse_count) :
  node_(node), document_(document), parse_count_(parse_count) {}

Local<Value> ASTNode::NewInstance(TSNode node, TSDocument *document, size_t parse_count) {
  Local<Object> self;
  MaybeLocal<Object> maybe_self = Nan::New(constructor)->NewInstance(Nan::GetCurrentContext());
  if (maybe_self.ToLocal(&self)) {
    (new ASTNode(node, document, parse_count))->Wrap(self);
    return self;
  } else {
    return Nan::Null();
  }
}

void ASTNode::New(const Nan::FunctionCallbackInfo<Value> &info) {
  info.GetReturnValue().Set(Nan::Null());
}

void ASTNode::ToString(const Nan::FunctionCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    const char *string = ts_node_string(node->node_, node->document_);
    info.GetReturnValue().Set(Nan::New(string).ToLocalChecked());
    free((char *)string);
  }
}

void ASTNode::IsValid(const Nan::FunctionCallbackInfo<Value> &info) {
  ASTNode *node = Unwrap(info.This());
  if (node) {
    bool result = node->parse_count_ == ts_document_parse_count(node->document_);
    info.GetReturnValue().Set(Nan::New<Boolean>(result));
  }
}

void ASTNode::IsMissing(const Nan::FunctionCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    bool result = ts_node_is_missing(node->node_);
    info.GetReturnValue().Set(Nan::New<Boolean>(result));
  }
}

void ASTNode::HasChanges(const Nan::FunctionCallbackInfo<Value> &info) {
  ASTNode *node = Unwrap(info.This());
  if (node) {
    bool result = ts_node_has_changes(node->node_);
    info.GetReturnValue().Set(Nan::New<Boolean>(result));
  }
}

void ASTNode::HasError(const Nan::FunctionCallbackInfo<Value> &info) {
  ASTNode *node = Unwrap(info.This());
  if (node) {
    bool result = ts_node_has_error(node->node_);
    info.GetReturnValue().Set(Nan::New<Boolean>(result));
  }
}

void ASTNode::FirstNamedChildForIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node && info.Length() > 0) {
    Nan::Maybe<uint32_t> byte = ByteCountFromJS(info[0]);
    if (byte.IsNothing()) return;
    TSNode result = ts_node_first_named_child_for_byte(node->node_, byte.FromJust());
    if (result.data) {
      info.GetReturnValue().Set(ASTNode::NewInstance(result, node->document_, node->parse_count_));
    }
  }
}

void ASTNode::FirstChildForIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node && info.Length() > 0) {
    Nan::Maybe<uint32_t> byte = ByteCountFromJS(info[0]);
    if (byte.IsNothing()) return;
    TSNode result = ts_node_first_child_for_byte(node->node_, byte.FromJust());
    if (result.data) {
      info.GetReturnValue().Set(ASTNode::NewInstance(result, node->document_, node->parse_count_));
    }
  }
}

void ASTNode::NamedDescendantForIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
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
    info.GetReturnValue().Set(ASTNode::NewInstance(result, node->document_, node->parse_count_));
  }
}

void ASTNode::DescendantForIndex(const Nan::FunctionCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
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
    info.GetReturnValue().Set(ASTNode::NewInstance(result, node->document_, node->parse_count_));
  }
}

void ASTNode::NamedDescendantForPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
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
    info.GetReturnValue().Set(ASTNode::NewInstance(result, node->document_, node->parse_count_));
  }
}

void ASTNode::DescendantForPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
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
    info.GetReturnValue().Set(ASTNode::NewInstance(result, node->document_, node->parse_count_));
  }
}

void ASTNode::Type(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    const char *result = ts_node_type(node->node_, node->document_);
    info.GetReturnValue().Set(Nan::New(result).ToLocalChecked());
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

void ASTNode::IsNamed(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    bool result = ts_node_is_named(node->node_);
    info.GetReturnValue().Set(Nan::New(result));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

void ASTNode::Id(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    uint64_t result = reinterpret_cast<uint64_t>(node->node_.data);
    info.GetReturnValue().Set(Nan::New<Number>(result));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

void ASTNode::StartIndex(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    int32_t result = ts_node_start_byte(node->node_) / 2;
    info.GetReturnValue().Set(Nan::New<Integer>(result));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

void ASTNode::EndIndex(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    int32_t result = ts_node_end_byte(node->node_) / 2;
    info.GetReturnValue().Set(Nan::New<Integer>(result));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

void ASTNode::StartPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    TSPoint result = ts_node_start_point(node->node_);
    point_transfer_buffer[0] = result.row;
    point_transfer_buffer[1] = result.column / 2;
  }
}

void ASTNode::EndPosition(const Nan::FunctionCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    TSPoint result = ts_node_end_point(node->node_);
    point_transfer_buffer[0] = result.row;
    point_transfer_buffer[1] = result.column / 2;
  }
}

void ASTNode::Children(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node)
    info.GetReturnValue().Set(ASTNodeArray::NewInstance(node->node_, node->document_, node->parse_count_, false));
  else
    info.GetReturnValue().Set(Nan::Null());
}

void ASTNode::NamedChildren(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node)
    info.GetReturnValue().Set(ASTNodeArray::NewInstance(node->node_, node->document_, node->parse_count_, true));
  else
    info.GetReturnValue().Set(Nan::Null());
}

void ASTNode::FirstChild(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    TSNode child = ts_node_child(node->node_, 0);
    if (child.data) {
      info.GetReturnValue().Set(ASTNode::NewInstance(child, node->document_, node->parse_count_));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

void ASTNode::FirstNamedChild(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    TSNode child = ts_node_named_child(node->node_, 0);
    if (child.data) {
      info.GetReturnValue().Set(ASTNode::NewInstance(child, node->document_, node->parse_count_));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

void ASTNode::LastChild(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    uint32_t child_count = ts_node_child_count(node->node_);
    if (child_count > 0) {
      TSNode child = ts_node_child(node->node_, child_count - 1);
      info.GetReturnValue().Set(ASTNode::NewInstance(child, node->document_, node->parse_count_));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

void ASTNode::LastNamedChild(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    uint32_t child_count = ts_node_named_child_count(node->node_);
    if (child_count > 0) {
      TSNode child = ts_node_named_child(node->node_, child_count - 1);
      info.GetReturnValue().Set(ASTNode::NewInstance(child, node->document_, node->parse_count_));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

void ASTNode::Parent(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    TSNode parent = ts_node_parent(node->node_);
    if (parent.data) {
      info.GetReturnValue().Set(ASTNode::NewInstance(parent, node->document_, node->parse_count_));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

void ASTNode::ChildIndex(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    uint32_t child_index = ts_node_child_index(node->node_);
    if (child_index != UINT32_MAX) {
      info.GetReturnValue().Set(Nan::New(child_index));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

void ASTNode::NextSibling(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    TSNode sibling = ts_node_next_sibling(node->node_);
    if (sibling.data) {
      info.GetReturnValue().Set(ASTNode::NewInstance(sibling, node->document_, node->parse_count_));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

void ASTNode::NextNamedSibling(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    TSNode sibling = ts_node_next_named_sibling(node->node_);
    if (sibling.data) {
      info.GetReturnValue().Set(ASTNode::NewInstance(sibling, node->document_, node->parse_count_));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

void ASTNode::PreviousSibling(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    TSNode sibling = ts_node_prev_sibling(node->node_);
    if (sibling.data) {
      info.GetReturnValue().Set(ASTNode::NewInstance(sibling, node->document_, node->parse_count_));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

void ASTNode::PreviousNamedSibling(Local<String> property, const Nan::PropertyCallbackInfo<Value> &info) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    TSNode sibling = ts_node_prev_named_sibling(node->node_);
    if (sibling.data) {
      info.GetReturnValue().Set(ASTNode::NewInstance(sibling, node->document_, node->parse_count_));
      return;
    }
  }
  info.GetReturnValue().Set(Nan::Null());
}

}  // namespace node_tree_sitter
