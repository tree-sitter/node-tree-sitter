var binding = require("./build/Release/tree_sitter_binding");

exports.compile = binding.compile;
exports.grammar = require("./lib/build_grammar");
exports.rules = require("./lib/rules");
