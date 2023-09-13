const Parser = require("..");
const Javascript = require("tree-sitter-javascript");
const jsParser = new Parser();
jsParser.setLanguage(Javascript);

describe("Jest test 1", () => {
  it("should work", () => {
    const code = jsParser.parse(`
      const Parser = require(".");
      const Javascript = require("tree-sitter-javascript");
      const jsParser = new Parser();
    `)
    const output = code.rootNode.toString()
    expect(output).toBe('(program (lexical_declaration (variable_declarator name: (identifier) value: (call_expression function: (identifier) arguments: (arguments (string))))) (lexical_declaration (variable_declarator name: (identifier) value: (call_expression function: (identifier) arguments: (arguments (string))))) (lexical_declaration (variable_declarator name: (identifier) value: (new_expression constructor: (identifier) arguments: (arguments)))))');
  })
})
