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
    if (!this.nodes) this.nodes = {};
    rootNode.call(this);
    return unmarshalNode(this);
  }
})

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

Parser.prototype.parse = function(input, oldTree, bufferSize) {
  if (typeof input === 'string') {
    return parse.call(this, new StringInput(input, bufferSize), oldTree);
  } else {
    return parse.call(this, input, oldTree);
  }
};

Parser.prototype.parseTextBuffer = function(buffer, oldTree, options) {
  const snapshot = buffer.getSnapshot();
  const syncOperationLimit = options && options.syncOperationLimit;
  return new Promise(resolve => {
    parseTextBuffer.call(this, result => {
      snapshot.destroy();
      resolve(result);
    }, snapshot, oldTree, syncOperationLimit)
  });
};

Parser.prototype.parseTextBufferSync = function(buffer, oldTree) {
  const snapshot = buffer.getSnapshot();
  const tree = parseTextBufferSync.call(this, snapshot, oldTree);
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

const {pointTransferArray, nodeTransferArray} = binding;

function unmarshalNode(tree) {
  const key = `${nodeTransferArray[0]}${nodeTransferArray[1]}`
  if (key === '00') return null;
  let result = tree.nodes[key];
  if (!result) {
    result = new SyntaxNode(tree);
    result._0 = nodeTransferArray[0];
    result._1 = nodeTransferArray[1];
    result._2 = nodeTransferArray[2];
    result._3 = nodeTransferArray[3];
    result._4 = nodeTransferArray[4];
    result._5 = nodeTransferArray[5];
    tree.nodes[key] = result;
  }
  return result;
}

function marshalNode(node) {
  nodeTransferArray[0] = node._0;
  nodeTransferArray[1] = node._1;
  nodeTransferArray[2] = node._2;
  nodeTransferArray[3] = node._3;
  nodeTransferArray[4] = node._4;
  nodeTransferArray[5] = node._5;
}

function unmarshalPoint() {
  return {row: pointTransferArray[0], column: pointTransferArray[1]};
}

module.exports = Parser;
