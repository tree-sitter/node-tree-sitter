/** @type {typeof import('tree-sitter')} */
const Parser = require("../index.js");

const C = require('tree-sitter-c');
const EmbeddedTemplate = require('tree-sitter-embedded-template');
const JavaScript = require('tree-sitter-javascript');
const JSON = require('tree-sitter-json');
const Python = require('tree-sitter-python');
const assert = require('node:assert');
const { beforeEach, describe, it } = require('node:test');

const JSON_EXAMPLE = `

[
  123,
  false,
  {
    "x": null
  }
]
`

/**
 * @param {import('tree-sitter').Tree} tree
 */
function getAllNodes(tree) {
  const result = [];
  let visitedChildren = false;
  let cursor = tree.walk();
  while (true) {
    if (!visitedChildren) {
      result.push(cursor.currentNode);
      if (!cursor.gotoFirstChild()) {
        visitedChildren = true;
      }
    } else if (cursor.gotoNextSibling()) {
      visitedChildren = false;
    } else if (!cursor.gotoParent()) {
      break;
    }
  }
  return result;
}

describe("Node", () => {
  /** @type {import('tree-sitter')} */
  let parser;

  beforeEach(() => {
    parser = new Parser();
    parser.setLanguage(JavaScript);
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
      // @ts-ignore
      assert.deepEqual(classNode.fields, ['bodyNode', 'decoratorNodes', 'nameNode'])

      // @ts-ignore
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

  describe(".child", () => {
    it("returns the child at the given index", () => {
      parser.setLanguage(JSON);
      const tree = parser.parse(JSON_EXAMPLE);
      const arrayNode = tree.rootNode.child(0);

      assert.equal(arrayNode.type, "array");
      assert.equal(arrayNode.namedChildCount, 3);
      assert.equal(arrayNode.startIndex, JSON_EXAMPLE.indexOf("["));
      assert.equal(arrayNode.endIndex, JSON_EXAMPLE.indexOf("]") + 1);
      assert.deepEqual(arrayNode.startPosition, { row: 2, column: 0 });
      assert.deepEqual(arrayNode.endPosition, { row: 8, column: 1 });
      assert.equal(arrayNode.childCount, 7);

      const leftBracketNode = arrayNode.child(0);
      const numberNode = arrayNode.child(1);
      const commaNode1 = arrayNode.child(2);
      const falseNode = arrayNode.child(3);
      const commaNode2 = arrayNode.child(4);
      const objectNode = arrayNode.child(5);
      const rightBracketNode = arrayNode.child(6);

      assert.equal(leftBracketNode.type, "[");
      assert.equal(numberNode.type, "number");
      assert.equal(commaNode1.type, ",");
      assert.equal(falseNode.type, "false");
      assert.equal(commaNode2.type, ",");
      assert.equal(objectNode.type, "object");
      assert.equal(rightBracketNode.type, "]");

      assert(!leftBracketNode.isNamed);
      assert(numberNode.isNamed);
      assert(!commaNode1.isNamed);
      assert(falseNode.isNamed);
      assert(!commaNode2.isNamed);
      assert(objectNode.isNamed);
      assert(!rightBracketNode.isNamed);

      assert.equal(numberNode.startIndex, JSON_EXAMPLE.indexOf("123"));
      assert.equal(numberNode.endIndex, JSON_EXAMPLE.indexOf("123") + 3);
      assert.deepEqual(numberNode.startPosition, { row: 3, column: 2 });
      assert.deepEqual(numberNode.endPosition, { row: 3, column: 5 });

      assert.equal(falseNode.startIndex, JSON_EXAMPLE.indexOf("false"));
      assert.equal(falseNode.endIndex, JSON_EXAMPLE.indexOf("false") + 5);
      assert.deepEqual(falseNode.startPosition, { row: 4, column: 2 });
      assert.deepEqual(falseNode.endPosition, { row: 4, column: 7 });

      assert.equal(objectNode.startIndex, JSON_EXAMPLE.indexOf("{"));
      assert.equal(objectNode.endIndex, JSON_EXAMPLE.indexOf("}") + 1);
      assert.deepEqual(objectNode.startPosition, { row: 5, column: 2 });
      assert.deepEqual(objectNode.endPosition, { row: 7, column: 3 });

      assert.equal(objectNode.childCount, 3);
      const leftBraceNode = objectNode.child(0);
      const pairNode = objectNode.child(1);
      const rightBraceNode = objectNode.child(2);

      assert.equal(leftBraceNode.type, "{");
      assert.equal(pairNode.type, "pair");
      assert.equal(rightBraceNode.type, "}");

      assert(!leftBraceNode.isNamed);
      assert(pairNode.isNamed);
      assert(!rightBraceNode.isNamed);

      assert.equal(pairNode.startIndex, JSON_EXAMPLE.indexOf('"x"'));
      assert.equal(pairNode.endIndex, JSON_EXAMPLE.indexOf("null") + 4);
      assert.deepEqual(pairNode.startPosition, { row: 6, column: 4 });
      assert.deepEqual(pairNode.endPosition, { row: 6, column: 13 });

      assert.equal(pairNode.childCount, 3);
      const stringNode = pairNode.child(0);
      const colonNode = pairNode.child(1);
      const nullNode = pairNode.child(2);

      assert.equal(stringNode.type, "string");
      assert.equal(colonNode.type, ":");
      assert.equal(nullNode.type, "null");

      assert(stringNode.isNamed);
      assert(!colonNode.isNamed);
      assert(nullNode.isNamed);

      assert.equal(stringNode.startIndex, JSON_EXAMPLE.indexOf('"x"'));
      assert.equal(stringNode.endIndex, JSON_EXAMPLE.indexOf('"x"') + 3);
      assert.deepEqual(stringNode.startPosition, { row: 6, column: 4 });
      assert.deepEqual(stringNode.endPosition, { row: 6, column: 7 });

      assert.equal(nullNode.startIndex, JSON_EXAMPLE.indexOf("null"));
      assert.equal(nullNode.endIndex, JSON_EXAMPLE.indexOf("null") + 4);
      assert.deepEqual(nullNode.startPosition, { row: 6, column: 9 });
      assert.deepEqual(nullNode.endPosition, { row: 6, column: 13 });

      assert.equal(stringNode.parent, pairNode);
      assert.equal(nullNode.parent, pairNode);
      assert.equal(pairNode.parent, objectNode);
      assert.equal(numberNode.parent, arrayNode);
      assert.equal(falseNode.parent, arrayNode);
      assert.equal(objectNode.parent, arrayNode);
      assert.equal(arrayNode.parent, tree.rootNode);
      assert.equal(tree.rootNode.parent, null);
    });
  });

  describe(".namedChild", () => {
    it("returns the named child at the given index", () => {
      parser.setLanguage(JSON);
      const tree = parser.parse(JSON_EXAMPLE);
      const arrayNode = tree.rootNode.namedChild(0);

      const numberNode = arrayNode.namedChild(0);
      const falseNode = arrayNode.namedChild(1);
      const objectNode = arrayNode.namedChild(2);

      assert.equal(numberNode.type, "number");
      assert.equal(numberNode.startIndex, JSON_EXAMPLE.indexOf("123"));
      assert.equal(numberNode.endIndex, JSON_EXAMPLE.indexOf("123") + 3);
      assert.deepEqual(numberNode.startPosition, { row: 3, column: 2 });
      assert.deepEqual(numberNode.endPosition, { row: 3, column: 5 });

      assert.equal(falseNode.type, "false");
      assert.equal(falseNode.startIndex, JSON_EXAMPLE.indexOf("false"));
      assert.equal(falseNode.endIndex, JSON_EXAMPLE.indexOf("false") + 5);
      assert.deepEqual(falseNode.startPosition, { row: 4, column: 2 });
      assert.deepEqual(falseNode.endPosition, { row: 4, column: 7 });

      assert.equal(objectNode.type, "object");
      assert.equal(objectNode.startIndex, JSON_EXAMPLE.indexOf("{"));
      assert.deepEqual(objectNode.startPosition, { row: 5, column: 2 });
      assert.deepEqual(objectNode.endPosition, { row: 7, column: 3 });

      assert.equal(objectNode.namedChildCount, 1);

      const pairNode = objectNode.namedChild(0);
      assert.equal(pairNode.type, "pair");
      assert.equal(pairNode.startIndex, JSON_EXAMPLE.indexOf('"x"'));
      assert.equal(pairNode.endIndex, JSON_EXAMPLE.indexOf("null") + 4);
      assert.deepEqual(pairNode.startPosition, { row: 6, column: 4 });
      assert.deepEqual(pairNode.endPosition, { row: 6, column: 13 });

      const stringNode = pairNode.namedChild(0);
      const nullNode = pairNode.namedChild(1);

      assert.equal(stringNode.startIndex, JSON_EXAMPLE.indexOf('"x"'));
      assert.equal(stringNode.endIndex, JSON_EXAMPLE.indexOf('"x"') + 3);
      assert.deepEqual(stringNode.startPosition, { row: 6, column: 4 });
      assert.deepEqual(stringNode.endPosition, { row: 6, column: 7 });

      assert.equal(nullNode.startIndex, JSON_EXAMPLE.indexOf("null"));
      assert.equal(nullNode.endIndex, JSON_EXAMPLE.indexOf("null") + 4);
      assert.deepEqual(nullNode.startPosition, { row: 6, column: 9 });
      assert.deepEqual(nullNode.endPosition, { row: 6, column: 13 });

      assert.equal(stringNode.parent, pairNode);
      assert.equal(nullNode.parent, pairNode);
      assert.equal(pairNode.parent, objectNode);
      assert.equal(numberNode.parent, arrayNode);
      assert.equal(falseNode.parent, arrayNode);
      assert.equal(objectNode.parent, arrayNode);
      assert.equal(arrayNode.parent, tree.rootNode);
      assert.equal(tree.rootNode.parent, null);
    });
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

  describe(".childrenForFieldName", () => {
    it("returns an array of child nodes for the given field name", () => {
      parser.setLanguage(Python);
      const source = `
        if one:
            a()
        elif two:
            b()
        elif three:
            c()
        elif four:
    d()`

      const tree = parser.parse(source);
      const node = tree.rootNode.firstChild;
      assert.equal(node.type, 'if_statement');
      const alternatives = node.childrenForFieldName('alternative');
      const alternative_texts = alternatives.map(n => {
        const condition = n.childForFieldName('condition');
        return source.slice(condition.startIndex, condition.endIndex);
      });
      assert.deepEqual(alternative_texts, ['two', 'three', 'four']);
    });
  });

  describe(".childForFieldName", () => {
    it("checks the parent node", () => {
      const tree = parser.parse("foo(a().b[0].c.d.e())");
      const callNode = tree.rootNode.firstNamedChild.firstNamedChild;
      assert.equal(callNode.type, "call_expression");

      // Regression test - when a field points to a hidden node (in this case, `_expression`)
      // the hidden node should not be added to the node parent cache.
      assert.equal(
        callNode.childForFieldName("function").parent,
        callNode
      );
    });

    it("checks that extra hidden children are skipped", () => {
      parser.setLanguage(Python);
      const tree = parser.parse("while a:\n  pass");
      const whileNode = tree.rootNode.firstChild;
      assert.equal(whileNode.type, "while_statement");
      assert.equal(whileNode.childForFieldName("body"), whileNode.child(3));
    });
  });

  describe(".fieldNameForChild", () => {
    it("returns the field name for the given child node", () => {
      parser.setLanguage(C);
      const tree = parser.parse("int w = x + y;");
      const translation_unit_node = tree.rootNode;
      const declaration_node = translation_unit_node.firstNamedChild;

      const binary_expression_node = declaration_node
        .childForFieldName("declarator")
        .childForFieldName("value");

      assert.equal(binary_expression_node.fieldNameForChild(0), "left");
      assert.equal(binary_expression_node.fieldNameForChild(1), "operator");
      assert.equal(binary_expression_node.fieldNameForChild(2), "right");
      // Negative test - Not a valid child index
      assert.equal(binary_expression_node.fieldNameForChild(3), null);
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
        // @ts-ignore
        sumNode.descendantForIndex(1, {});
      }, /Character index must be a number/);

      assert.throws(() => {
        // @ts-ignore
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
        // @ts-ignore
        sumNode.descendantForPosition(1, {});
      }, /Point must be a {row, column} object/);

      assert.throws(() => {
        // @ts-ignore
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
      let descendants = outerSum.descendantsOfType('number', { row: 0, column: 2 }, { row: 0, column: 15 })
      assert.deepEqual(
        descendants.map(node => node.startIndex),
        [4, 12]
      );

      descendants = outerSum.descendantsOfType('identifier', { row: 0, column: 2 }, { row: 0, column: 15 })
      assert.deepEqual(
        descendants.map(node => node.startIndex),
        [8]
      );

      descendants = outerSum.descendantsOfType('identifier', { row: 0, column: 0 }, { row: 0, column: 30 })
      assert.deepEqual(
        descendants.map(node => node.startIndex),
        [0, 8, 16]
      );

      descendants = outerSum.descendantsOfType('number', { row: 0, column: 0 }, { row: 0, column: 30 })
      assert.deepEqual(
        descendants.map(node => node.startIndex),
        [4, 12, 20]
      );

      descendants = outerSum.descendantsOfType(
        ['identifier', 'number'],
        { row: 0, column: 0 },
        { row: 0, column: 30 }
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

      descendants = outerSum.firstChild.descendantsOfType('number', { row: 0, column: 0 }, { row: 0, column: 30 })
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

      // @ts-ignore
      assert.throws(() => number.closest({ a: 1 }), /Argument must be a string or array of strings/)
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

  describe(".hasError", () => {
    it("returns true if the node contains an error", () => {
      const tree = parser.parse("1 + 2 * * 3");
      const node = tree.rootNode;
      assert.equal(
        node.toString(),
        '(program (expression_statement (binary_expression left: (number) right: (binary_expression left: (number) (ERROR) right: (number)))))'
      );

      const sum = node.firstChild.firstChild;
      assert(sum.hasError);
      assert(!sum.children[0].hasError);
      assert(!sum.children[1].hasError);
      assert(sum.children[2].hasError);
    });
  });

  describe(".isMissing", () => {
    it("returns true if the node is missing from the source and was inserted via error recovery", () => {
      const tree = parser.parse("(2 ||)");
      const node = tree.rootNode;
      assert.equal(
        node.toString(),
        "(program (expression_statement (parenthesized_expression (binary_expression left: (number) right: (MISSING identifier)))))"
      );

      const sum = node.firstChild.firstChild.firstNamedChild;
      assert.equal(sum.type, 'binary_expression')
      assert(sum.hasError);
      assert(!sum.children[0].isMissing);
      assert(!sum.children[1].isMissing);
      assert(sum.children[2].isMissing);
    });
  });

  describe(".isExtra", () => {
    it("returns true if the node is an extra node like comments", () => {
      const tree = parser.parse("foo(/* hi */);");
      const node = tree.rootNode;
      const comment_node = node.descendantForIndex(7, 7);

      assert.equal(node.type, "program");
      assert.equal(comment_node.type, "comment");
      assert(!node.isExtra);
      assert(comment_node.isExtra);
    });
  });

  describe(".toString()", () => {
    it("returns a string representation of the node", () => {
      const tree = parser.parse("if (a) b");
      const rootNode = tree.rootNode;
      const ifNode = rootNode.descendantForIndex(0, 0);
      const parenNode = rootNode.descendantForIndex(3, 3);
      const identifierNode = rootNode.descendantForIndex(4, 4);
      assert.equal(ifNode.type, "if");
      assert.equal(ifNode.toString(), "(\"if\")");
      assert.equal(parenNode.type, "(");
      assert.equal(parenNode.toString(), "(\"(\")");
      assert.equal(identifierNode.type, "identifier");
      assert.equal(identifierNode.toString(), "(identifier)");
    });
  });

  describe(".text", () => {
    Object.entries({
      '.parse(String)': (parser, src) => parser.parse(src),
      '.parse(Function)': (parser, src) =>
        parser.parse(offset => src.substr(offset, 4)),
    }).forEach(([method, parse]) =>
      it(`returns the text of a node generated by ${method}`, async () => {
        const src = "α0 / b👎c👎"
        const [numeratorSrc, denominatorSrc] = src.split(/\s*\/\s+/)
        const tree = await parse(parser, src)
        const quotientNode = tree.rootNode.firstChild.firstChild;
        const [numerator, slash, denominator] = quotientNode.children;

        assert.equal(src, tree.rootNode.text, 'root node text');
        assert.equal(denominatorSrc, denominator.text, 'denominator text');
        assert.equal(src, quotientNode.text, 'quotient text');
        assert.equal(numeratorSrc, numerator.text, 'numerator text');
        assert.equal('/', slash.text, '"/" text');
      })
    )
  });

  describe(".descendantCount", () => {
    it("returns the number of descendants", () => {
      parser.setLanguage(JSON);
      const tree = parser.parse(JSON_EXAMPLE);
      const valueNode = tree.rootNode;
      const allNodes = getAllNodes(tree);

      assert.equal(valueNode.descendantCount, allNodes.length);

      const cursor = tree.walk();
      for (let i = 0; i < allNodes.length; i++) {
        const node = allNodes[i];
        cursor.gotoDescendant(i)
        assert.equal(cursor.currentNode, node, `index ${i}`);
      }

      for (let i = allNodes.length - 1; i >= 0; i--) {
        const node = allNodes[i];
        cursor.gotoDescendant(i)
        assert.equal(cursor.currentNode, node, `rev index ${i}`);
      }
    });

    it("tests a single node tree", () => {
      parser.setLanguage(EmbeddedTemplate);
      const tree = parser.parse("hello");

      const nodes = getAllNodes(tree);
      assert.equal(nodes.length, 2);
      assert.equal(tree.rootNode.descendantCount, 2);

      const cursor = tree.walk();

      cursor.gotoDescendant(0);
      assert.equal(cursor.currentDepth, 0);
      assert.equal(cursor.currentNode, nodes[0]);

      cursor.gotoDescendant(1);
      assert.equal(cursor.currentDepth, 1);
      assert.equal(cursor.currentNode, nodes[1]);
    });
  });

  describe(".descendantFor{Index, Position}", () => {
    it("returns the descendant at the given index", () => {
      parser.setLanguage(JSON);
      const tree = parser.parse(JSON_EXAMPLE);
      const arrayNode = tree.rootNode;

      // Leaf node exactly matches the given bounds - index query
      const colonIndex = JSON_EXAMPLE.indexOf(":");
      let colonNode = arrayNode.descendantForIndex(colonIndex, colonIndex + 1);
      assert.equal(colonNode.type, ":");
      assert.equal(colonNode.startIndex, colonIndex);
      assert.equal(colonNode.endIndex, colonIndex + 1);
      assert.deepEqual(colonNode.startPosition, { row: 6, column: 7 });
      assert.deepEqual(colonNode.endPosition, { row: 6, column: 8 });

      // Leaf node exactly matches the given bounds - point query
      colonNode = arrayNode.descendantForPosition({ row: 6, column: 7 }, { row: 6, column: 8 });
      assert.equal(colonNode.type, ":");
      assert.equal(colonNode.startIndex, colonIndex);
      assert.equal(colonNode.endIndex, colonIndex + 1);
      assert.deepEqual(colonNode.startPosition, { row: 6, column: 7 });
      assert.deepEqual(colonNode.endPosition, { row: 6, column: 8 });

      // The given point is between two adjacent leaf nodes - index query
      colonNode = arrayNode.descendantForIndex(colonIndex, colonIndex);
      assert.equal(colonNode.type, ":");
      assert.equal(colonNode.startIndex, colonIndex);
      assert.equal(colonNode.endIndex, colonIndex + 1);
      assert.deepEqual(colonNode.startPosition, { row: 6, column: 7 });
      assert.deepEqual(colonNode.endPosition, { row: 6, column: 8 });

      // The given point is between two adjacent leaf nodes - point query
      colonNode = arrayNode.descendantForPosition({ row: 6, column: 7 }, { row: 6, column: 7 });
      assert.equal(colonNode.type, ":");
      assert.equal(colonNode.startIndex, colonIndex);
      assert.equal(colonNode.endIndex, colonIndex + 1);
      assert.deepEqual(colonNode.startPosition, { row: 6, column: 7 });
      assert.deepEqual(colonNode.endPosition, { row: 6, column: 8 });

      // Leaf node starts at the lower bound, ends after the upper bound - index query
      const stringIndex = JSON_EXAMPLE.indexOf('"x"');
      let stringNode = arrayNode.descendantForIndex(stringIndex, stringIndex + 2);
      assert.equal(stringNode.type, "string");
      assert.equal(stringNode.startIndex, stringIndex);
      assert.equal(stringNode.endIndex, stringIndex + 3);
      assert.deepEqual(stringNode.startPosition, { row: 6, column: 4 });
      assert.deepEqual(stringNode.endPosition, { row: 6, column: 7 });

      // Leaf node starts at the lower bound, ends after the upper bound - point query
      stringNode = arrayNode.descendantForPosition({ row: 6, column: 4 }, { row: 6, column: 6 });
      assert.equal(stringNode.type, "string");
      assert.equal(stringNode.startIndex, stringIndex);
      assert.equal(stringNode.endIndex, stringIndex + 3);
      assert.deepEqual(stringNode.startPosition, { row: 6, column: 4 });
      assert.deepEqual(stringNode.endPosition, { row: 6, column: 7 });

      // Leaf node starts before the lower bound, ends at the upper bound - index query
      const nullIndex = JSON_EXAMPLE.indexOf("null");
      let nullNode = arrayNode.descendantForIndex(nullIndex + 1, nullIndex + 4);
      assert.equal(nullNode.type, "null");
      assert.equal(nullNode.startIndex, nullIndex);
      assert.equal(nullNode.endIndex, nullIndex + 4);
      assert.deepEqual(nullNode.startPosition, { row: 6, column: 9 });
      assert.deepEqual(nullNode.endPosition, { row: 6, column: 13 });

      // Leaf node starts before the lower bound, ends at the upper bound - point query
      nullNode = arrayNode.descendantForPosition({ row: 6, column: 11 }, { row: 6, column: 13 });
      assert.equal(nullNode.type, "null");
      assert.equal(nullNode.startIndex, nullIndex);
      assert.equal(nullNode.endIndex, nullIndex + 4);
      assert.deepEqual(nullNode.startPosition, { row: 6, column: 9 });
      assert.deepEqual(nullNode.endPosition, { row: 6, column: 13 });

      // The bounds span multiple leaf nodes - return the smallest node that does span it.
      let pairNode = arrayNode.descendantForIndex(stringIndex + 2, stringIndex + 4);
      assert.equal(pairNode.type, "pair");
      assert.equal(pairNode.startIndex, stringIndex);
      assert.equal(pairNode.endIndex, stringIndex + 9);
      assert.deepEqual(pairNode.startPosition, { row: 6, column: 4 });
      assert.deepEqual(pairNode.endPosition, { row: 6, column: 13 });

      assert.equal(colonNode.parent, pairNode);

      // No leaf spans the given range - return the smallest node that does span it.
      pairNode = arrayNode.descendantForPosition({ row: 6, column: 6 }, { row: 6, column: 8 });
      assert.equal(pairNode.type, "pair");
      assert.equal(pairNode.startIndex, stringIndex);
      assert.equal(pairNode.endIndex, stringIndex + 9);
      assert.deepEqual(pairNode.startPosition, { row: 6, column: 4 });
      assert.deepEqual(pairNode.endPosition, { row: 6, column: 13 });
    });
  });

  describe('.rootNodeWithOffset', () => {
    it('returns the root node of the tree, offset by the given byte offset', () => {
      const tree = parser.parse('  if (a) b');
      const node = tree.rootNodeWithOffset(6, { row: 2, column: 2 });
      assert.equal(node.startIndex, 8);
      assert.equal(node.endIndex, 16);
      assert.deepEqual(node.startPosition, { row: 2, column: 4 });
      assert.deepEqual(node.endPosition, { row: 2, column: 12 });

      let child = node.firstChild.child(2);
      assert.equal(child.type, 'expression_statement');
      assert.equal(child.startIndex, 15);
      assert.equal(child.endIndex, 16);
      assert.deepEqual(child.startPosition, { row: 2, column: 11 });
      assert.deepEqual(child.endPosition, { row: 2, column: 12 });

      const cursor = node.walk();
      cursor.gotoFirstChild();
      cursor.gotoFirstChild();
      cursor.gotoNextSibling();
      child = cursor.currentNode;
      assert.equal(child.type, 'parenthesized_expression');
      assert.equal(child.startIndex, 11);
      assert.equal(child.endIndex, 14);
      assert.deepEqual(child.startPosition, { row: 2, column: 7 });
      assert.deepEqual(child.endPosition, { row: 2, column: 10 });
    });
  });

  describe("Numeric symbols respect simple aliases", () => {
    it("should ensure numeric symbol ids for an alias match the normal id", () => {
      parser.setLanguage(Python);

      // Example 1:
      // Python argument lists can contain "splat" arguments, which are not allowed within
      // other expressions. This includes `parenthesized_list_splat` nodes like `(*b)`. These
      // `parenthesized_list_splat` nodes are aliased as `parenthesized_expression`. Their numeric
      // `symbol`, aka `kind_id` should match that of a normal `parenthesized_expression`.
      const tree = parser.parse("(a((*b)))");
      const root = tree.rootNode;
      assert.equal(
        root.toString(),
        "(module (expression_statement (parenthesized_expression (call function: (identifier) arguments: (argument_list (parenthesized_expression (list_splat (identifier))))))))"
      );

      const outerExprNode = root.firstChild.firstChild;
      assert.equal(outerExprNode.type, "parenthesized_expression");

      const innerExprNode = outerExprNode.firstNamedChild.childForFieldName("arguments").firstNamedChild;
      assert.equal(innerExprNode.type, "parenthesized_expression");
      assert.equal(innerExprNode.typeId, outerExprNode.typeId);
    });
  });

  describe("Property accesses", () => {
    it("shouldn't segfault when accessing properties on the prototype", () => {
      const tree = parser.parse('2 + 2');
      const nodePrototype = Object.getPrototypeOf(tree.rootNode);
      // const nodePrototype = tree.rootNode.__proto__;

      const properties = [
        "type",
        "typeId",
        "isNamed",
        "isMissing",
        "hasChanges",
        "hasError",
        "isError",
        "text",
        "startPosition",
        "endPosition",
        "startIndex",
        "endIndex",
        "parent",
        "children",
        "namedChildren",
        "childCount",
        "namedChildCount",
        "firstChild",
        "firstNamedChild",
        "lastChild",
        "lastNamedChild",
        "nextSibling",
        "nextNamedSibling",
        "previousSibling",
        "previousNamedSibling",
      ];
      for (const property of properties) {
        assert.throws(() => { nodePrototype[property]; }, TypeError)
      }

      const methods = [
        "toString",
        "walk",
        // these take arguments but the "this" check happens first
        "child",
        "namedChild",
        "firstChildForIndex",
        "firstNamedChildForIndex",
        "namedDescendantForIndex",
        "descendantForIndex",
        "descendantsOfType",
        "namedDescendantForPosition",
        "descendantForPosition",
        "closest",
      ];
      for (const method of methods) {
        assert.throws(nodePrototype[method], TypeError)
      }
    });
  });
});
