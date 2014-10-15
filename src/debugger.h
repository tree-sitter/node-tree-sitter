#ifndef NODE_TREE_SITTER_DEBUGGER_H_
#define NODE_TREE_SITTER_DEBUGGER_H_

#include <v8.h>
#include <tree_sitter/runtime.h>

namespace node_tree_sitter {

TSDebugger DebuggerMake(v8::Handle<v8::Function>);

}  // namespace node_tree_sitter

#endif  // NODE_TREE_SITTER_DEBUGGER_H_
