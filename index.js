var binding = require("bindings")("tree_sitter_runtime_binding"),
    Document = binding.Document,
    StringInput = require("./lib/string_input");

Document.prototype.setInputString = function(string) {
  return this.setInput(new StringInput(string));
};

exports.Document = Document;
