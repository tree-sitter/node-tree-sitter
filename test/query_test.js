const fs = require("fs");
const Parser = require("..");
const JavaScript = require("tree-sitter-javascript");
const { assert } = require("chai");
const {Query, QueryCursor} = Parser

describe("Query", () => {

  const parser = new Parser();
  parser.setLanguage(JavaScript);

  describe("new", () => {
    it("works with buffer", () => {
      const query = new Query(JavaScript, `
        (function_declaration name: (identifier) @fn-def)
        (call_expression function: (identifier) @fn-ref)
      `);
    });

    it("works with string", () => {
      const query = new Query(JavaScript, Buffer.from(`
        (function_declaration name: (identifier) @fn-def)
        (call_expression function: (identifier) @fn-ref)
      `));
    });
  });

  describe(".exec", () => {
    // XXX: fix setting up this test
    it.skip("works", () => {
      const tree = parser.parse("function one() { two(); function three() {} }");
      const query = new Query(JavaScript, `
        (function_declaration name: (identifier) @fn-def)
        (call_expression function: (identifier) @fn-ref)
      `);
      const matches = [];
      const cursor = new QueryCursor();
      cursor.exec(query, tree, (patternName, node) => {
        console.log(patternName, node);
      });
      assert.deepEqual(formatMatches(matches), [
        { pattern: 0, captures: [{ name: "fn-def", text: "one" }] },
        { pattern: 1, captures: [{ name: "fn-ref", text: "two" }] },
        { pattern: 0, captures: [{ name: "fn-def", text: "three" }] },
      ]);
    });
  });

  describe.skip(".matches", () => {
    it("returns all of the matches for the given query", () => {
      const tree = parser.parse("function one() { two(); function three() {} }");
      const query = new Query(JavaScript, `
        (function_declaration name: (identifier) @fn-def)
        (call_expression function: (identifier) @fn-ref)
      `);
      const matches = query.matches(tree.rootNode);
      assert.deepEqual(formatMatches(matches), [
        { pattern: 0, captures: [{ name: "fn-def", text: "one" }] },
        { pattern: 1, captures: [{ name: "fn-ref", text: "two" }] },
        { pattern: 0, captures: [{ name: "fn-def", text: "three" }] },
      ]);
    });

    it("can search in a specified ranges", () => {
      const tree = parser.parse("[a, b,\nc, d,\ne, f,\ng, h]");
      const query = new Query(JavaScript, "(identifier) @element");
      const matches = query.matches(
        tree.rootNode,
        { row: 1, column: 1 },
        { row: 3, column: 1 }
      );
      assert.deepEqual(formatMatches(matches), [
        { pattern: 0, captures: [{ name: "element", text: "d" }] },
        { pattern: 0, captures: [{ name: "element", text: "e" }] },
        { pattern: 0, captures: [{ name: "element", text: "f" }] },
        { pattern: 0, captures: [{ name: "element", text: "g" }] },
      ]);
    });
  });

  describe.skip(".captures", () => {
    it("returns all of the captures for the given query, in order", () => {
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
      assert.deepEqual(formatCaptures(captures), [
        { name: "method.def", text: "bc" },
        { name: "delimiter", text: ":" },
        { name: "method.alias", text: "de" },
        { name: "function.def", text: "fg" },
        { name: "operator", text: "=" },
        { name: "function.alias", text: "hi" },
        { name: "method.def", text: "jk" },
        { name: "delimiter", text: ":" },
        { name: "method.alias", text: "lm" },
        { name: "function.def", text: "no" },
        { name: "operator", text: "=" },
        { name: "function.alias", text: "pq" },
      ]);
    });

    it("handles conditions that compare the text of capture to literal strings", () => {
      const tree = parser.parse(`
        const ab = require('./ab');
        new Cd(EF);
      `);

      const query = new Query(JavaScript, `
        (identifier) @variable
        ((identifier) @function.builtin
         (#eq? @function.builtin "require"))
        ((identifier) @constructor
         (#match? @constructor "^[A-Z]"))
        ((identifier) @constant
         (#match? @constant "^[A-Z]{2,}$"))
      `);

      const captures = query.captures(tree.rootNode);
      assert.deepEqual(formatCaptures(captures), [
        { name: "variable", text: "ab" },
        { name: "variable", text: "require" },
        { name: "function.builtin", text: "require" },
        { name: "variable", text: "Cd" },
        { name: "constructor", text: "Cd" },
        { name: "variable", text: "EF" },
        { name: "constructor", text: "EF" },
        { name: "constant", text: "EF" },
      ]);
    });

    it("handles conditions that compare the text of capture to each other", () => {
      const tree = parser.parse(`
        ab = abc + 1;
        def = de + 1;
        ghi = ghi + 1;
      `);

      const query = new Query(JavaScript, `
        (
          (assignment_expression
            left: (identifier) @id1
            right: (binary_expression
              left: (identifier) @id2))
          (#eq? @id1 @id2)
        )
      `);

      const captures = query.captures(tree.rootNode);
      assert.deepEqual(formatCaptures(captures), [
        { name: "id1", text: "ghi" },
        { name: "id2", text: "ghi" },
      ]);
    });

    it("handles patterns with properties", () => {
      const tree = parser.parse(`a(b.c);`);
      const query = new Query(JavaScript, `
        ((call_expression (identifier) @func)
         (#set! foo)
         (#set! bar baz))
        ((property_identifier) @prop
         (#is? foo)
         (#is-not? bar baz))
      `);

      const captures = query.captures(tree.rootNode);
      assert.deepEqual(formatCaptures(captures), [
        { name: "func", text: "a", setProperties: { foo: null, bar: "baz" } },
        {
          name: "prop",
          text: "c",
          assertedProperties: { foo: null },
          refutedProperties: { bar: "baz" },
        },
      ]);
    });
  });
});

function formatMatches(matches) {
  return matches.map(({ pattern, captures }) => ({
    pattern,
    captures: formatCaptures(captures),
  }));
}

function formatCaptures(captures) {
  return captures.map((c) => {
    const node = c.node;
    delete c.node;
    c.text = node.text;
    return c;
  });
}
