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

const {rootNode} = Tree.prototype;

Object.defineProperty(Tree.prototype, 'rootNode', {
  get() {
    if (!this._nodes) this._nodes = {};
    rootNode.call(this);
    return unmarshalNode(this);
  }
})

Tree.prototype.walk = function () {
  return this.rootNode.walk()
}

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
    NodeMethods.parent(this.tree);
    return unmarshalNode(this.tree);
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
    NodeMethods.firstChild(this.tree);
    return unmarshalNode(this.tree);
  }

  get firstNamedChild() {
    marshalNode(this);
    NodeMethods.firstNamedChild(this.tree);
    return unmarshalNode(this.tree);
  }

  get lastChild() {
    marshalNode(this);
    NodeMethods.lastChild(this.tree);
    return unmarshalNode(this.tree);
  }

  get lastNamedChild() {
    marshalNode(this);
    NodeMethods.lastNamedChild(this.tree);
    return unmarshalNode(this.tree);
  }

  get nextSibling() {
    marshalNode(this);
    NodeMethods.nextSibling(this.tree);
    return unmarshalNode(this.tree);
  }

  get nextNamedSibling() {
    marshalNode(this);
    NodeMethods.nextNamedSibling(this.tree);
    return unmarshalNode(this.tree);
  }

  get previousSibling() {
    marshalNode(this);
    NodeMethods.previousSibling(this.tree);
    return unmarshalNode(this.tree);
  }

  get previousNamedSibling() {
    marshalNode(this);
    NodeMethods.previousNamedSibling(this.tree);
    return unmarshalNode(this.tree);
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
    NodeMethods.child(this.tree, index);
    return unmarshalNode(this.tree);
  }

  namedChild(index) {
    marshalNode(this);
    NodeMethods.namedChild(this.tree, index);
    return unmarshalNode(this.tree);
  }

  firstChildForIndex(index) {
    marshalNode(this);
    NodeMethods.firstChildForIndex(this.tree, index);
    return unmarshalNode(this.tree);
  }

  firstNamedChildForIndex(index) {
    marshalNode(this);
    NodeMethods.firstNamedChildForIndex(this.tree, index);
    return unmarshalNode(this.tree);
  }

  namedDescendantForIndex(start, end) {
    marshalNode(this);
    if (end != null) {
      NodeMethods.namedDescendantForIndex(this.tree, start, end);
    } else {
      NodeMethods.namedDescendantForIndex(this.tree, start, start);
    }
    return unmarshalNode(this.tree);
  }

  descendantForIndex(start, end) {
    marshalNode(this);
    if (end != null) {
      NodeMethods.descendantForIndex(this.tree, start, end);
    } else {
      NodeMethods.descendantForIndex(this.tree, start, start);
    }
    return unmarshalNode(this.tree);
  }

  descendantsOfType(type, start, end) {
    marshalNode(this);
    const count = NodeMethods.descendantsOfType(this.tree, type, start, end);
    return unmarshalNodes(this.tree, count);
  }

  namedDescendantForPosition(start, end) {
    marshalNode(this);
    if (end != null) {
      NodeMethods.namedDescendantForPosition(this.tree, start, end);
    } else {
      NodeMethods.namedDescendantForPosition(this.tree, start, start);
    }
    return unmarshalNode(this.tree);
  }

  descendantForPosition(start, end) {
    marshalNode(this);
    if (end != null) {
      NodeMethods.descendantForPosition(this.tree, start, end);
    } else {
      NodeMethods.descendantForPosition(this.tree, start, start);
    }
    return unmarshalNode(this.tree);
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
  const key = `${nodeTransferArray[offset]}${nodeTransferArray[offset + 1]}`
  if (key === '00') return null;
  let result = tree._nodes[key];
  if (!result) {
    result = new SyntaxNode(tree);
    tree._nodes[key] = result;
  }
  for (let i = 0; i < NODE_FIELD_COUNT; i++) {
    result[i] = nodeTransferArray[offset + i];
  }
  return result;
}

function unmarshalNodes(tree, count) {
  const result = new Array(count);
  let offset = 0;
  for (let i = 0; i < count; i++) {
    result[i] = unmarshalNode(tree, offset)
    offset += 6
  }
  return result
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
