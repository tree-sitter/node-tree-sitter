const Parser = require(".");
const Javascript = require("tree-sitter-javascript");
const jsParser = new Parser();
jsParser.setLanguage(Javascript);
const rootNode = jsParser.parse(`
const Parser = require(".");
const Javascript = require("tree-sitter-javascript");
const jsParser = new Parser();
`)
console.log(rootNode.rootNode.toString(), 'end')