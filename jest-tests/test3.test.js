const Parser = require("..");
const constants = require("./constants");
const Javascript = require("tree-sitter-javascript");
const jsParser = new Parser();
jsParser.setLanguage(Javascript);

describe("Jest test 1 duplicate", () => {
  it("should work", () => {
    const code = jsParser.parse(constants.INPUT)
    const output = code.rootNode.toString()
    expect(output).toBe(constants.OUTPUT);
  })
})
