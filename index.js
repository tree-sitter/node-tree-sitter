let binding;
try {
  binding = require('./build/Release/tree_sitter_runtime_binding');
} catch (e) {
  try {
    binding = require('./build/Debug/tree_sitter_runtime_binding');
  } catch (_) {
    throw e;
  }
}

const util = require('util')
const {Parser, NodeMethods, Tree, TreeCursor} = binding;

const {rootNode, edit} = Tree.prototype;

Object.defineProperty(Tree.prototype, 'rootNode', {
  get() {
    return rootNode.call(this) || unmarshalNode(this);
  }
});

Tree.prototype.edit = function(arg) {
  edit.call(
    this,
    arg.startPosition.row, arg.startPosition.column,
    arg.oldEndPosition.row, arg.oldEndPosition.column,
    arg.newEndPosition.row, arg.newEndPosition.column,
    arg.startIndex,
    arg.oldEndIndex,
    arg.newEndIndex
  );
};

Tree.prototype.walk = function() {
  return this.rootNode.walk()
};

class SyntaxNode {
  constructor(tree) {
    this.tree = tree;
  }

  [util.inspect.custom]() {
    return 'SyntaxNode {\n' +
      '  type: ' + this.type + ',\n' +
      '  startPosition: ' + pointToString(this.startPosition) + ',\n' +
      '  endPosition: ' + pointToString(this.endPosition) + ',\n' +
      '  childCount: ' + this.childCount + ',\n' +
      '}'
  }

  get type() {
    marshalNode(this);
    return NodeMethods.type(this.tree);
  }

  get isNamed() {
    marshalNode(this);
    return NodeMethods.isNamed(this.tree);
  }

  get text() {
    return this.tree.getText(this);
  }

  get startPosition() {
    marshalNode(this);
    NodeMethods.startPosition(this.tree);
    return unmarshalPoint();
  }

  get endPosition() {
    marshalNode(this);
    NodeMethods.endPosition(this.tree);
    return unmarshalPoint();
  }

  get startIndex() {
    marshalNode(this);
    return NodeMethods.startIndex(this.tree);
  }

  get endIndex() {
    marshalNode(this);
    return NodeMethods.endIndex(this.tree);
  }

  get parent() {
    marshalNode(this);
    return NodeMethods.parent(this.tree) || unmarshalNode(this.tree);
  }

  get children() {
    marshalNode(this);
    const nodes = NodeMethods.children(this.tree);
    unmarshalNodes(this.tree, nodes);
    return nodes
  }

  get namedChildren() {
    marshalNode(this);
    const nodes = NodeMethods.namedChildren(this.tree);
    unmarshalNodes(this.tree, nodes);
    return nodes
  }

  get childCount() {
    marshalNode(this);
    return NodeMethods.childCount(this.tree);
  }

  get namedChildCount() {
    marshalNode(this);
    return NodeMethods.namedChildCount(this.tree);
  }

  get firstChild() {
    marshalNode(this);
    return NodeMethods.firstChild(this.tree) || unmarshalNode(this.tree);
  }

  get firstNamedChild() {
    marshalNode(this);
    return NodeMethods.firstNamedChild(this.tree) || unmarshalNode(this.tree);
  }

  get lastChild() {
    marshalNode(this);
    return NodeMethods.lastChild(this.tree) || unmarshalNode(this.tree);
  }

  get lastNamedChild() {
    marshalNode(this);
    return NodeMethods.lastNamedChild(this.tree) || unmarshalNode(this.tree);
  }

  get nextSibling() {
    marshalNode(this);
    return NodeMethods.nextSibling(this.tree) || unmarshalNode(this.tree);
  }

  get nextNamedSibling() {
    marshalNode(this);
    return NodeMethods.nextNamedSibling(this.tree) || unmarshalNode(this.tree);
  }

  get previousSibling() {
    marshalNode(this);
    return NodeMethods.previousSibling(this.tree) || unmarshalNode(this.tree);
  }

