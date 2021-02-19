const Parser = require(".");
const JavaScript = require("tree-sitter-javascript");

const { Query } = Parser;
const parser = new Parser();
parser.setLanguage(JavaScript);
const tree = parser.parse(`
const ab = require('./ab');
new Cd(EF);
`);

const query = new Query(JavaScript, `
(identifier) @variable
((identifier) @function.builtin
 (#eq? @function.builtin "require"))
((identifier) @constructor
 (#match? @constructor "^[A-Z]"))
((identifier) @constant
 (#match? @constant "^[A-Z]{2,}$"))
`);

const captures = query.captures(tree.rootNode);

console.log(captures);