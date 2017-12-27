const {Document, ASTNode, pointTransferArray} = require('./build/Release/tree_sitter_runtime_binding');
const StringInput = require("./lib/string_input");

Document.prototype.setInputString = function(string, bufferSize) {
  this.invalidate()
  this.setInput(new StringInput(string, bufferSize));
  return this;
};

const {startPosition, endPosition} = ASTNode.prototype

Object.defineProperty(ASTNode.prototype, 'startPosition', {
  get() {
    startPosition.call(this)
    return {row: pointTransferArray[0], column: pointTransferArray[1]}
  }
})

Object.defineProperty(ASTNode.prototype, 'endPosition', {
  get() {
    endPosition.call(this)
    return {row: pointTransferArray[0], column: pointTransferArray[1]}
  }
})

exports.Document = Document;
