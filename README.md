node tree-sitter
================

[![Build Status](https://travis-ci.org/tree-sitter/node-tree-sitter.svg?branch=master)](https://travis-ci.org/tree-sitter/node-tree-sitter)
[![Build status](https://ci.appveyor.com/api/projects/status/0pqlhnnk6i9rsakj/branch/master?svg=true)](https://ci.appveyor.com/project/maxbrunsfeld/node-tree-sitter/branch/master)

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
const sourceCode = 'let x = 1; console.log(x);';
const tree = parser.parse(sourceCode);
```

and inspect the syntax tree.

```javascript
console.log(tree.rootNode.toString());

// (program
//   (lexical_declaration
//     (variable_declarator (identifier) (number)))
//   (expression_statement
//     (call_expression
//       (member_expression (identifier) (property_identifier))
//       (arguments (identifier)))))

const callExpression = tree.rootNode.child(1).firstChild;
console.log(callExpression);

// { type: 'call_expression',
//   startPosition: {row: 0, column: 16},
//   endPosition: {row: 0, column: 30},
//   startIndex: 0,
//   endIndex: 30 }
```

If your source code *changes*, you can update the syntax tree. This will take less time than the first parse.

```javascript
// Replace 'let' with 'const'
const newSourceCode = 'const x = 1; console.log(x);';

tree.edit({
  startIndex: 0,
  oldEndIndex: 3,
  newEndIndex: 5,
  startPosition: {row: 0, column: 0},
  oldEndPosition: {row: 0, column: 3},
  newEndPosition: {row: 0, column: 5},
});

const newTree = parser.parse(newCode, tree);
```

### Parsing Text From a Custom Data Structure

If your text is stored in a data structure other than a single string, you can parse it by supplying a callback to `parse` instead of a string:

```javascript
const sourceLines = [
  'let x = 1;',
  'console.log(x);'
];

const tree = parser.parse((index, position) => {
  let line = sourceLines[position.row];
  if (line) {
    return line.slice(position.column);
  }
});
```

### Asynchronous Parsing

If you have source code stored in a [superstring](https://github.com/atom/superstring) `TextBuffer`, you can parse that source code on a background thread with a `Promise`-based interface:

```javascript
const {TextBuffer} = require('superstring');

async function test() {
  const buffer = new TextBuffer('const x= 1; console.log(x);');
  const newTree = await parser.parseTextBuffer(buffer, oldTree);
}
```

Using a background thread can introduce a slight delay, so you may want to allow *some* work to be done on the main thread, in the hopes that parsing will complete so quickly that you won't even *need* a background thread:

```javascript
async function test2() {
  const buffer = new TextBuffer('const x= 1; console.log(x);');
  const newTree = await parser.parseTextBuffer(buffer, oldTree, {
    syncOperationCount: 1000
  });
}
```
