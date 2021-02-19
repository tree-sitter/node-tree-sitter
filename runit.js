const Parser = require(".");
const JavaScript = require("tree-sitter-javascript");

const { Query } = Parser;
const parser = new Parser();
parser.setLanguage(JavaScript);
const tree = parser.parse(`
a({
  bc: function de() {
    const fg = function hi() {}
  },
  jk: function lm() {
    const no = function pq() {}
  },
});
`);
const query = new Query(JavaScript, `
(pair
  key: _ @method.def
  (function
    name: (identifier) @method.alias))
(variable_declarator
  name: _ @function.def
  value: (function
    name: (identifier) @function.alias))
":" @delimiter
"=" @operator
`);

const captures = query.captures(tree.rootNode);

console.log(captures);