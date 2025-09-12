/// <reference path="../tree-sitter.d.ts" />
/** @type {typeof import("tree-sitter")} */
const Parser = require("../index.js");
const Rust = require("tree-sitter-rust");
const assert = require('node:assert');
const { describe, it } = require('node:test');
const { LookaheadIterator } = Parser;

describe("LookaheadIterator", () => {

  const parser = new Parser();
  parser.setLanguage(Rust);

  describe("test lookahead iterator", () => {
    it("should iterate correctly", () => {
      const tree = parser.parse("struct Stuff {}");

      const cursor = tree.walk();

      assert(cursor.gotoFirstChild()); // struct
      assert(cursor.gotoFirstChild()); // struct keyword

      const nextState = cursor.currentNode.nextParseState;
      assert.notEqual(nextState, 0);
      assert(cursor.gotoNextSibling()); // type_identifier
      assert.equal(nextState, cursor.currentNode.parseState);
      assert.equal(cursor.currentNode.grammarType, "identifier");
      assert.notEqual(cursor.currentNode.grammarId, cursor.currentNode.typeId);

      const expectedSymbols = ["//", "/*", "identifier", "line_comment", "block_comment"]
      const lookahead = new LookaheadIterator(Rust, nextState);
      let symbols = Array.from(lookahead);
      assert.deepEqual(symbols, expectedSymbols);

      assert(lookahead.resetState(nextState));
      symbols = Array.from(lookahead);
      assert.deepEqual(symbols, expectedSymbols);

      assert(lookahead.reset(Rust, nextState));
      symbols = Array.from(lookahead);
      assert.deepEqual(symbols, expectedSymbols);
    });
  });
});
