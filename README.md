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

Create a grammar using [tree-sitter-cli](http://github.com/tree-sitter/tree-sitter-cli). See [the JavaScript grammar](http://github.com/tree-sitter/tree-sitter-javascript) and the [Go grammar](http://github.com/tree-sitter/tree-sitter-go) for some examples.

Make a document using your grammar:

```javascript
const {Document} = require('tree-sitter');

const document = new Document();
document.setLanguage(require('tree-sitter-javascript'));
document.setInputString('let x = 1; x++; console.log(x);');
document.parse();
```

Access the document's AST:

```javascript
console.log(document.rootNode.toString());

// (program
//   (lexical_declaration
//     (variable_declarator (identifier) (number)))
//   (expression_statement
//     (update_expression (identifier)))
//   (expression_statement
//     (call_expression
//       (member_expression (identifier) (property_identifier))
//       (arguments (identifier)))))

const callExpression = document.rootNode.children[2].children[0];
console.log(callExpression);

// { type: 'call_expression',
//   startPosition: {row: 0, column: 16},
//   endPosition: {row: 0, column: 30},
//   startIndex: 0,
//   endIndex: 30,
//   children: { length: 2 } }
```
