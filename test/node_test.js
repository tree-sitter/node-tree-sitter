const Parser = require("..");
const JavaScript = require('tree-sitter-javascript');
const { assert } = require("chai");
const { TextBuffer } = require("superstring");

describe("Node", () => {
  let parser;

  beforeEach(() => {
    parser = new Parser().setLanguage(JavaScript);
  });

  describe("subclasses", () => {
    it("generates a subclass for each node type", () => {
      const tree = parser.parse(`
        class A {
          @autobind
          @something
          b(c, d) {
            return c + d;
          }
        }
      `);

      const classNode = tree.rootNode.firstChild;
      assert.deepEqual(classNode.fields, ['bodyNode', 'decoratorNodes', 'nameNode'])

      const methodNode = classNode.bodyNode.firstNamedChild;
      assert.equal(methodNode.constructor.name, 'MethodDefinitionNode');
      assert.equal(methodNode.nameNode.text, 'b');
      assert.deepEqual(methodNode.fields, ['bodyNode', 'decoratorNodes', 'nameNode', 'parametersNode'])

      const decoratorNodes = methodNode.decoratorNodes;
      assert.deepEqual(decoratorNodes.map(_ => _.text), ['@autobind', '@something'])

      const paramsNode = methodNode.parametersNode;
      assert.equal(paramsNode.constructor.name, 'FormalParametersNode');
      assert.equal(paramsNode.namedChildren.length, 2);

      const bodyNode = methodNode.bodyNode;
      assert.equal(bodyNode.constructor.name, 'StatementBlockNode');

      const returnNode = bodyNode.namedChildren[0];
      assert.equal(returnNode.constructor.name, 'ReturnStatementNode');

      const binaryNode = returnNode.firstNamedChild;
      assert.equal(binaryNode.constructor.name, 'BinaryExpressionNode');

      assert.equal(binaryNode.leftNode.text, 'c')
      assert.equal(binaryNode.rightNode.text, 'd')
      assert.equal(binaryNode.operatorNode.type, '+')
    })
  });

  describe(".children", () => {
    it("returns an array of child nodes", () => {
      const tree = parser.parse("x10 + 1000");
      assert.equal(1, tree.rootNode.children.length);
      const sumNode = tree.rootNode.firstChild.firstChild;
      assert.deepEqual(
        sumNode.children.map(child => child.type),
        ["identifier", "+", "number"]
      );
    });
  });

  describe(".namedChildren", () => {
    it("returns an array of named child nodes", () => {
      const tree = parser.parse("x10 + 1000");
      const sumNode = tree.rootNode.firstChild.firstChild;
      assert.equal(1, tree.rootNode.namedChildren.length);
      assert.deepEqual(
        ["identifier", "number"],
        sumNode.namedChildren.map(child => child.type)
      );
    });
  });

  describe(".startIndex and .endIndex", () => {
    it("returns the character index where the node starts/ends in the text", () => {
      const tree = parser.parse("a👍👎1 / b👎c👎");
      const quotientNode = tree.rootNode.firstChild.firstChild;

      assert.equal(0, quotientNode.startIndex);
      assert.equal(15, quotientNode.endIndex);
      assert.deepEqual(
        [0, 7, 9],
        quotientNode.children.map(child => child.startIndex)
      );
      assert.deepEqual(
        [6, 8, 15],
        quotientNode.children.map(child => child.endIndex)
      );
    });
  });

  describe(".startPosition and .endPosition", () => {
    it("returns the row and column where the node starts/ends in the text", () => {
      const tree = parser.parse("x10 + 1000");
      const sumNode = tree.rootNode.firstChild.firstChild;
      assert.equal("binary_expression", sumNode.type);

      assert.deepEqual({ row: 0, column: 0 }, sumNode.startPosition);
      assert.deepEqual({ row: 0, column: 10 }, sumNode.endPosition);
      assert.deepEqual(
        [{ row: 0, column: 0 }, { row: 0, column: 4 }, { row: 0, column: 6 }],
        sumNode.children.map(child => child.startPosition)
      );
      assert.deepEqual(
        [{ row: 0, column: 3 }, { row: 0, column: 5 }, { row: 0, column: 10 }],
        sumNode.children.map(child => child.endPosition)
      );
    });

    it("handles characters that occupy two UTF16 code units", () => {
      const tree = parser.parse("a👍👎1 /\n b👎c👎");
      const sumNode = tree.rootNode.firstChild.firstChild;
      assert.deepEqual(
        [
          [{ row: 0, column: 0 }, { row: 0, column: 6 }],
          [{ row: 0, column: 7 }, { row: 0, column: 8 }],
          [{ row: 1, column: 1 }, { row: 1, column: 7 }]
        ],
        sumNode.children.map(child => [child.startPosition, child.endPosition])
      );
    });
  });

  describe(".parent", () => {
    it("returns the node's parent", () => {
      const tree = parser.parse("x10 + 1000");
      const sumNode = tree.rootNode.firstChild;
      const variableNode = sumNode.firstChild;
      assert.equal(sumNode, variableNode.parent);
      assert.equal(tree.rootNode, sumNode.parent);
    });
  });

  describe('.child(), .firstChild, .lastChild', () => {
    it('returns null when the node has no children', () => {
      const tree = parser.parse("x10 + 1000");
      const sumNode = tree.rootNode.firstChild.firstChild;
      const variableNode = sumNode.firstChild;
      assert.equal(variableNode.firstChild, null);
      assert.equal(variableNode.lastChild, null);
      assert.equal(variableNode.firstNamedChild, null);
      assert.equal(variableNode.lastNamedChild, null);
      assert.equal(variableNode.child(1), null);
    })
  });

  describe(".nextSibling and .previousSibling", () => {
    it("returns the node's next and previous sibling", () => {
      const tree = parser.parse("x10 + 1000");
      const sumNode = tree.rootNode.firstChild.firstChild;
      assert.equal(sumNode.children[1], sumNode.children[0].nextSibling);
      assert.equal(sumNode.children[2], sumNode.children[1].nextSibling);
      assert.equal(
        sumNode.children[0],
        sumNode.children[1].previousSibling
      );
      assert.equal(
        sumNode.children[1],
        sumNode.children[2].previousSibling
      );
    });
  });

  describe(".nextNamedSibling and .previousNamedSibling", () => {
    it("returns the node's next and previous named sibling", () => {
      const tree = parser.parse("x10 + 1000");
      const sumNode = tree.rootNode.firstChild.firstChild;
      assert.equal(
        sumNode.namedChildren[1],
        sumNode.namedChildren[0].nextNamedSibling
      );
      assert.equal(
        sumNode.namedChildren[0],
        sumNode.namedChildren[1].previousNamedSibling
      );
    });
  });

  describe(".descendantForIndex(min, max)", () => {
    it("returns the smallest node that spans the given range", () => {
      const tree = parser.parse("x10 + 1000");
      const sumNode = tree.rootNode.firstChild.firstChild;
      assert.equal("identifier", sumNode.descendantForIndex(1, 2).type);
      assert.equal("+", sumNode.descendantForIndex(4, 4).type);

      assert.throws(() => {
        sumNode.descendantForIndex(1, {});
      }, /Character index must be a number/);

      assert.throws(() => {
        sumNode.descendantForIndex();
      }, /Character index must be a number/);
    });
  });

  describe(".namedDescendantForIndex", () => {
    it("returns the smallest node that spans the given range", () => {
      const tree = parser.parse("x10 + 1000");
      const sumNode = tree.rootNode.firstChild;
      assert.equal("identifier", sumNode.descendantForIndex(1, 2).type);
      assert.equal("+", sumNode.descendantForIndex(4, 4).type);
    });
  });

  describe(".descendantForPosition(min, max)", () => {
    it("returns the smallest node that spans the given range", () => {
      const tree = parser.parse("x10 + 1000");
      const sumNode = tree.rootNode.firstChild.firstChild;

      assert.equal(
        "identifier",
        sumNode.descendantForPosition(
          { row: 0, column: 1 },
          { row: 0, column: 2 }
        ).type
      );

      assert.equal(
        "+",
        sumNode.descendantForPosition({ row: 0, column: 4 }).type
      );

      assert.throws(() => {
        sumNode.descendantForPosition(1, {});
      }, /Point must be a {row, column} object/);

      assert.throws(() => {
        sumNode.descendantForPosition();
      }, /Point must be a {row, column} object/);
    });
  });

  describe(".namedDescendantForPosition(min, max)", () => {
    it("returns the smallest named node that spans the given range", () => {
      const tree = parser.parse("x10 + 1000");
      const sumNode = tree.rootNode.firstChild;

      assert.equal(
        sumNode.namedDescendantForPosition(
          { row: 0, column: 1 },
          { row: 0, column: 2 }
        ).type,
        "identifier",
      );

      assert.equal(
        sumNode.namedDescendantForPosition({ row: 0, column: 4 }).type,
        'binary_expression'
      );
    });
  });

  describe('.descendantsOfType(type, min, max)', () => {
    it('finds all of the descendants of the given type in the given range', () => {
      const tree = parser.parse("a + 1 * b * 2 + c + 3");
      const outerSum = tree.rootNode.firstChild.firstChild;
      let descendants = outerSum.descendantsOfType('number', {row: 0, column: 2}, {row: 0, column: 15})
      assert.deepEqual(
        descendants.map(node => node.startIndex),
        [4, 12]
      );

      descendants = outerSum.descendantsOfType('identifier', {row: 0, column: 2}, {row: 0, column: 15})
      assert.deepEqual(
        descendants.map(node => node.startIndex),
        [8]
      );

      descendants = outerSum.descendantsOfType('identifier', {row: 0, column: 0}, {row: 0, column: 30})
      assert.deepEqual(
        descendants.map(node => node.startIndex),
        [0, 8, 16]
      );

      descendants = outerSum.descendantsOfType('number', {row: 0, column: 0}, {row: 0, column: 30})
      assert.deepEqual(
        descendants.map(node => node.startIndex),
        [4, 12, 20]
      );

      descendants = outerSum.descendantsOfType(
        ['identifier', 'number'],
        {row: 0, column: 0},
        {row: 0, column: 30}
      )
      assert.deepEqual(
        descendants.map(node => node.startIndex),
        [0, 4, 8, 12, 16, 20]
      );

      descendants = outerSum.descendantsOfType('number')
      assert.deepEqual(
        descendants.map(node => node.startIndex),
        [4, 12, 20]
      );

      descendants = outerSum.firstChild.descendantsOfType('number', {row: 0, column: 0}, {row: 0, column: 30})
      assert.deepEqual(
        descendants.map(node => node.startIndex),
        [4, 12]
      );
    })
  });

  describe('.closest(type)', () => {
    it('returns the closest ancestor of the given type', () => {
      const tree = parser.parse("a(b + -d.e)");
      const property = tree.rootNode.descendantForIndex("a(b + -d.".length);
      assert.equal(property.type, 'property_identifier');

      const unary = property.closest('unary_expression')
      assert.equal(unary.type, 'unary_expression')
      assert.equal(unary.startIndex, 'a(b + '.length)
      assert.equal(unary.endIndex, 'a(b + -d.e'.length)

      const sum = property.closest(['binary_expression', 'call_expression'])
      assert.equal(sum.type, 'binary_expression')
      assert.equal(sum.startIndex, 2)
      assert.equal(sum.endIndex, 'a(b + -d.e'.length)
    });

    it('throws an exception when an invalid argument is given', () => {
      const tree = parser.parse("a + 1 * b * 2 + c + 3");
      const number = tree.rootNode.descendantForIndex(4)

      assert.throws(() => number.closest({a: 1}), /Argument must be a string or array of strings/)
    });
  });

  describe(".firstChildForIndex(index)", () => {
    it("returns the first child that extends beyond the given index", () => {
      const tree = parser.parse("x10 + 1000");
      const sumNode = tree.rootNode.firstChild.firstChild;

      assert.equal("identifier", sumNode.firstChildForIndex(0).type);
      assert.equal("identifier", sumNode.firstChildForIndex(1).type);
      assert.equal("+", sumNode.firstChildForIndex(3).type);
      assert.equal("number", sumNode.firstChildForIndex(5).type);
    });
  });

  describe(".firstNamedChildForIndex(index)", () => {
    it("returns the first child that extends beyond the given index", () => {
      const tree = parser.parse("x10 + 1000");
      const sumNode = tree.rootNode.firstChild.firstChild;

      assert.equal("identifier", sumNode.firstNamedChildForIndex(0).type);
      assert.equal("identifier", sumNode.firstNamedChildForIndex(1).type);
      assert.equal("number", sumNode.firstNamedChildForIndex(3).type);
    });
  });

  describe(".hasError()", () => {
    it("returns true if the node contains an error", () => {
      const tree = parser.parse("1 + 2 * * 3");
      const node = tree.rootNode;
      assert.equal(
        node.toString(),
        '(program (expression_statement (binary_expression left: (number) right: (binary_expression left: (number) (ERROR) right: (number)))))'
      );

      const sum = node.firstChild.firstChild;
      assert(sum.hasError());
      assert(!sum.children[0].hasError());
      assert(!sum.children[1].hasError());
      assert(sum.children[2].hasError());
    });
  });

  describe(".isMissing()", () => {
    it("returns true if the node is missing from the source and was inserted via error recovery", () => {
      const tree = parser.parse("(2 ||)");
      const node = tree.rootNode;
      assert.equal(
        node.toString(),
        "(program (expression_statement (parenthesized_expression (binary_expression left: (number) right: (MISSING identifier)))))"
      );

      const sum = node.firstChild.firstChild.firstNamedChild;
      assert.equal(sum.type, 'binary_expression')
      assert(sum.hasError());
      assert(!sum.children[0].isMissing());
      assert(!sum.children[1].isMissing());
      assert(sum.children[2].isMissing());
    });
  });

  describe(".text", () => {
    Object.entries({
      '.parse(String)': (parser, src) => parser.parse(src),
      '.parse(Function)': (parser, src) =>
        parser.parse(offset => src.substr(offset, 4)),
      '.parseTextBuffer': (parser, src) =>
        parser.parseTextBuffer(new TextBuffer(src)),
      '.parseTextBufferSync': (parser, src) =>
        parser.parseTextBufferSync(new TextBuffer(src))
    }).forEach(([method, parse]) =>
      it(`returns the text of a node generated by ${method}`, async () => {
        const src = "α0 / b👎c👎"
        const [numeratorSrc, denominatorSrc] = src.split(/\s*\/\s+/)
        const tree = await parse(parser, src)
        const quotientNode = tree.rootNode.firstChild.firstChild;
        const [numerator, slash, denominator] = quotientNode.children;

        assert.equal(src, tree.rootNode.text,          'root node text');
        assert.equal(denominatorSrc, denominator.text, 'denominator text');
        assert.equal(src, quotientNode.text,           'quotient text');
        assert.equal(numeratorSrc, numerator.text,     'numerator text');
        assert.equal('/', slash.text,                  '"/" text');
      })
    )
  })

  describe(".childNodeForFieldName", () => {
    it(`finds a child node by name`, async () => {
      const tree = parser.parse(`1 + 2`);

      const node = tree.rootNode?.firstChild?.firstChild;
      assert.equal(node?.childNodeForFieldName("left").text, '1');
      assert.equal(node?.childNodeForFieldName("right").text, '2');
    });
  })
});
