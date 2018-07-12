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

  get type() {
    marshalNode(this);
    return NodeMethods.type(this.tree);
  }

  get isNamed() {
    marshalNode(this);
    return NodeMethods.isNamed(this.tree);
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
    const {childCount} = this;
    const result = new Array(childCount);
    for (let i = 0; i < childCount; i++) {
      result[i] = this.child(i);
    }
    return result;
  }

  get namedChildren() {
    const {namedChildCount} = this;
    const result = new Array(namedChildCount);
    for (let i = 0; i < namedChildCount; i++) {
      result[i] = this.namedChild(i);
    }
    return result;
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

  descendantsOfType(type, start, end) {
    marshalNode(this);
    const nodes = NodeMethods.descendantsOfType(this.tree, type, start, end);
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

  walk () {
    marshalNode(this);
    return NodeMethods.walk(this.tree);
  }
}

class StringInput {
  constructor(string, bufferSize) {
    this.position = 0;
    this.string = string;
    this.bufferSize = Number.isFinite(bufferSize) ? bufferSize : null;
  }

  seek(position) {
    this.position = position;
  }

  read() {
    const result = this.string.slice(this.position);
    this.position = this.string.length;
    return result;
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
  if (typeof input === 'string') {
    const inputString = input;
    input = (offset) => inputString.slice(offset)
  }
  return parse.call(
    this,
    input,
    oldTree,
    bufferSize,
    includedRanges
  );
};

Parser.prototype.parseTextBuffer = function(
  buffer, oldTree,
  {syncOperationLimit, includedRanges} = {}
) {
  const snapshot = buffer.getSnapshot();
  return new Promise(resolve => {
    parseTextBuffer.call(
      this,
      result => {
        snapshot.destroy();
        resolve(result);
      },
      snapshot,
      oldTree,
      includedRanges,
      syncOperationLimit
    )
  });
};

Parser.prototype.parseTextBufferSync = function(buffer, oldTree, {includedRanges}={}) {
  const snapshot = buffer.getSnapshot();
  const tree = parseTextBufferSync.call(this, snapshot, oldTree, includedRanges);
  snapshot.destroy();
  return tree;
};

const {startPosition, endPosition} = TreeCursor.prototype;

Object.defineProperty(TreeCursor.prototype, 'startPosition', {
  get() {
    startPosition.call(this);
    return unmarshalPoint();
  }
});

Object.defineProperty(TreeCursor.prototype, 'endPosition', {
  get() {
    endPosition.call(this);
    return unmarshalPoint();
  }
});

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

module.exports = Parser;
module.exports.Tree = Tree;
module.exports.SyntaxNode = SyntaxNode;
