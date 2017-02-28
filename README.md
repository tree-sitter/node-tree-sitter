tree-sitter
===========

[![Build Status](https://travis-ci.org/tree-sitter/node-tree-sitter.svg?branch=master)](https://travis-ci.org/tree-sitter/node-tree-sitter)

Incremental parsers for node

### Installation

```
npm install tree-sitter
```

### Usage

Make a document:

```javascript
const {Document} = require('tree-sitter');
const document = new Document();
```

Create a grammar using [tree-sitter-cli](http://github.com/tree-sitter/tree-sitter-cli). See [the JavaScript grammar](http://github.com/tree-sitter/tree-sitter-javascript) for an example.

Set the document's language:

```javascript
document.setLanguage(require('tree-sitter-javascript'));
```

Set the document's text:

```javascript
document.setInputString('var inc = function(n) { return n + 1; }; inc(5);');
```

Access the document's AST:

```javascript
document.parse();
document.rootNode.toString();

/*
 *  (program
 *    (var_declaration
 *      (identifier)
 *      (function (formal_parameters (identifier)) (statement_block
 *        (return_statement (math_op (identifier) (number))))))
 *    (expression_statement (function_call
 *      (identifier) (number))))
 */

const program = document.children[0];
program.children[0];

/*
 *  { name: 'var_declaration',
 *    startPosition: {row: 0, column: 0},
 *    endPosition: {row: 0, column: 40},
 *    startIndex: 0,
 *    endIndex: 40,
 *    children: { length: 2 } }
 */
```
