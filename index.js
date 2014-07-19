var binding = require("bindings")("tree_sitter_binding");

module.exports = {
  Document: binding.Document,
  compile: binding.compile,
  loadParserLib: binding.loadParserLib,
  buildParser: require("./lib/build_parser"),
  grammar: require("./lib/build_grammar"),
  rules: require("./lib/rules")
};
