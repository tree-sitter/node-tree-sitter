node tree-sitter
================

[![Build Status](https://travis-ci.org/tree-sitter/node-tree-sitter.svg?branch=master)](https://travis-ci.org/tree-sitter/node-tree-sitter)
[![Build status](https://ci.appveyor.com/api/projects/status/vtmbd6i92e97l55w/branch/master?svg=true)](https://ci.appveyor.com/project/maxbrunsfeld/tree-sitter/branch/master)

Incremental parsers for node

### Installation

```sh
npm install tree-sitter
```

### Usage

First, you'll need a Tree-sitter grammar for the language you want to parse. There are many [existing grammars](https://github.com/tree-sitter) such as [tree-sitter-javascript](http://github.com/tree-sitter/tree-sitter-javascript) and [tree-sitter-go](http://github.com/tree-sitter/tree-sitter-go). You can also develop a new grammar using the [Tree-sitter CLI](http://github.com/tree-sitter/tree-sitter-cli).

Once you've got your grammar, create a parser with that grammar.

```javascript
const Parser = require('tree-sitter');
const JavaScript = require('tree-sitter-javascript');

const parser = new Parser();
parser.setLanguage(JavaScript);
```

Then you can parse some source code,

```javascript
const sourceCode = 'var x = 1; x++; console.log(x);';
const tree = parser.parse(sourceCode);
```

and inspect the syntax tree.

```javascript
console.log(tree.rootNode.toString());

// (program
//   (variable_declaration
//     (variable_declarator (identifier) (number)))
//   (expression_statement
//     (update_expression (identifier)))
//   (expression_statement
//     (call_expression
//       (member_expression (identifier) (property_identifier))
//       (arguments (identifier)))))

const callExpression = tree.rootNode.children[2].firstChild;
console.log(callExpression);

// { type: 'call_expression',
//   startPosition: {row: 0, column: 16},
//   endPosition: {row: 0, column: 30},
//   startIndex: 0,
//   endIndex: 30,
//   children: { length: 2 } }
```

If your source code *changes*, you can update the syntax tree. This will take less time than the first parse.

```javascript
// Replace 'var' with 'let'
tree.edit({
  startIndex: 0,
  lengthAdded: 3,
  lengthRemoved: 3,
  startPosition: {row: 0, column: 0},
  extentAdded: {row: 0, column: 3},
  extentRemoved: {row: 0, column: 3},
});

const newSourceCode = 'let x = 1; x++; console.log(x);';
const newTree = parser.parse(newCode, tree);
```
