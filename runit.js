const Parser = require(".");
const Javascript = require("tree-sitter-javascript");
const jsParser = new Parser();
jsParser.setLanguage(Javascript);

const code = jsParser.parse(`
const Parser = require(".");
const Javascript = require("tree-sitter-javascript");
const jsParser = new Parser();
`)
const output = code.rootNode.toString()
console.log(output);
