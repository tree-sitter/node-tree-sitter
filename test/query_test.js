const fs = require("fs");
const Parser = require("..");
const JavaScript = require("tree-sitter-javascript");
const { assert } = require("chai");
const {Query, QueryCursor} = Parser

const sourceBuffer = fs.readFileSync(require.resolve('tree-sitter-javascript/queries/highlights.scm'));
const sourceString = sourceBuffer.toString();

const code = `
const x = 42;
console.log(x);
`;

describe("Query", () => {

  describe("new", () => {
    it("works with buffer", () => {
      const query = new Query(JavaScript, sourceBuffer);
    });

    it("works with string", () => {
      const query = new Query(JavaScript, sourceString);
    });
  });
});

describe("QueryCursor", () => {

  describe(".exec", () => {
    it("works", () => {
      const parser = new Parser();
      parser.setLanguage(JavaScript);
      const tree = parser.parse(code);
      const query = new Query(JavaScript, sourceBuffer);
      const cursor = new QueryCursor();
      cursor.exec(query, tree, (patternName, node) => {
        console.log(patternName, node);
      });
    });
  });
});
