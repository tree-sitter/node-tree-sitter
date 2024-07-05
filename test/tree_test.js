const Parser = require("..");
const JavaScript = require('tree-sitter-javascript');
const Rust = require('tree-sitter-rust');
const { assert } = require("chai");

describe("Tree", () => {
  let parser;

  beforeEach(() => {
    parser = new Parser();
    parser.setLanguage(JavaScript)
  });

  describe('.edit', () => {
    let input, edit

    it('updates the positions of existing nodes', () => {
      input = 'abc + cde';

      tree = parser.parse(input);
      assert.equal(
        tree.rootNode.toString(),
        "(program (expression_statement (binary_expression left: (identifier) right: (identifier))))"
      );

      const sumNode = tree.rootNode.firstChild.firstChild;
      let variableNode1 = sumNode.firstChild;
      let variableNode2 = sumNode.lastChild;
      assert.equal(variableNode1.startIndex, 0);
      assert.equal(variableNode1.endIndex, 3);
      assert.equal(variableNode2.startIndex, 6);
      assert.equal(variableNode2.endIndex, 9);

      ([input, edit] = spliceInput(input, input.indexOf('bc'), 0, ' * '));
      assert.equal(input, 'a * bc + cde');

      tree.edit(edit);
      assert.equal(variableNode1.startIndex, 0);
      assert.equal(variableNode1.endIndex, 6);
      assert.equal(variableNode2.startIndex, 9);
      assert.equal(variableNode2.endIndex, 12);

      tree = parser.parse(input, tree);
      assert.equal(
        tree.rootNode.toString(),
        "(program (expression_statement (binary_expression left: (binary_expression left: (identifier) right: (identifier)) right: (identifier))))"
      );
    });

    it("handles non-ascii characters", () => {
      input = 'αβδ + cde';

      tree = parser.parse(input);
      assert.equal(
        tree.rootNode.toString(),
        "(program (expression_statement (binary_expression left: (identifier) right: (identifier))))"
      );

      const variableNode = tree.rootNode.firstChild.firstChild.lastChild;

      ([input, edit] = spliceInput(input, input.indexOf('δ'), 0, '👍 * '));
      assert.equal(input, 'αβ👍 * δ + cde');

      tree.edit(edit);
      assert.equal(variableNode.startIndex, input.indexOf('cde'));

      tree = parser.parse(input, tree);
      assert.equal(
        tree.rootNode.toString(),
        "(program (expression_statement (binary_expression left: (binary_expression left: (identifier) right: (identifier)) right: (identifier))))"
      );
    });
  });

  describe('.getEditedRange()', () => {
    it('returns the range of tokens that have been edited', () => {
      const inputString = 'abc + def + ghi + jkl + mno';
      const tree = parser.parse(inputString);

      assert.equal(tree.getEditedRange(), null)

      tree.edit({
        startIndex: 7,
        oldEndIndex: 7,
        newEndIndex: 8,
        startPosition: { row: 0, column: 7 },
        oldEndPosition: { row: 0, column: 7 },
        newEndPosition: { row: 0, column: 8 }
      });

      tree.edit({
        startIndex: 21,
        oldEndIndex: 21,
        newEndIndex: 22,
        startPosition: { row: 0, column: 21 },
        oldEndPosition: { row: 0, column: 21 },
        newEndPosition: { row: 0, column: 22 }
      });

      assert.deepEqual(tree.getEditedRange(), {
        startIndex: 6,
        endIndex: 23,
        startPosition: { row: 0, column: 6 },
        endPosition: { row: 0, column: 23 },
      });
    })
  });

  describe(".getChangedRanges()", () => {
    it("reports the ranges of text whose syntactic meaning has changed", () => {
      let sourceCode = "abcdefg + hij";
      const tree1 = parser.parse(sourceCode);

      assert.equal(
        tree1.rootNode.toString(),
        "(program (expression_statement (binary_expression left: (identifier) right: (identifier))))"
      );

      sourceCode = "abc + defg + hij";
      tree1.edit({
        startIndex: 2,
        oldEndIndex: 2,
        newEndIndex: 5,
        startPosition: { row: 0, column: 2 },
        oldEndPosition: { row: 0, column: 2 },
        newEndPosition: { row: 0, column: 5 }
      });

      const tree2 = parser.parse(sourceCode, tree1);
      assert.equal(
        tree2.rootNode.toString(),
        "(program (expression_statement (binary_expression left: (binary_expression left: (identifier) right: (identifier)) right: (identifier))))"
      );

      const ranges = tree1.getChangedRanges(tree2);
      assert.deepEqual(ranges, [
        {
          startIndex: 0,
          endIndex: "abc + defg".length,
          startPosition: { row: 0, column: 0 },
          endPosition: { row: 0, column: "abc + defg".length }
        }
      ]);
    });

    it('throws an exception if the argument is not a tree', () => {
      const tree1 = parser.parse("abcdefg + hij");

      assert.throws(() => {
        tree1.getChangedRanges({});
      }, /Argument must be a tree/);
    })
  });

  describe(".walk()", () => {
    it("returns a cursor that can be used to walk the tree", () => {
      parser.setLanguage(Rust);

      //    let mut parser = Parser::new();
      // parser.set_language(&get_language("rust")).unwrap();
      //
      // let tree = parser
      //     .parse(
      //         "
      //             struct Stuff {
      //                 a: A,
      //                 b: Option<B>,
      //             }
      //         ",
      //         None,
      //     )
      //     .unwrap();
      //

      const tree = parser.parse(`
                struct Stuff {
                    a: A,
                    b: Option<B>,
                }
      `);

      const cursor = tree.walk();
      assert.equal(cursor.nodeType, "source_file");

      assert(cursor.gotoFirstChild());
      assert.equal(cursor.nodeType, "struct_item");

      assert(cursor.gotoFirstChild());
      assert.equal(cursor.nodeType, "struct");
      assert(!cursor.nodeIsNamed);

      assert(cursor.gotoNextSibling());
      assert.equal(cursor.nodeType, "type_identifier");
      assert(cursor.nodeIsNamed);

      assert(cursor.gotoNextSibling());
      assert.equal(cursor.nodeType, "field_declaration_list");
      assert(cursor.nodeIsNamed);

      assert(cursor.gotoLastChild());
      assert.equal(cursor.nodeType, "}");
      assert(!cursor.nodeIsNamed);
      assert.deepEqual(cursor.startPosition, { row: 4, column: 16 });

      assert(cursor.gotoPreviousSibling());
      assert.equal(cursor.nodeType, ",");
      assert(!cursor.nodeIsNamed);
      assert.deepEqual(cursor.startPosition, { row: 3, column: 32 });

      assert(cursor.gotoPreviousSibling());
      assert.equal(cursor.nodeType, "field_declaration");
      assert(cursor.nodeIsNamed);
      assert.deepEqual(cursor.startPosition, { row: 3, column: 20 });

      assert(cursor.gotoPreviousSibling());
      assert.equal(cursor.nodeType, ",");
      assert(!cursor.nodeIsNamed);
      assert.deepEqual(cursor.startPosition, { row: 2, column: 24 });

      assert(cursor.gotoPreviousSibling());
      assert.equal(cursor.nodeType, "field_declaration");
      assert(cursor.nodeIsNamed);
      assert.deepEqual(cursor.startPosition, { row: 2, column: 20 });

      assert(cursor.gotoPreviousSibling());
      assert.equal(cursor.nodeType, "{");
      assert(!cursor.nodeIsNamed);
      assert.deepEqual(cursor.startPosition, { row: 1, column: 29 });

      const copy = tree.walk();
      copy.resetTo(cursor);

      assert.equal(copy.nodeType, "{");
      assert(!copy.nodeIsNamed);

      assert(copy.gotoParent());
      assert.equal(copy.nodeType, "field_declaration_list");
      assert(copy.nodeIsNamed);

      assert(copy.gotoParent());
      assert.equal(copy.nodeType, "struct_item");
    });

    it("can fetch the previous sibling", () => {
      parser.setLanguage(Rust);

      const text = `
      // Hi there
      // This is fun!
      // Another one!
      `;

      const tree = parser.parse(text);

      const cursor = tree.walk();
      assert.equal(cursor.nodeType, "source_file");

      assert(cursor.gotoLastChild());
      assert.equal(cursor.nodeType, "line_comment");
      assert.equal(cursor.currentNode.text, "// Another one!");

      assert(cursor.gotoPreviousSibling());
      assert.equal(cursor.nodeType, "line_comment");
      assert.equal(cursor.currentNode.text, "// This is fun!");

      assert(cursor.gotoPreviousSibling());
      assert.equal(cursor.nodeType, "line_comment");
      assert.equal(cursor.currentNode.text, "// Hi there");

      assert(!cursor.gotoPreviousSibling());
    });

    it("can access fields of nodes", () => {
      const tree = parser.parse("function /*1*/ bar /*2*/ () {}");

      const cursor = tree.walk();
      assert.equal(cursor.nodeType, "program");

      assert(cursor.gotoFirstChild());
      assert.equal(cursor.nodeType, "function_declaration");
      assert.equal(cursor.currentFieldName, null);

      assert(cursor.gotoFirstChild());
      assert.equal(cursor.nodeType, "function");
      assert.equal(cursor.currentFieldName, null);

      assert(cursor.gotoNextSibling());
      assert.equal(cursor.nodeType, "comment");
      assert.equal(cursor.currentFieldName, null);

      assert(cursor.gotoNextSibling());
      assert.equal(cursor.nodeType, "identifier");
      assert.equal(cursor.currentFieldName, "name");

      assert(cursor.gotoNextSibling());
      assert.equal(cursor.nodeType, "comment");
      assert.equal(cursor.currentFieldName, null);

      assert(cursor.gotoNextSibling());
      assert.equal(cursor.nodeType, "formal_parameters");
      assert.equal(cursor.currentFieldName, "parameters");
    });

    it("can access children by positions", () => {
      const source = `
    [
        one,
        {
            two: tree
        },
        four, five, six
    ];`.slice(1);
      const tree = parser.parse(source);

      const cursor = tree.walk();
      assert.equal(cursor.nodeType, "program");

      assert.equal(cursor.gotoFirstChildForPosition({ row: 7, column: 0 }), null);
      assert.equal(cursor.gotoFirstChildForPosition({ row: 6, column: 7 }), null);
      assert.equal(cursor.nodeType, "program");

      // descend to expression statement
      assert.equal(cursor.gotoFirstChildForPosition({ row: 6, column: 6 }), 0);
      assert.equal(cursor.nodeType, "expression_statement");

      // step into ';' and back up
      assert.equal(cursor.gotoFirstChildForPosition({ row: 7, column: 0 }), null);
      assert.equal(cursor.gotoFirstChildForPosition({ row: 6, column: 6 }), 1);
      assert.deepEqual(cursor.startPosition, { row: 6, column: 5 });
      assert.equal(cursor.nodeType, ";");
      assert(cursor.gotoParent());

      // descend into array
      assert.equal(cursor.gotoFirstChildForPosition({ row: 6, column: 4 }), 0);
      assert.equal(cursor.nodeType, "array");
      assert.deepEqual(cursor.startPosition, { row: 0, column: 4 });

      // step into '[' and back up
      assert.equal(cursor.gotoFirstChildForPosition({ row: 0, column: 4 }), 0);
      assert.equal(cursor.nodeType, "[");
      assert.deepEqual(cursor.startPosition, { row: 0, column: 4 });
      assert(cursor.gotoParent());

      // step into identifier 'one' and back up
      assert.equal(cursor.gotoFirstChildForPosition({ row: 1, column: 0 }), 1);
      assert.equal(cursor.nodeType, "identifier");
      assert.deepEqual(cursor.startPosition, { row: 1, column: 8 });
      assert(cursor.gotoParent());
      assert.equal(cursor.gotoFirstChildForPosition({ row: 1, column: 10 }), 1);
      assert.equal(cursor.nodeType, "identifier");
      assert.deepEqual(cursor.startPosition, { row: 1, column: 8 });
      assert(cursor.gotoParent());

      // step into first ',' and back up
      assert.equal(cursor.gotoFirstChildForPosition({ row: 1, column: 12 }), 2);
      assert.equal(cursor.nodeType, ",");
      assert.deepEqual(cursor.startPosition, { row: 1, column: 11 });
      assert(cursor.gotoParent());

      // step into identifier 'four' and back up
      assert.equal(cursor.gotoFirstChildForPosition({ row: 5, column: 0 }), 5);
      assert.equal(cursor.nodeType, "identifier");
      assert.deepEqual(cursor.startPosition, { row: 5, column: 8 });
      assert(cursor.gotoParent());
      assert.equal(cursor.gotoFirstChildForPosition({ row: 5, column: 10 }), 5);
      assert.equal(cursor.nodeType, "identifier");
      assert.deepEqual(cursor.startPosition, { row: 5, column: 8 });
      assert(cursor.gotoParent());

      // step into ']' and back up
      assert.equal(cursor.gotoFirstChildForPosition({ row: 6, column: 0 }), 10);
      assert.equal(cursor.nodeType, "]");
      assert.deepEqual(cursor.startPosition, { row: 6, column: 4 });
      assert(cursor.gotoParent());
      assert.equal(cursor.gotoFirstChildForPosition({ row: 6, column: 0 }), 10);
      assert.equal(cursor.nodeType, "]");
      assert.deepEqual(cursor.startPosition, { row: 6, column: 4 });
      assert(cursor.gotoParent());

      // descend into object
      assert.equal(cursor.gotoFirstChildForPosition({ row: 2, column: 0 }), 3);
      assert.equal(cursor.nodeType, "object");
      assert.deepEqual(cursor.startPosition, { row: 2, column: 8 });
    });

    it('returns a cursor that can be used to walk the tree', () => {
      const tree = parser.parse('a * b + c / d');

      const cursor = tree.walk();
      assertCursorState(cursor, {
        nodeType: 'program',
        nodeIsNamed: true,
        nodeIsMissing: false,
        startPosition: { row: 0, column: 0 },
        endPosition: { row: 0, column: 13 },
        startIndex: 0,
        endIndex: 13
      });

      assert(cursor.gotoFirstChild());
      assertCursorState(cursor, {
        nodeType: 'expression_statement',
        nodeIsNamed: true,
        nodeIsMissing: false,
        startPosition: { row: 0, column: 0 },
        endPosition: { row: 0, column: 13 },
        startIndex: 0,
        endIndex: 13
      });

      assert(cursor.gotoFirstChild());
      assertCursorState(cursor, {
        nodeType: 'binary_expression',
        nodeIsNamed: true,
        nodeIsMissing: false,
        startPosition: { row: 0, column: 0 },
        endPosition: { row: 0, column: 13 },
        startIndex: 0,
        endIndex: 13
      });

      assert(cursor.gotoFirstChild());
      assertCursorState(cursor, {
        nodeType: 'binary_expression',
        nodeIsNamed: true,
        nodeIsMissing: false,
        startPosition: { row: 0, column: 0 },
        endPosition: { row: 0, column: 5 },
        startIndex: 0,
        endIndex: 5
      });

      assert(cursor.gotoFirstChild());
      assertCursorState(cursor, {
        nodeType: 'identifier',
        nodeIsNamed: true,
        nodeIsMissing: false,
        startPosition: { row: 0, column: 0 },
        endPosition: { row: 0, column: 1 },
        startIndex: 0,
        endIndex: 1
      });

      assert(!cursor.gotoFirstChild())
      assert(cursor.gotoNextSibling());
      assertCursorState(cursor, {
        nodeType: '*',
        nodeIsNamed: false,
        nodeIsMissing: false,
        startPosition: { row: 0, column: 2 },
        endPosition: { row: 0, column: 3 },
        startIndex: 2,
        endIndex: 3
      });

      assert(cursor.gotoNextSibling());
      assertCursorState(cursor, {
        nodeType: 'identifier',
        nodeIsNamed: true,
        nodeIsMissing: false,
        startPosition: { row: 0, column: 4 },
        endPosition: { row: 0, column: 5 },
        startIndex: 4,
        endIndex: 5
      });

      assert(!cursor.gotoNextSibling());
      assert(cursor.gotoParent());
      assertCursorState(cursor, {
        nodeType: 'binary_expression',
        nodeIsNamed: true,
        nodeIsMissing: false,
        startPosition: { row: 0, column: 0 },
        endPosition: { row: 0, column: 5 },
        startIndex: 0,
        endIndex: 5
      });

      assert(cursor.gotoNextSibling());
      assertCursorState(cursor, {
        nodeType: '+',
        nodeIsNamed: false,
        nodeIsMissing: false,
        startPosition: { row: 0, column: 6 },
        endPosition: { row: 0, column: 7 },
        startIndex: 6,
        endIndex: 7
      });

      assert(cursor.gotoNextSibling());
      assertCursorState(cursor, {
        nodeType: 'binary_expression',
        nodeIsNamed: true,
        nodeIsMissing: false,
        startPosition: { row: 0, column: 8 },
        endPosition: { row: 0, column: 13 },
        startIndex: 8,
        endIndex: 13
      });

      const childIndex = cursor.gotoFirstChildForIndex(12);
      assertCursorState(cursor, {
        nodeType: 'identifier',
        nodeIsNamed: true,
        nodeIsMissing: false,
        startPosition: { row: 0, column: 12 },
        endPosition: { row: 0, column: 13 },
        startIndex: 12,
        endIndex: 13
      });
      assert.equal(childIndex, 2);

      assert(!cursor.gotoNextSibling());
      assert(cursor.gotoParent());
      assert(cursor.gotoParent());
      assert(cursor.gotoParent());
      assert(cursor.gotoParent());
      assert(!cursor.gotoParent());
    });

    it('returns a cursor that can be reset anywhere in the tree', () => {
      const tree = parser.parse('a * b + c / d');
      const cursor = tree.walk();
      const root = tree.rootNode.firstChild;

      cursor.reset(root.firstChild.firstChild);
      assertCursorState(cursor, {
        nodeType: 'binary_expression',
        nodeIsNamed: true,
        nodeIsMissing: false,
        startPosition: { row: 0, column: 0 },
        endPosition: { row: 0, column: 5 },
        startIndex: 0,
        endIndex: 5
      });

      cursor.gotoFirstChild()
      assertCursorState(cursor, {
        nodeType: 'identifier',
        nodeIsNamed: true,
        nodeIsMissing: false,
        startPosition: { row: 0, column: 0 },
        endPosition: { row: 0, column: 1 },
        startIndex: 0,
        endIndex: 1
      });

      assert(cursor.gotoParent());
      assert(!cursor.gotoParent());
    })
  });

  describe(".rootNode", () => {
    it("tests tree node equality", () => {
      parser.setLanguage(Rust);

      const tree = parser.parse("struct A {}");
      const node1 = tree.rootNode;
      const node2 = tree.rootNode;
      assert.deepEqual(node1, node2);
      assert.deepEqual(node1.firstChild, node2.firstChild);
      assert.notDeepEqual(node1.firstChild, node2);
    });
  });

  describe(".printDotGraph()", () => {
    it("prints a dot graph to the output file", () => {
      if (process.platform === "win32") {
        return;
      }

      const tmp = require("tmp");
      const debugGraphFile = tmp.fileSync({ postfix: ".dot" });
      const tree = parser.parse("const zero = 0");
      tree.printDotGraph(debugGraphFile.fd);

      const fs = require('fs');
      const logReader = fs.readFileSync(debugGraphFile.name, 'utf8').split('\n');
      for (let line of logReader) {
        const match = line.match(/error-cost: (\d+)/);
        if (match) {
          assert.equal(parseInt(match[1]), 0); // error-cost should always be 0
        }
      }

      debugGraphFile.removeCallback();
    });
  });
});

