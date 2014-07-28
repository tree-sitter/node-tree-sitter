var binding = require("bindings")("tree_sitter_runtime_binding"),
    Document = binding.Document,
    Parser = binding.Parser;

Document.prototype.setInputString = function(string) {
  this.setInput(new StringInput(string));
};

function StringInput(string) {
  this.position = 0;
  this.string = string;
}

StringInput.prototype.seek = function(n) {
  this.position = n;
}

StringInput.prototype.read = function() {
  var result = this.string.slice(this.position);
  this.position = result.length;
  return result;
}

module.exports = {
  Document: Document,
  Parser: Parser,
};