  get previousNamedSibling() {
    marshalNode(this);
    return NodeMethods.previousNamedSibling(this.tree) || unmarshalNode(this.tree);
  }

  hasChanges() {
    marshalNode(this);
    return NodeMethods.hasChanges(this.tree);
  }

  hasError() {
    marshalNode(this);
    return NodeMethods.hasError(this.tree);
  }

  isMissing() {
    marshalNode(this);
    return NodeMethods.isMissing(this.tree);
  }

  toString() {
    marshalNode(this);
    return NodeMethods.toString(this.tree);
  }

  child(index) {
    marshalNode(this);
    return NodeMethods.child(this.tree, index) || unmarshalNode(this.tree);
  }

  namedChild(index) {
    marshalNode(this);
    return NodeMethods.namedChild(this.tree, index) || unmarshalNode(this.tree);
  }

  firstChildForIndex(index) {
    marshalNode(this);
    return NodeMethods.firstChildForIndex(this.tree, index) || unmarshalNode(this.tree);
  }

  firstNamedChildForIndex(index) {
    marshalNode(this);
    return NodeMethods.firstNamedChildForIndex(this.tree, index) || unmarshalNode(this.tree);
  }

  namedDescendantForIndex(start, end) {
    marshalNode(this);
    let result
    if (end != null) {
      result = NodeMethods.namedDescendantForIndex(this.tree, start, end);
    } else {
      result = NodeMethods.namedDescendantForIndex(this.tree, start, start);
    }
    return result || unmarshalNode(this.tree);
  }

  descendantForIndex(start, end) {
    marshalNode(this);
    let result
    if (end != null) {
      result = NodeMethods.descendantForIndex(this.tree, start, end);
    } else {
      result = NodeMethods.descendantForIndex(this.tree, start, start);
    }
    return result || unmarshalNode(this.tree);
  }

  descendantsOfType(types, start, end) {
    marshalNode(this);
    if (typeof types === 'string') types = [types]
    const nodes = NodeMethods.descendantsOfType(this.tree, types, start, end);
    unmarshalNodes(this.tree, nodes);
    return nodes
  }

  namedDescendantForPosition(start, end) {
    marshalNode(this);
    let result
    if (end != null) {
      result = NodeMethods.namedDescendantForPosition(this.tree, start, end);
    } else {
      result = NodeMethods.namedDescendantForPosition(this.tree, start, start);
    }
    return result || unmarshalNode(this.tree);
  }

  descendantForPosition(start, end) {
    marshalNode(this);
    let result
    if (end != null) {
      result = NodeMethods.descendantForPosition(this.tree, start, end);
    } else {
      result = NodeMethods.descendantForPosition(this.tree, start, start);
    }
    return result || unmarshalNode(this.tree);
  }

  closest(types) {
    if (typeof types === 'string') types = [types]
    marshalNode(this);
    return NodeMethods.closest(this.tree, types) || unmarshalNode(this.tree);
  }

  walk () {
    marshalNode(this);
    const cursor = NodeMethods.walk(this.tree);
    cursor.tree = this.tree;
    return cursor;
  }
}

const {parse, parseTextBuffer, parseTextBufferSync, setLanguage} = Parser.prototype;
const languageSymbol = Symbol('parser.language');

Parser.prototype.setLanguage = function(language) {
  setLanguage.call(this, language);
  this[languageSymbol] = language;
  return this;
};

Parser.prototype.getLanguage = function(language) {
  return this[languageSymbol] || null;
};

Parser.prototype.parse = function(input, oldTree, {bufferSize, includedRanges}={}) {
  let getText, treeInput = input
  if (typeof input === 'string') {
    const inputString = input;
    input = (offset, position) => inputString.slice(offset)
    getText = getTextFromString
  } else {
    getText = getTextFromFunction
  }
  const tree = parse.call(
    this,
    input,
    oldTree,
    bufferSize,
    includedRanges
  );
  if (tree) {
    tree.input = treeInput
    tree.getText = getText
  }
  return tree
};

