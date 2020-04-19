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

const vm = require('vm');
const util = require('util')
const {Query, QueryCursor, Parser, NodeMethods, Tree, TreeCursor} = binding;

const {rootNode, edit} = Tree.prototype;

Object.defineProperty(Tree.prototype, 'rootNode', {
  get() {
    return unmarshalNode(rootNode.call(this), this);
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
    return this.constructor.name + ' {\n' +
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

  get typeId() {
    marshalNode(this);
    return NodeMethods.typeId(this.tree);
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
    return unmarshalNode(NodeMethods.parent(this.tree), this.tree);
  }

  get children() {
    marshalNode(this);
    return unmarshalNodes(NodeMethods.children(this.tree), this.tree);
  }

  get namedChildren() {
    marshalNode(this);
    return unmarshalNodes(NodeMethods.namedChildren(this.tree), this.tree);
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
    return unmarshalNode(NodeMethods.firstChild(this.tree), this.tree);
  }

  get firstNamedChild() {
    marshalNode(this);
    return unmarshalNode(NodeMethods.firstNamedChild(this.tree), this.tree);
  }

  get lastChild() {
    marshalNode(this);
    return unmarshalNode(NodeMethods.lastChild(this.tree), this.tree);
  }

  get lastNamedChild() {
    marshalNode(this);
    return unmarshalNode(NodeMethods.lastNamedChild(this.tree), this.tree);
  }

  get nextSibling() {
    marshalNode(this);
    return unmarshalNode(NodeMethods.nextSibling(this.tree), this.tree);
  }

  get nextNamedSibling() {
    marshalNode(this);
    return unmarshalNode(NodeMethods.nextNamedSibling(this.tree), this.tree);
  }

  get previousSibling() {
    marshalNode(this);
    return unmarshalNode(NodeMethods.previousSibling(this.tree), this.tree);
  }

  get previousNamedSibling() {
    marshalNode(this);
    return unmarshalNode(NodeMethods.previousNamedSibling(this.tree), this.tree);
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
    return unmarshalNode(NodeMethods.child(this.tree, index), this.tree);
  }

  namedChild(index) {
    marshalNode(this);
    return unmarshalNode(NodeMethods.namedChild(this.tree, index), this.tree);
  }

  firstChildForIndex(index) {
    marshalNode(this);
    return unmarshalNode(NodeMethods.firstChildForIndex(this.tree, index), this.tree);
  }

  firstNamedChildForIndex(index) {
    marshalNode(this);
    return unmarshalNode(NodeMethods.firstNamedChildForIndex(this.tree, index), this.tree);
  }

  namedDescendantForIndex(start, end) {
    marshalNode(this);
    if (end == null) end = start;
    return unmarshalNode(NodeMethods.namedDescendantForIndex(this.tree, start, end), this.tree);
  }

  descendantForIndex(start, end) {
    marshalNode(this);
    if (end == null) end = start;
    return unmarshalNode(NodeMethods.descendantForIndex(this.tree, start, end), this.tree);
  }

  descendantsOfType(types, start, end) {
    marshalNode(this);
    if (typeof types === 'string') types = [types]
    return unmarshalNodes(NodeMethods.descendantsOfType(this.tree, types, start, end), this.tree);
  }

  namedDescendantForPosition(start, end) {
    marshalNode(this);
    if (end == null) end = start;
    return unmarshalNode(NodeMethods.namedDescendantForPosition(this.tree, start, end), this.tree);
  }

  descendantForPosition(start, end) {
    marshalNode(this);
    if (end == null) end = start;
    return unmarshalNode(NodeMethods.descendantForPosition(this.tree, start, end), this.tree);
  }

  closest(types) {
    marshalNode(this);
    if (typeof types === 'string') types = [types]
    return unmarshalNode(NodeMethods.closest(this.tree, types), this.tree);
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
  if (!language.nodeSubclasses) {
    initializeLanguageNodeClasses(language)
  }
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
    tree.language = this.getLanguage()
  }
  return tree
};

Parser.prototype.parseTextBuffer = function(
  buffer, oldTree,
  {syncTimeoutMicros, includedRanges} = {}
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
        tree.language = this.getLanguage()
      }
      resolveTreePromise(tree);
    },
    snapshot,
    oldTree,
    includedRanges,
    syncTimeoutMicros
  );

  // If the parse finished synchronously within the time specified by the
  // `syncTimeoutMicros` parameter, then return the tree immediately
  // so that callers have the option of continuing synchronously.
  return tree || treePromise
};

