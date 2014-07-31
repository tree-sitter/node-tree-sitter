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
document.setInputString("function(arg1, arg2) { arg2; }");
```

Access the document's AST:

```javascript
var rootNode = document.rootNode();
rootNode.toString();

/*
Returns:
(program
  (expression_statement (function
    (formal_parameters (identifier) (identifier))
    (statement_block
      (expression_statement (identifier))))))
*/

var statement = rootNote.children[0],
    func = statement.children[0],
    parameters = func.children[0];
parameters.children.length;

/*
Returns: 2
*/
```
