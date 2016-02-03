var binding = require('./build/Release/tree_sitter_runtime_binding'),
    Document = binding.Document,
    StringInput = require("./lib/string_input");

Document.prototype.setInputString = function(string, bufferSize) {
  this.invalidate()
  this.setInput(new StringInput(string, bufferSize));
  return this;
};

exports.Document = Document;
