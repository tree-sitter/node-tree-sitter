tree-sitter
===========

[![Build Status](https://travis-ci.org/maxbrunsfeld/node-tree-sitter.svg?branch=master)](https://travis-ci.org/maxbrunsfeld/node-tree-sitter)

Incremental parsers for node

### Installation

```
npm install tree-sitter
```

### Usage

Make a document:

```javascript
var treeSitter = require("tree-sitter");
var document = new treeSitter.Document();
```

Create a language module using [tree-sitter-compiler](http://github.com/maxbrunsfeld/node-tree-sitter-compiler). See [the javascript module](http://github.com/maxbrunsfeld/node-tree-sitter-javascript) for an example.

Set the document's language:

```javascript
document.setLanguage(require("tree-sitter-javascript"));
```

Set the document's text:

```javascript
document.setInputString("var inc = function(n) { return n + 1; }; inc(5);");
```

Access the document's AST:

```javascript
document.rootNode.toString()

/*
 *    (program
 *      (var_declaration
 *        (identifier)
 *        (function (formal_parameters (identifier)) (statement_block
 *          (return_statement (math_op (identifier) (number))))))
 *      (expression_statement (function_call
 *        (identifier) (number))))
 */

var program = document.children[0];
program.children[0];

/*
 *  { name: 'var_declaration',
 *    row: 0,
 *    column: 0,
 *    start: 0,
 *    end: 40,
 *    children: { length: 2 } }
 */
```
