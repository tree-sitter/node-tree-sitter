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

const {Parser, Node, NodeArray, pointTransferArray} = binding;

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

const {parse, setLanguage} = Parser.prototype;
const languageSymbol = Symbol('parser.language');

Parser.prototype.setLanguage = function(language) {
  setLanguage.call(this, language);
  this[languageSymbol] = language;
  return this;
}

Parser.prototype.getLanguage = function(language) {
  return this[languageSymbol] || null;
}

Parser.prototype.parse = function(input, oldTree, bufferSize) {
  if (typeof input === 'string') {
    return parse.call(this, new StringInput(input, bufferSize), oldTree);
  } else {
    return parse.call(this, input, oldTree);
  }
}

NodeArray.prototype[Symbol.iterator] = function*() {
  let node = this[0];

  const getNext = this.isNamed ?
    (node) => node.nextNamedSibling :
    (node) => node.nextSibling;

  while (node) {
    yield node;
    node = getNext(node);
  }
}

const {startPosition, endPosition} = Node.prototype

Object.defineProperty(Node.prototype, 'startPosition', {
  get() {
    startPosition.call(this);
    return {row: pointTransferArray[0], column: pointTransferArray[1]};
  }
});

Object.defineProperty(Node.prototype, 'endPosition', {
  get() {
    endPosition.call(this);
    return {row: pointTransferArray[0], column: pointTransferArray[1]};
  }
});

module.exports = Parser;
