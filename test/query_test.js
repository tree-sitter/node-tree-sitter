const fs = require("fs");
const Parser = require("..");
const C = require("tree-sitter-c");
const Java = require("tree-sitter-java");
const JavaScript = require("tree-sitter-javascript");
const JSON = require("tree-sitter-json");
const Python = require("tree-sitter-python");
const Ruby = require("tree-sitter-ruby");
const Rust = require("tree-sitter-rust");
const assert = require('node:assert');
const { describe, it } = require('node:test');
const { Query, QueryCursor } = Parser

describe("Query", () => {

  const parser = new Parser();
  parser.setLanguage(JavaScript);

  describe("errors", () => {
    it("errors on invalid syntax", () => {
      assert.doesNotThrow(() => new Query(JavaScript, "(if_statement)"))
      assert.doesNotThrow(() => new Query(JavaScript, "(if_statement condition:(parenthesized_expression (identifier)))"))
      assert.throws(() => new Query(JavaScript, "(if_statement"), "TSQueryErrorSyntax");
      assert.throws(() => new Query(JavaScript, "; comment 1\n; comment 2\n  (if_statement))"), "TSQueryErrorSyntax");
      assert.throws(() => new Query(JavaScript, "(if_statement identifier)"), "TSQueryErrorSyntax");
      assert.throws(() => new Query(JavaScript, "(if_statement condition:)"), "TSQueryErrorSyntax");
      assert.throws(() => new Query(JavaScript, "(identifier) \"h "), "TSQueryErrorSyntax");
      assert.throws(() => new Query(JavaScript, "((identifier) ()"), "TSQueryErrorSyntax");
      assert.throws(() => new Query(JavaScript, "((identifier) [])"), "TSQueryErrorSyntax");
      assert.throws(() => new Query(JavaScript, "((identifier) (#a)"), "TSQueryErrorSyntax");
      assert.throws(() => new Query(JavaScript, "((identifier) @x (#eq? @x a"), "TSQueryErrorSyntax");
      assert.throws(() => new Query(JavaScript, "(statement_block .)"), "TSQueryErrorSyntax");
      assert.throws(() => new Query(JavaScript, "(statement_block ! (if_statement))"), "TSQueryErrorSyntax");
      assert.throws(() => new Query(C, "(parameter_list [ \")\" @foo)"), "TSQueryErrorSyntax");
      assert.throws(() => new Query(Python, "[(unary_operator (_) @operand) (not_operator (_) @operand]"), "");
    });

    it("errors on invalid symbols", () => {
      assert.throws(() => new Query(JavaScript, "(clas)"), "TSQueryErrorNodeType");
      assert.throws(() => new Query(JavaScript, "(if_statement (arrayyyyy))"), "TSQueryErrorNodeType");
      assert.throws(() => new Query(JavaScript, "(if_statement condition: (non_existent3))"), "TSQueryErrorNodeType");
      assert.throws(() => new Query(JavaScript, "(if_statement condit: (identifier))"), "TSQueryErrorField");
      assert.throws(() => new Query(JavaScript, "(if_statement conditioning: (identifier))"), "TSQueryErrorField");
      assert.throws(() => new Query(JavaScript, "(if_statement !alternativ)"), "TSQueryErrorField");
      assert.throws(() => new Query(JavaScript, "(if_statement !alternatives)"), "TSQueryErrorField");
    });

    it("errors on invalid predicates", () => {
      assert.throws(() => new Query(JavaScript, "((identifier) @id (@id))"), "TSQueryErrorSyntax");
      assert.throws(() => new Query(JavaScript, "((identifier) @id (#eq? @id))"), "Wrong number of arguments");
      assert.throws(() => new Query(JavaScript, "((identifier) @id (#eq? @id @ok))"), "TSQueryErrorCapture");
    });

    it("errors on impossible patterns", () => {
      assert.throws(() => new Query(JavaScript, "(binary_expression left: (expression (identifier)) left: (expression (identifier)))"), "TSQueryErrorStructure");
      assert.doesNotThrow(() => new Query(JavaScript, "(function_declaration name: (identifier) (statement_block))"));
      assert.throws(() => new Query(JavaScript, "(function_declaration name: (statement_block))"), "TSQueryErrorStructure");
      assert.doesNotThrow(() => new Query(Ruby, "(call receiver: (call))"));
      assert.throws(() => new Query(Ruby, "(call receiver: (binary))"), "TSQueryErrorStructure");
      assert.doesNotThrow(() => new Query(JavaScript, `[
                (function_expression (identifier))
                (function_declaration (identifier))
                (generator_function_declaration (identifier))
            ]`));
      assert.throws(() => new Query(JavaScript, `[
                    (function_expression (identifier))
                    (function_declaration (object))
                    (generator_function_declaration (identifier))
                ]`), "TSQueryErrorStructure");
      assert.throws(() => new Query(JavaScript, "(identifier (identifier))"), "TSQueryErrorStructure");
      assert.throws(() => new Query(JavaScript, "(true (true))"), "TSQueryErrorStructure");
      assert.doesNotThrow(() => new Query(JavaScript, "(if_statement condition: (parenthesized_expression(expression) @cond))"));
      assert.throws(() => new Query(JavaScript, "(if_statement condition: (expression))"), "TSQueryErrorStructure");
    });
  });

  describe("new", () => {
    it("works with string", () => {
      assert.doesNotThrow(() => new Query(JavaScript, `
        (function_declaration name: (identifier) @fn-def)
        (call_expression function: (identifier) @fn-ref)
      `));
    });

    it("works with Buffer", () => {
      assert.doesNotThrow(() => new Query(JavaScript, Buffer.from(`
        (function_declaration name: (identifier) @fn-def)
        (call_expression function: (identifier) @fn-ref)
      `)));
    });

    it("verifies possible patterns with aliased parent nodes", () => {
      assert.doesNotThrow(() => new Query(Ruby, "(destructured_parameter (identifier))"));
      assert.throws(() => new Query(Ruby, "(destructured_parameter (string))"), "TSQueryErrorStructure");
    });
  });

  describe(".matches", () => {
    it("returns all of the matches for the given query", () => {
      const tree = parser.parse("function one() { two(); function three() {} }");
      const query = new Query(JavaScript, `
        (function_declaration name: (identifier) @fn-def)
        (call_expression function: (identifier) @fn-ref)
      `);
      const matches = query.matches(tree.rootNode);
      assert.deepEqual(formatMatches(tree, matches), [
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
        { startPosition: { row: 1, column: 1 }, endPosition: { row: 3, column: 1 } },
      );
      assert.deepEqual(formatMatches(tree, matches), [
        { pattern: 0, captures: [{ name: "element", text: "d" }] },
        { pattern: 0, captures: [{ name: "element", text: "e" }] },
        { pattern: 0, captures: [{ name: "element", text: "f" }] },
        { pattern: 0, captures: [{ name: "element", text: "g" }] },
      ]);
    });

    it("finds optional nodes even when using #eq? predicate", () => {
      const tree = parser.parse(`
        { one: true };
        { one: true, two: true };
      `);
      const query = new Query(JavaScript, `
        (
          (object (pair key: (property_identifier) @a) (pair key: (property_identifier) @b)?)
          (#eq? @a one)
          (#eq? @b two)
        )
      `);
      const matches = query.matches(tree.rootNode);
      assert.deepEqual(formatMatches(tree, matches), [
        { pattern: 0, captures: [{ name: 'a', text: 'one' }] },
        { pattern: 0, captures: [{ name: 'a', text: 'one' }, { name: 'b', text: 'two' }] },
      ]);
    });
  });

  describe(".captures", () => {
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
          (function_expression
            name: (identifier) @method.alias))
        (variable_declarator
          name: _ @function.def
          value: (function_expression
            name: (identifier) @function.alias))
        ":" @delimiter
        "=" @operator
      `);

      const captures = query.captures(tree.rootNode);
      assert.deepEqual(formatCaptures(tree, captures), [
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
      assert.deepEqual(formatCaptures(tree, captures), [
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
      assert.deepEqual(formatCaptures(tree, captures), [
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
      assert.deepEqual(formatCaptures(tree, captures), [
        { name: "func", text: "a", setProperties: { foo: null, bar: "baz" } },
        {
          name: "prop",
          text: "c",
          assertedProperties: { foo: null },
          refutedProperties: { bar: "baz" },
        },
      ]);
    });

    it("handles quantified captures properly", () => {
      let captures;

      const tree = parser.parse(`
        /// foo
        /// bar
        /// baz
      `);

      let query = new Query(JavaScript, `
        (
          (comment)+ @foo
          (#any-eq? @foo "/// foo")
        )
      `);

      let expectCount = (tree, queryText, expectedCount) => {
        query = new Query(JavaScript, queryText);
        captures = query.captures(tree.rootNode);
        assert.equal(captures.length, expectedCount);
      };

      expectCount(
        tree,
        ` ( (comment)+ @foo (#any-eq? @foo "/// foo") ) `,
        3
      );

      expectCount(
        tree,
        ` ( (comment)+ @foo (#eq? @foo "/// foo") ) `,
        0
      );

      expectCount(
        tree,
        ` ( (comment)+ @foo (#any-not-eq? @foo "/// foo") ) `,
        3
      );

      expectCount(
        tree,
        ` ( (comment)+ @foo (#not-eq? @foo "/// foo") ) `,
        0
      );

      expectCount(
        tree,
        ` ( (comment)+ @foo (#match? @foo "^/// foo") ) `,
        0
      );

      expectCount(
        tree,
        ` ( (comment)+ @foo (#any-match? @foo "^/// foo") ) `,
        3
      );

      expectCount(
        tree,
        ` ( (comment)+ @foo (#not-match? @foo "^/// foo") ) `,
        0
      );

      expectCount(
        tree,
        ` ( (comment)+ @foo (#not-match? @foo "fsdfsdafdfs") ) `,
        3
      );

      expectCount(
        tree,
        ` ( (comment)+ @foo (#any-not-match? @foo "^///") ) `,
        0
      );

      expectCount(
        tree,
        ` ( (comment)+ @foo (#any-not-match? @foo "^/// foo") ) `,
        3
      );
    })
  });

  describe("match limit", () => {
    it("has too many permutations to track", () => {
      const query = new Query(JavaScript, `
        (array (identifier) @pre (identifier) @post)
      `);

      const source = "[" + "hello, ".repeat(50) + "];";

      const tree = parser.parse(source);
      const matches = query.matches(tree.rootNode, { matchLimit: 32 });

      // For this pathological query, some match permutations will be dropped.
      // Just check that a subset of the results are returned, and crash or
      // leak occurs.
      assert.deepEqual(
        formatMatches(tree, matches)[0],
        { pattern: 0, captures: [{ name: "pre", text: "hello" }, { name: "post", text: "hello" }] },
      )
      assert.equal(query.matchLimit, 32);
      assert.equal(query.didExceedMatchLimit(), true);
    });

    it("has alternatives", () => {
      const query = new Query(JavaScript, `
        (
          (comment) @doc
          ; not immediate
          (class_declaration) @class
        )
        (call_expression
          function: [
              (identifier) @function
              (member_expression property: (property_identifier) @method)
          ]
        )
      `);

      const source = '/* hi */ a.b(); '.repeat(50);

      const tree = parser.parse(source);
      const matches = query.matches(tree.rootNode, { matchLimit: 32 });

      assert.deepEqual(
        formatMatches(tree, matches),
        Array(50).fill({ pattern: 1, captures: [{ name: "method", text: "b" }] }),
      );
      assert.equal(query.matchLimit, 32);
      assert.equal(query.didExceedMatchLimit(), true);
    });
  });

  describe(".disableCapture", () => {
    it("disables a capture", () => {
      const query = new Query(JavaScript, `
        (function_declaration
          (identifier) @name1 @name2 @name3
          (statement_block) @body1 @body2)
      `);

      const source = "function foo() { return 1; }";
      const tree = parser.parse(source);

      let matches = query.matches(tree.rootNode);
      assert.deepEqual(formatMatches(tree, matches), [
        {
          pattern: 0,
          captures: [
            { name: "name1", text: "foo" },
            { name: "name2", text: "foo" },
            { name: "name3", text: "foo" },
            { name: "body1", text: "{ return 1; }" },
            { name: "body2", text: "{ return 1; }" },
          ],
        },
      ]);

      // disabling captures still works when there are multiple captures on a
      // single node.
      query.disableCapture("name2");
      matches = query.matches(tree.rootNode);
      assert.deepEqual(formatMatches(tree, matches), [
        {
          pattern: 0,
          captures: [
            { name: "name1", text: "foo" },
            { name: "name3", text: "foo" },
            { name: "body1", text: "{ return 1; }" },
            { name: "body2", text: "{ return 1; }" },
          ],
        },
      ]);
    });
  });

  describe(".disablePattern", () => {
    it("disables a pattern", () => {
      const query = new Query(JavaScript, `
        (function_declaration
          name: (identifier) @name)
        (function_declaration
          body: (statement_block) @body)
        (class_declaration
          name: (identifier) @name)
        (class_declaration
          body: (class_body) @body)
      `);

      // disable the patterns that match names
      query.disablePattern(0);
      query.disablePattern(2);

      const source = "class A { constructor() {} } function b() { return 1; }";
      const tree = parser.parse(source);

      const matches = query.matches(tree.rootNode);

      assert.deepEqual(formatMatches(tree, matches), [
        {
          pattern: 3,
          captures: [
            { name: "body", text: "{ constructor() {} }" },
          ],
        },
        {
          pattern: 1,
          captures: [
            { name: "body", text: "{ return 1; }" },
          ],
        },
      ]);
    });
  });

  describe(".startIndexForPattern", () => {
    it("returns the index where the given pattern starts in the query's source", () => {
      const patterns1 = `
        "+" @operator
        "-" @operator
        "*" @operator
        "=" @operator
        "=>" @operator
      `.trimStart();

      const patterns2 = `
        (identifier) @a
        (string) @b
      `.trimStart();

      const patterns3 = `
        ((identifier) @b (#match? @b i))
        (function_declaration name: (identifier) @c)
        (method_definition name: (property_identifier) @d)
      `.trimStart();

      const source = patterns1 + patterns2 + patterns3;

      const query = new Query(JavaScript, source);

      assert.equal(query.startIndexForPattern(0), 0);
      assert.equal(query.startIndexForPattern(5), patterns1.length);
      assert.equal(query.startIndexForPattern(7), patterns1.length + patterns2.length);
    });
  });

  describe(".isPatternGuaranteedAtStep", () => {
    it("returns true if the given pattern is guaranteed to match at the given step", () => {
      const rows = [
        {
          description: "no guaranteed steps",
          language: Python,
          pattern: `(expression_statement (string))`,
          resultsBySubstring: [["expression_statement", false], ["string", false]],
        },
        {
          description: "all guaranteed steps",
          language: JavaScript,
          pattern: `(object "{" "}")`,
          resultsBySubstring: [["object", false], ["{", true], ["}", true]],
        },
        {
          description: "a fallible step that is optional",
          language: JavaScript,
          pattern: `(object "{" (identifier)? @foo "}")`,
          resultsBySubstring: [
            ["object", false],
            ["{", true],
            ["(identifier)?", false],
            ["}", true],
          ],
        },
        {
          description: "multiple fallible steps that are optional",
          language: JavaScript,
          pattern: `(object "{" (identifier)? @id1 ("," (identifier) @id2)? "}")`,
          resultsBySubstring: [
            ["object", false],
            ["{", true],
            ["(identifier)? @id1", false],
            ["\",\"", false],
            ["}", true],
          ],
        },
        {
          description: "guaranteed step after fallibe step",
          language: JavaScript,
          pattern: `(pair (property_identifier) ":")`,
          resultsBySubstring: [["pair", false], ["property_identifier", false], [":", true]],
        },
        {
          description: "fallible step in between two guaranteed steps",
          language: JavaScript,
          pattern: `(ternary_expression
                condition: (_)
                "?"
                consequence: (call_expression)
                ":"
                alternative: (_))`,
          resultsBySubstring: [
            ["condition:", false],
            ["\"?\"", false],
            ["consequence:", false],
            ["\":\"", true],
            ["alternative:", true],
          ],
        },
        {
          description: "one guaranteed step after a repetition",
          language: JavaScript,
          pattern: `(object "{" (_) "}")`,
          resultsBySubstring: [["object", false], ["{", false], ["(_)", false], ["}", true]],
        },
        {
          description: "guaranteed steps after multiple repetitions",
          language: JSON,
          pattern: `(object "{" (pair) "," (pair) "," (_) "}")`,
          resultsBySubstring: [
            ["object", false],
            ["{", false],
            ["(pair) \",\" (pair)", false],
            ["(pair) \",\" (_)", false],
            ["\",\" (_)", false],
            ["(_)", true],
            ["}", true],
          ],
        },
        {
          description: "a guaranteed step with a field",
          language: JavaScript,
          pattern: `(binary_expression left: (expression) right: (_))`,
          resultsBySubstring: [
            ["binary_expression", false],
            ["(expression)", false],
            ["(_)", true],
          ],
        },
        {
          description: "multiple guaranteed steps with fields",
          language: JavaScript,
          pattern: `(function_declaration name: (identifier) body: (statement_block))`,
          resultsBySubstring: [
            ["function_declaration", false],
            ["identifier", true],
            ["statement_block", true],
          ],
        },
        {
          description: "nesting, one guaranteed step",
          language: JavaScript,
          pattern: `
                (function_declaration
                    name: (identifier)
                    body: (statement_block "{" (expression_statement) "}"))`,
          resultsBySubstring: [
            ["function_declaration", false],
            ["identifier", false],
            ["statement_block", false],
            ["{", false],
            ["expression_statement", false],
            ["}", true],
          ],
        },
        {
          description: "a guaranteed step after some deeply nested hidden nodes",
          language: Ruby,
          pattern: `
            (singleton_class
                value: (constant)
                "end")
            `,
          resultsBySubstring: [
            ["singleton_class", false],
            ["constant", false],
            ["end", true],
          ],
        },
        {
          description: "nesting, no guaranteed steps",
          language: JavaScript,
          pattern: `
            (call_expression
                function: (member_expression
                  property: (property_identifier) @template-tag)
                arguments: (template_string)) @template-call
            `,
          resultsBySubstring: [["property_identifier", false], ["template_string", false]],
        },
        {
          description: "a guaranteed step after a nested node",
          language: JavaScript,
          pattern: `
            (subscript_expression
                object: (member_expression
                    object: (identifier) @obj
                    property: (property_identifier) @prop)
                "[")
            `,
          resultsBySubstring: [
            ["identifier", false],
            ["property_identifier", false],
            ["[", true],
          ],
        },
        {
          description: "a step that is fallible due to a predicate",
          language: JavaScript,
          pattern: `
            (subscript_expression
                object: (member_expression
                    object: (identifier) @obj
                    property: (property_identifier) @prop)
                "["
                (#match? @prop "foo"))
            `,
          resultsBySubstring: [
            ["identifier", false],
            ["property_identifier", false],
            ["[", true],
          ],
        },
        {
          description: "alternation where one branch has guaranteed steps",
          language: JavaScript,
          pattern: `
            [
                (unary_expression (identifier))
                (call_expression
                  function: (_)
                  arguments: (_))
                (binary_expression right:(call_expression))
            ]
           `,
          resultsBySubstring: [
            ["identifier", false],
            ["right:", false],
            ["function:", true],
            ["arguments:", true],
          ],
        },
        {
          description: "guaranteed step at the end of an aliased parent node",
          language: Ruby,
          pattern: `
            (method_parameters "(" (identifier) @id")")
            `,
          resultsBySubstring: [["\"(\"", false], ["(identifier)", false], ["\")\"", true]],
        },
        {
          description: "long, but not too long to analyze",
          language: JavaScript,
          pattern: `
            (object "{" (pair) (pair) (pair) (pair) "}")
            `,
          resultsBySubstring: [
            ["\"{\"", false],
            ["(pair)", false],
            ["(pair) \"}\"", false],
            ["\"}\"", true],
          ],
        },
        {
          description: "too long to analyze",
          language: JavaScript,
          pattern: `
            (object "{" (pair) (pair) (pair) (pair) (pair) (pair) (pair) (pair) (pair) (pair) (pair) (pair) "}")
            `,
          resultsBySubstring: [
            ["\"{\"", false],
            ["(pair)", false],
            ["(pair) \"}\"", false],
            ["\"}\"", false],
          ],
        },
        {
          description: "hidden nodes that have several fields",
          language: Java,
          pattern: `
            (method_declaration name: (identifier))
            `,
          resultsBySubstring: [["name:", true]],
        },
        {
          description: "top-level non-terminal extra nodes",
          language: Ruby,
          pattern: `
            (heredoc_body
                (interpolation)
                (heredoc_end) @end)
            `,
          resultsBySubstring: [
            ["(heredoc_body", false],
            ["(interpolation)", false],
            ["(heredoc_end)", true],
          ],
        },
        // TODO: figure out why line comments, an extra, are no longer allowed *anywhere*
        // likely culprits are the fact that it's no longer a token itself or that it uses an
        // external token
        // {
        //     description: "multiple extra nodes",
        //     language: Rust,
        //     pattern: `
        //     (call_expression
        //         (line_comment) @a
        //         (line_comment) @b
        //         (arguments))
        //     `,
        //     resultsBySubstring: [
        //         ["(line_comment) @a", false],
        //         ["(line_comment) @b", false],
        //         ["(arguments)", true],
        //     ],
        // },
      ];

      for (const row of rows) {
        const query = new Query(row.language, row.pattern);
        for (const [substring, isDefinite] of row.resultsBySubstring) {
          const offset = row.pattern.indexOf(substring);
          assert.equal(
            query.isPatternGuaranteedAtStep(offset),
            isDefinite,
            `Description: ${row.description}, Pattern: ${row.pattern}, Substring: ${substring}, expected isDefinite to be: ${isDefinite}`,
          );
        }
      }
    });
  });

  describe(".isPatternRooted", () => {
    it("returns true if the given pattern has a single root node", () => {
      const rows = [
        {
          description: "simple token",
          pattern: "(identifier)",
          isRooted: true,
        },
        {
          description: "simple non-terminal",
          pattern: "(function_definition name: (identifier))",
          isRooted: true,
        },
        {
          description: "alternative of many tokens",
          pattern: '["if" "def" (identifier) (comment)]',
          isRooted: true,
        },
        {
          description: "alternative of many non-terminals",
          pattern: `
            [
              (function_definition name: (identifier))
              (class_definition name: (identifier))
              (block)
            ]
          `,
          isRooted: true,
        },
        {
          description: "two siblings",
          pattern: '("{" "}")',
          isRooted: false,
        },
        {
          description: "top-level repetition",
          pattern: "(comment)*",
          isRooted: false,
        },
        {
          description: "alternative where one option has two siblings",
          pattern: `
            [
              (block)
              (class_definition)
              ("(" ")")
              (function_definition)
            ]
          `,
          isRooted: false,
        },
        {
          description: "alternative where one option has a top-level repetition",
          pattern: `
            [
              (block)
              (class_definition)
              (comment)*
              (function_definition)
            ]
          `,
          isRooted: false,
        },
      ];

      parser.setLanguage(Python);
      for (const row of rows) {
        const query = new Query(Python, row.pattern);
        assert.equal(
          query.isPatternRooted(0),
          row.isRooted,
          `Description: ${row.description}, Pattern: ${row.pattern}`,
        );
      }
    });
  });

  describe(".isPatternNonLocal", () => {
    it("returns true if the given pattern has a single root node", () => {
      const rows = [
        {
          description: "simple token",
          pattern: "(identifier)",
          language: Python,
          isNonLocal: false,
        },
        {
          description: "siblings that can occur in an argument list",
          pattern: "((identifier) (identifier))",
          language: Python,
          isNonLocal: true,
        },
        {
          description: "siblings that can occur in a statement block",
          pattern: "((return_statement) (return_statement))",
          language: Python,
          isNonLocal: true,
        },
        {
          description: "siblings that can occur in a source file",
          pattern: "((function_definition) (class_definition))",
          language: Python,
          isNonLocal: true,
        },
        {
          description: "siblings that can't occur in any repetition",
          pattern: `("{" "}")`,
          language: Python,
          isNonLocal: false,
        },
        {
          description: "siblings that can't occur in any repetition, wildcard root",
          pattern: `(_ "{" "}") @foo`,
          language: JavaScript,
          isNonLocal: false,
        },
        {
          description: "siblings that can occur in a class body, wildcard root",
          pattern: "(_ (method_definition) (method_definition)) @foo",
          language: JavaScript,
          isNonLocal: true,
        },
        {
          description: "top-level repetitions that can occur in a class body",
          pattern: "(method_definition)+ @foo",
          language: JavaScript,
          isNonLocal: true,
        },
        {
          description: "top-level repetitions that can occur in a statement block",
          pattern: "(return_statement)+ @foo",
          language: JavaScript,
          isNonLocal: true,
        },
        {
          description: "rooted pattern that can occur in a statement block",
          pattern: "(return_statement) @foo",
          language: JavaScript,
          isNonLocal: false,
        },
      ];

      for (const row of rows) {
        const query = new Query(row.language, row.pattern);
        assert.equal(
          query.isPatternNonLocal(0),
          row.isNonLocal,
          `Description: ${row.description}, Pattern: ${row.pattern}`,
        );
      }
    });
  });

  describe("max start depth", () => {
    it("will not explore child nodes beyond the given depth", () => {
      const source = `
if (a1 && a2) {
    if (b1 && b2) { }
    if (c) { }
}
if (d) {
    if (e1 && e2) { }
    if (f) { }
}
`;

      const rows = [
        {
          description: "depth 0: match translation unit",
          depth: 0,
          pattern: `
              (translation_unit) @capture
          `,
          matches: [
            [0, [["capture", "if (a1 && a2) {\n    if (b1 && b2) { }\n    if (c) { }\n}\nif (d) {\n    if (e1 && e2) { }\n    if (f) { }\n}\n"]]],
          ]
        },
        {
          description: "depth 0: match none",
          depth: 0,
          pattern: `
              (if_statement) @capture
          `,
          matches: []
        },
        {
          description: "depth 1: match 2 if statements at the top level",
          depth: 1,
          pattern: `
              (if_statement) @capture
          `,
          matches: [
            [0, [["capture", "if (a1 && a2) {\n    if (b1 && b2) { }\n    if (c) { }\n}"]]],
            [0, [["capture", "if (d) {\n    if (e1 && e2) { }\n    if (f) { }\n}"]]],
          ]
        },
        {
          description: "depth 1 with deep pattern: match the only the first if statement",
          depth: 1,
          pattern: `
              (if_statement
                  condition: (parenthesized_expression
                      (binary_expression)
                  )
              ) @capture
          `,
          matches: [
            [0, [["capture", "if (a1 && a2) {\n    if (b1 && b2) { }\n    if (c) { }\n}"]]],
          ]
        },
        {
          description: "depth 3 with deep pattern: match all if statements with a binexpr condition",
          depth: 3,
          pattern: `
              (if_statement
                  condition: (parenthesized_expression
                      (binary_expression)
                  )
              ) @capture
          `,
          matches: [
            [0, [["capture", "if (a1 && a2) {\n    if (b1 && b2) { }\n    if (c) { }\n}"]]],
            [0, [["capture", "if (b1 && b2) { }"]]],
            [0, [["capture", "if (e1 && e2) { }"]]],
          ]
        },
      ];

      parser.setLanguage(C);
      const tree = parser.parse(source);

      for (const row of rows) {
        const query = new Query(C, row.pattern);
        const matches = query.matches(tree.rootNode, { maxStartDepth: row.depth });
        const expected = row.matches.map(([pattern, captures]) => ({
          pattern,
          captures: captures.map(([name, text]) => ({ name, text })),
        }));
        assert.deepEqual(formatMatches(tree, matches), expected, row.description);
      }
    });

    it("tests more", () => {
      const source = `
{
    { }
    {
        { }
    }
}
`
      const rows = [
        {
          depth: 0,
          matches: [
            [0, [["capture", "{\n    { }\n    {\n        { }\n    }\n}"]]],
          ]
        },
        {
          depth: 1,
          matches: [
            [0, [["capture", "{\n    { }\n    {\n        { }\n    }\n}"]]],
            [0, [["capture", "{ }"]]],
            [0, [["capture", "{\n        { }\n    }"]]],
          ]
        },
        {
          depth: 2,
          matches: [
            [0, [["capture", "{\n    { }\n    {\n        { }\n    }\n}"]]],
            [0, [["capture", "{ }"]]],
            [0, [["capture", "{\n        { }\n    }"]]],
            [0, [["capture", "{ }"]]],
          ]
        },
      ];

      parser.setLanguage(C);
      const tree = parser.parse(source);
      const query = new Query(C, "(compound_statement) @capture");
      const matches = query.matches(tree.rootNode);
      const node = matches[0].captures[0].node;
      assert.equal(node.type, "compound_statement");

      for (const row of rows) {
        const matches = query.matches(node, { maxStartDepth: row.depth });
        const expected = row.matches.map(([pattern, captures]) => ({
          pattern,
          captures: captures.map(([name, text]) => ({ name, text })),
        }));
        assert.deepEqual(formatMatches(tree, matches), expected);
      }
    });
  });
});

function formatMatches(tree, matches) {
  return matches.map(({ pattern, captures }) => ({
    pattern,
    captures: formatCaptures(tree, captures),
  }));
}

function formatCaptures(tree, captures) {
  return captures.map((c) => {
    const node = c.node;
    delete c.node;
    c.text = tree.getText(node);
    return c;
  });
}