function assertCursorState(cursor, params) {
  assert.isBoolean(cursor.nodeIsNamed);
  assert.isBoolean(cursor.nodeIsMissing);

  assert.equal(cursor.nodeType, params.nodeType);
  assert.equal(cursor.nodeIsNamed, params.nodeIsNamed);
  assert.equal(cursor.nodeIsMissing, params.nodeIsMissing);
  assert.deepEqual(cursor.startPosition, params.startPosition);
  assert.deepEqual(cursor.endPosition, params.endPosition);
  assert.deepEqual(cursor.startIndex, params.startIndex);
  assert.deepEqual(cursor.endIndex, params.endIndex);

  const node = cursor.currentNode
  assert.equal(node.type, params.nodeType);
  assert.equal(node.isNamed, params.nodeIsNamed);
  assert.equal(node.isMissing, params.nodeIsMissing);
  assert.deepEqual(node.startPosition, params.startPosition);
  assert.deepEqual(node.endPosition, params.endPosition);
  assert.deepEqual(node.startIndex, params.startIndex);
  assert.deepEqual(node.endIndex, params.endIndex);
}

function spliceInput(input, startIndex, lengthRemoved, newText) {
  const oldEndIndex = startIndex + lengthRemoved;
  const newEndIndex = startIndex + newText.length;
  const startPosition = getExtent(input.slice(0, startIndex));
  const oldEndPosition = getExtent(input.slice(0, oldEndIndex));
  input = input.slice(0, startIndex) + newText + input.slice(oldEndIndex);
  const newEndPosition = getExtent(input.slice(0, newEndIndex));
  return [
    input,
    {
      startIndex, startPosition,
      oldEndIndex, oldEndPosition,
      newEndIndex, newEndPosition
    }
  ];
}

function getExtent(text) {
  let row = 0
  let index;
  for (index = 0; index != -1; index = text.indexOf('\n', index)) {
    index++
    row++;
  }
  return { row, column: text.length - index };
}
