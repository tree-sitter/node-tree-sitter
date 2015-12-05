#include "./ast_node.h"
#include <nan.h>
#include <tree_sitter/runtime.h>
#include <v8.h>
#include "./ast_node_array.h"
#include "./util.h"

namespace node_tree_sitter {

using namespace v8;

Nan::Persistent<Function> ASTNode::constructor;

void ASTNode::Init(Local<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("ASTNode").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  GetterPair enum_getters[3] = {
    {"start", Start},
    {"end", End},
    {"type", Type},
  };

  GetterPair non_enum_getters[7] = {
    {"parent", Parent},
    {"children", Children},
    {"namedChildren", NamedChildren},
    {"nextSibling", NextSibling},
    {"nextNamedSibling", NextNamedSibling},
    {"previousSibling", PreviousSibling},
    {"previousNamedSibling", PreviousNamedSibling},
  };

  FunctionPair methods[4] = {
    {"isValid", IsValid},
    {"toString", ToString},
    {"descendantForRange", DescendantForRange},
    {"namedDescendantForRange", NamedDescendantForRange},
  };

  for (size_t i = 0; i < sizeof(enum_getters) / sizeof(enum_getters[0]); i++)
    Nan::SetAccessor(
      tpl->InstanceTemplate(),
      Nan::New(enum_getters[i].name).ToLocalChecked(),
      enum_getters[i].callback);

  for (size_t i = 0; i < sizeof(non_enum_getters) / sizeof(non_enum_getters[0]); i++)
    Nan::SetAccessor(
      tpl->InstanceTemplate(),
      Nan::New(non_enum_getters[i].name).ToLocalChecked(),
      non_enum_getters[i].callback,
      0, Local<Value>(), DEFAULT, DontEnum);

  for (size_t i = 0; i < sizeof(methods) / sizeof(methods[0]); i++)
    Nan::SetPrototypeMethod(tpl, methods[i].name, methods[i].callback);

  constructor.Reset(Nan::Persistent<Function>(tpl->GetFunction()));
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
  Local<Object> instance = Nan::New(constructor)->NewInstance(0, NULL);
  (new ASTNode(node, document, parse_count))->Wrap(instance);
  return instance;
}

NAN_METHOD(ASTNode::New) {
  info.GetReturnValue().Set(Nan::Null());
}

NAN_METHOD(ASTNode::ToString) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    const char *string = ts_node_string(node->node_, node->document_);
    info.GetReturnValue().Set(Nan::New(string).ToLocalChecked());
    free((char *)string);
  }
}

NAN_METHOD(ASTNode::IsValid) {
  ASTNode *node = Unwrap(info.This());
  if (node) {
    bool result = node->parse_count_ == ts_document_parse_count(node->document_);
    info.GetReturnValue().Set(Nan::New<Boolean>(result));
  }
}

NAN_METHOD(ASTNode::NamedDescendantForRange) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    int min, max;
    switch (info.Length()) {
      case 1: {
        min = max = info[0]->Int32Value();
        break;
      }
      case 2: {
        min = info[0]->Int32Value();
        max = info[1]->Int32Value();
        break;
      }
      default:
        Nan::ThrowTypeError("Must provide 1 or 2 numeric arguments");
        return;
    }

    TSNode result = ts_node_named_descendant_for_range(node->node_, min, max);
    info.GetReturnValue().Set(ASTNode::NewInstance(result, node->document_, node->parse_count_));
  }
}

NAN_METHOD(ASTNode::DescendantForRange) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    int min, max;
    switch (info.Length()) {
      case 1: {
        min = max = info[0]->Int32Value();
        break;
      }
      case 2: {
        min = info[0]->Int32Value();
        max = info[1]->Int32Value();
        break;
      }
      default:
        Nan::ThrowTypeError("Must provide 1 or 2 numeric arguments");
        return;
    }

    TSNode result = ts_node_descendant_for_range(node->node_, min, max);
    info.GetReturnValue().Set(ASTNode::NewInstance(result, node->document_, node->parse_count_));
  }
}

NAN_GETTER(ASTNode::Type) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    const char *result = ts_node_name(node->node_, node->document_);
    info.GetReturnValue().Set(Nan::New(result).ToLocalChecked());
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_GETTER(ASTNode::Start) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    size_t result = ts_node_start_char(node->node_);
    info.GetReturnValue().Set(Nan::New<Number>(result));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_GETTER(ASTNode::End) {
  ASTNode *node = UnwrapValid(info.This());
  if (node) {
    size_t result = ts_node_end_char(node->node_);
    info.GetReturnValue().Set(Nan::New<Number>(result));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}


NAN_GETTER(ASTNode::Children) {
  ASTNode *node = UnwrapValid(info.This());
  if (node)
    info.GetReturnValue().Set(ASTNodeArray::NewInstance(node->node_, node->document_, node->parse_count_, false));
  else
    info.GetReturnValue().Set(Nan::Null());
}

NAN_GETTER(ASTNode::NamedChildren) {
  ASTNode *node = UnwrapValid(info.This());
  if (node)
    info.GetReturnValue().Set(ASTNodeArray::NewInstance(node->node_, node->document_, node->parse_count_, true));
  else
    info.GetReturnValue().Set(Nan::Null());
}

NAN_GETTER(ASTNode::Parent) {
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

NAN_GETTER(ASTNode::NextSibling) {
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

NAN_GETTER(ASTNode::NextNamedSibling) {
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

NAN_GETTER(ASTNode::PreviousSibling) {
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

NAN_GETTER(ASTNode::PreviousNamedSibling) {
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
