var binding = require("bindings")("tree_sitter_binding");

module.exports = {
  Document: binding.Document,
  compile: binding.compile,
  loadParser: binding.loadParser,
  buildParser: require("./lib/build_parser"),
  grammar: require("./lib/build_grammar"),
  rules: require("./lib/rules")
};