Parser.prototype.parseTextBuffer = function(
  buffer, oldTree,
  {syncOperationLimit, includedRanges} = {}
) {
  let tree
  let resolveTreePromise
  const treePromise = new Promise(resolve => { resolveTreePromise = resolve })
  const snapshot = buffer.getSnapshot();
  parseTextBuffer.call(
    this,
    result => {
      tree = result
      snapshot.destroy();
      if (tree) {
        tree.input = buffer
        tree.getText = getTextFromTextBuffer
      }
      resolveTreePromise(tree);
    },
    snapshot,
    oldTree,
    includedRanges,
    syncOperationLimit
  );

  // If the parse finished synchronously because of the given `syncOperationLimit`,
  // then return the tree immediately so that callers have the option of continuing
  // synchronously.
  return tree || treePromise
};

Parser.prototype.parseTextBufferSync = function(buffer, oldTree, {includedRanges}={}) {
  const snapshot = buffer.getSnapshot();
  const tree = parseTextBufferSync.call(this, snapshot, oldTree, includedRanges);
  if (tree) {
    tree.input = buffer;
    tree.getText = getTextFromTextBuffer;
  }
  snapshot.destroy();
  return tree;
};

const {startPosition, endPosition, currentNode, reset} = TreeCursor.prototype;

Object.defineProperties(TreeCursor.prototype, {
  currentNode: {
    get() {
      return currentNode.call(this) || unmarshalNode(this.tree);
    }
  },
  startPosition: {
    get() {
      startPosition.call(this);
      return unmarshalPoint();
    }
  },
  endPosition: {
    get() {
      endPosition.call(this);
      return unmarshalPoint();
    }
  },
  nodeText: {
    get() {
      return this.tree.getText(this)
    }
  }
});

TreeCursor.prototype.reset = function(node) {
  marshalNode(node);
  reset.call(this);
}

function getTextFromString (node) {
  return this.input.substring(node.startIndex, node.endIndex);
}

function getTextFromFunction ({startIndex, endIndex}) {
  const {input} = this
  let result = '';
  const goalLength = endIndex - startIndex;
  while (result.length < goalLength) {
    const text = input(startIndex + result.length);
    result += text;
  }
  return result.substr(0, goalLength);
}

function getTextFromTextBuffer ({startPosition, endPosition}) {
  return this.input.getTextInRange({start: startPosition, end: endPosition});
}

const {pointTransferArray} = binding;

const NODE_FIELD_COUNT = 6;

function unmarshalNode(tree, offset = 0) {
  const {nodeTransferArray} = binding;
  if (nodeTransferArray[0] || nodeTransferArray[1]) {
    const result = new SyntaxNode(tree);
    for (let i = 0; i < NODE_FIELD_COUNT; i++) {
      result[i] = nodeTransferArray[offset + i];
    }
    tree._cacheNode(result);
    return result;
  }
  return null
}

function unmarshalNodes(tree, nodes) {
  let offset = 0;
  for (let i = 0, {length} = nodes; i < length; i++) {
    if (!nodes[i]) {
      nodes[i] = unmarshalNode(tree, offset)
      offset += NODE_FIELD_COUNT
    }
  }
}

function marshalNode(node) {
  const {nodeTransferArray} = binding;
  for (let i = 0; i < NODE_FIELD_COUNT; i++) {
    nodeTransferArray[i] = node[i];
  }
}

function unmarshalPoint() {
  return {row: pointTransferArray[0], column: pointTransferArray[1]};
}

function pointToString(point) {
  return `{row: ${point.row}, column: ${point.column}}`;
}

module.exports = Parser;
module.exports.Tree = Tree;
module.exports.SyntaxNode = SyntaxNode;
module.exports.TreeCursor = TreeCursor;