Parser.prototype.parseTextBufferSync = function(buffer, oldTree, {includedRanges}={}) {
  const snapshot = buffer.getSnapshot();
  const tree = parseTextBufferSync.call(this, snapshot, oldTree, includedRanges);
  if (tree) {
    tree.input = buffer;
    tree.getText = getTextFromTextBuffer;
    tree.language = this.getLanguage()
  }
  snapshot.destroy();
  return tree;
};

const {startPosition, endPosition, currentNode, reset} = TreeCursor.prototype;

Object.defineProperties(TreeCursor.prototype, {
  currentNode: {
    get() {
      return unmarshalNode(currentNode.call(this), this.tree);
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

const {exec} = QueryCursor.prototype;

QueryCursor.prototype.exec = function(query, tree, cb) {
  exec.call(this, query, tree, (patternName, nodeTypeId) => {
    const node = unmarshalNode(nodeTypeId, tree);
    cb(patternName, node);
  });
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
const ERROR_TYPE_ID = 0xFFFF

function unmarshalNode(value, tree, offset = 0) {
  if (typeof value === 'object') {
    const node = value;
    return node;
  } else {
    const nodeTypeId = value;
    const NodeClass = nodeTypeId === ERROR_TYPE_ID
      ? SyntaxNode
      : tree.language.nodeSubclasses[nodeTypeId];
    const {nodeTransferArray} = binding;
    if (nodeTransferArray[0] || nodeTransferArray[1]) {
      const result = new NodeClass(tree);
      for (let i = 0; i < NODE_FIELD_COUNT; i++) {
        result[i] = nodeTransferArray[offset + i];
      }
      tree._cacheNode(result);
      return result;
    }
    return null
  }
}

function unmarshalNodes(nodes, tree) {
  let offset = 0;
  for (let i = 0, {length} = nodes; i < length; i++) {
    const node = unmarshalNode(nodes[i], tree, offset);
    if (node !== nodes[i]) {
      nodes[i] = node;
      offset += NODE_FIELD_COUNT
    }
  }
  return nodes;
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

function initializeLanguageNodeClasses(language) {
  const nodeTypeNamesById = binding.getNodeTypeNamesById(language);
  const nodeFieldNamesById = binding.getNodeFieldNamesById(language);
  const nodeTypeInfo = language.nodeTypeInfo || [];

  const nodeSubclasses = [];
  for (let id = 0, n = nodeTypeNamesById.length; id < n; id++) {
    nodeSubclasses[id] = SyntaxNode;

    const typeName = nodeTypeNamesById[id];
    if (!typeName) continue;

    const typeInfo = nodeTypeInfo.find(info => info.named && info.type === typeName);
    if (!typeInfo) continue;

    const fieldNames = [];
    let classBody = '\n';
    if (typeInfo.fields) {
      for (const fieldName in typeInfo.fields) {
        const fieldId = nodeFieldNamesById.indexOf(fieldName);
        if (fieldId === -1) continue;
        if (typeInfo.fields[fieldName].multiple) {
          const getterName = camelCase(fieldName) + 'Nodes';
          fieldNames.push(getterName);
          classBody += `
            get ${getterName}() {
              marshalNode(this);
              return unmarshalNodes(NodeMethods.childNodesForFieldId(this.tree, ${fieldId}), this.tree);
            }
          `.replace(/\s+/g, ' ') + '\n';
        } else {
          const getterName = camelCase(fieldName, false) + 'Node';
          fieldNames.push(getterName);
          classBody += `
            get ${getterName}() {
              marshalNode(this);
              return unmarshalNode(NodeMethods.childNodeForFieldId(this.tree, ${fieldId}), this.tree);
            }
          `.replace(/\s+/g, ' ') + '\n';
        }
      }
    }

    const className = camelCase(typeName, true) + 'Node';
    const nodeSubclass = eval(`class ${className} extends SyntaxNode {${classBody}}; ${className}`);
    nodeSubclass.prototype.type = typeName;
    nodeSubclass.prototype.fields = Object.freeze(fieldNames.sort())
    nodeSubclasses[id] = nodeSubclass;
  }

  language.nodeSubclasses = nodeSubclasses
}

function camelCase(name, upperCase) {
  name = name.replace(/_(\w)/g, (match, letter) => letter.toUpperCase());
  if (upperCase) name = name[0].toUpperCase() + name.slice(1);
  return name;
}

module.exports = Parser;
module.exports.Query = Query;
module.exports.QueryCursor = QueryCursor;
module.exports.Tree = Tree;
module.exports.SyntaxNode = SyntaxNode;
module.exports.TreeCursor = TreeCursor;
