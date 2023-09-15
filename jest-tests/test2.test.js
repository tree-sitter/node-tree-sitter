const Parser = require("..");
const constants = require("./constants");
const Javascript = require("tree-sitter-javascript");
const jsParser = new Parser();
jsParser.setLanguage(Javascript);

describe("Jest test 1", () => {
  it("should work", () => {
    const code = jsParser.parse(constants.INPUT)
    if (code.rootNode) {
      const output = code.rootNode.toString()
      expect(output).toBe(constants.OUTPUT);
    }
  })
})
