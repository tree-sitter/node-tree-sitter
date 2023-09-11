const Parser = require("..");
const Rust = require("tree-sitter-rust");
const { assert } = require("chai");
const {LookaheadIterator} = Parser;

describe("LookaheadIterator", ()  => {

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
      assert.equal(cursor.currentNode.grammarName,  "identifier");
      assert.notEqual(cursor.currentNode.grammarId, cursor.currentNode.typeId);

      const expectedSymbols = ["identifier", "block_comment", "line_comment"]
      const lookahead = new LookaheadIterator(Rust, nextState);
      assert.deepEqual(lookahead.iterNames(), expectedSymbols);

      assert(lookahead.resetState(nextState));
      assert.deepEqual(lookahead.iterNames(), expectedSymbols);

      assert(lookahead.reset(Rust, nextState));
      assert.deepEqual(lookahead.iterNames(), expectedSymbols);
    });
  });
});
