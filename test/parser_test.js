const Parser = require("..");
const JavaScript = require('tree-sitter-javascript');
const { assert } = require("chai");
const {TextBuffer} = require('superstring');

describe("Parser", () => {
  let parser;

  beforeEach(() => {
    parser = new Parser();
  });

  describe(".setLanguage", () => {
    it("throws an exception when the supplied object is not a tree-sitter language", () => {
      assert.throws(() => parser.setLanguage({}), /Invalid language/);
      assert.throws(() => parser.setLanguage(undefined), /Invalid language/);
    });
  });

  describe(".setLogger", () => {
    let debugMessages;

    beforeEach(() => {
      debugMessages = [];
      parser.setLanguage(JavaScript);
      parser.setLogger((message) => debugMessages.push(message));
    });

    it("calls the given callback for each parse event", () => {
      parser.parse("a + b + c");
      assert.includeMembers(debugMessages, ["reduce", "accept", "shift"]);
    });

    it("allows the callback to be retrieved later", () => {
      let callback = () => null;

      parser.setLogger(callback);
      assert.equal(callback, parser.getLogger());

      parser.setLogger(false);
      assert.equal(null, parser.getLogger());
    });

    describe("when given a falsy value", () => {
      beforeEach(() => {
        parser.setLogger(false);
      });

      it("disables debugging", () => {
        parser.parse("a + b * c");
        assert.equal(0, debugMessages.length);
      });
    });

    describe("when given a truthy value that isn't a function", () => {
      it("raises an exception", () => {
        assert.throws(
          () => parser.setLogger("5"),
          /Logger callback must .* function .* falsy/
        );
      });
    });

    describe("when the given callback throws an exception", () => {
      let errorMessages, originalConsoleError, thrownError;

      beforeEach(() => {
        errorMessages = [];
        thrownError = new Error("dang.");

        originalConsoleError = console.error;
        console.error = (message, error) => {
          errorMessages.push([message, error]);
        };

        parser.setLogger((msg, params) => {
          throw thrownError;
        });
      });

      afterEach(() => {
        console.error = originalConsoleError;
      });

      it("logs the error to the console", () => {
        parser.parse("function() {}");

        assert.deepEqual(errorMessages[0], [
          "Error in debug callback:",
          thrownError
        ]);
      });
    });
  });

  describe(".parse", () => {
    beforeEach(() => {
      parser.setLanguage(JavaScript);
    });

    it("reads from the given input", () => {
      const parts = ["first", "_", "second", "_", "third"];
      const tree = parser.parse((index) => parts.shift());
      assert.equal(tree.rootNode.toString(), "(program (expression_statement (identifier)))");
    });

    describe("when the input callback returns something other than a string", () => {
      it("stops reading", () => {
        const parts = ["abc", "def", "ghi", {}, {}, {}, "second-word", " "];
        const tree = parser.parse(() => parts.shift());
        assert.equal(
          tree.rootNode.toString(),
          "(program (expression_statement (identifier)))"
        );
        assert.equal(tree.rootNode.endIndex, 9);
        assert.equal(parts.length, 2);
      });
    });

    describe("when the given input is not a function", () => {
      it("throws an exception", () => {
        assert.throws(() => parser.parse(null), /Input.*function/);
        assert.throws(() => parser.parse(5), /Input.*function/);
        assert.throws(() => parser.parse({}), /Input.*function/);
      });
    });

    it("handles long input strings", () => {
      const repeatCount = 10000;
      const inputString = "[" + "0,".repeat(repeatCount) + "]";

      const tree = parser.parse(inputString);
      assert.equal(tree.rootNode.type, "program");
      assert.equal(tree.rootNode.firstChild.firstChild.namedChildCount, repeatCount);
    });

    describe('when the `includedRanges` option is given', () => {
      it('parses the text within those ranges of the string', () => {
        const sourceCode = "<% foo() %> <% bar %>";

        const start1 = sourceCode.indexOf('foo');
        const end1 = start1 + 5
        const start2 = sourceCode.indexOf('bar');
        const end2 = start2 + 3

        const tree = parser.parse(sourceCode, null, {
          includedRanges: [
            {
              startIndex: start1,
              endIndex: end1,
              startPosition: {row: 0, column: start1},
              endPosition: {row: 0, column: end1}
            },
            {
              startIndex: start2,
              endIndex: end2,
              startPosition: {row: 0, column: start2},
              endPosition: {row: 0, column: end2}
            },
          ]
        });

        assert.equal(
          tree.rootNode.toString(),
          '(program (expression_statement (call_expression function: (identifier) arguments: (arguments))) (expression_statement (identifier)))'
        );
      })
    })
  });

  describe('.parseTextBuffer', () => {
    beforeEach(() => {
      parser.setLanguage(JavaScript);
    });

    it('parses the contents of the given text buffer asynchronously', async () => {
      const elementCount = 40;
      const sourceCode = "[" + "0,".repeat(elementCount) + "]";
      const buffer = new TextBuffer(sourceCode)

      const tree = await parser.parseTextBuffer(buffer);
      const arrayNode = tree.rootNode.firstChild.firstChild;
      assert.equal(arrayNode.type, "array");
      assert.equal(arrayNode.namedChildCount, elementCount);

      const editIndex = 5;
      buffer.setTextInRange(
        {start: {row: 0, column: editIndex}, end: {row: 0, column: editIndex}},
        'null,'
      );
      tree.edit({
        startIndex: editIndex,
        oldEndIndex: editIndex,
        newEndIndex: editIndex + 5,
        startPosition: {row: 0, column: editIndex},
        oldEndPosition: {row: 0, column: editIndex},
        newEndPosition: {row: 0, column: editIndex + 5}
      });

      const newTree = await parser.parseTextBuffer(buffer, tree);
      const newArrayNode = newTree.rootNode.firstChild.firstChild;
      assert.equal(newArrayNode.type, "array");
      assert.equal(newArrayNode.namedChildCount, elementCount + 1);
    });

    it('does not allow the parser to be mutated while parsing', async () => {
      const buffer = new TextBuffer('a + b + c');
      const treePromise = parser.parseTextBuffer(buffer);

      assert.throws(() => {
        parser.parse('first-word');
      }, /Parser is in use/);

      assert.throws(() => {
        parser.setLanguage(JavaScript);
      }, /Parser is in use/);

      assert.throws(() => {
        parser.printDotGraphs(true);
      }, /Parser is in use/);

      const tree = await treePromise;
      assert.equal(tree.rootNode.toString(), "(program (expression_statement (binary_expression left: (binary_expression left: (identifier) right: (identifier)) right: (identifier))))");

      parser.parse('a');
      parser.setLanguage(JavaScript);
      parser.printDotGraphs(true);
    });

    it('throws an error if the given object is not a TextBuffer', () => {
      assert.throws(() => {
        parser.parseTextBuffer({});
      });
    });

    it('does not try to call JS logger functions when parsing asynchronously', async () => {
      const messages = [];
      parser.setLogger(message => messages.push(message));

      const tree1 = parser.parse('first-word second-word');
      assert(messages.length > 0);
      messages.length = 0;

      const buffer = new TextBuffer('first-word second-word');
      const tree2 = await parser.parseTextBuffer(buffer);
      assert(messages.length === 0);

      const tree3 = parser.parseTextBufferSync(buffer);
      assert(messages.length > 0);

      assert.equal(tree2.rootNode.toString(), tree1.rootNode.toString())
      assert.equal(tree3.rootNode.toString(), tree1.rootNode.toString())
    })

    describe('when the `includedRanges` option is given', () => {
      it('parses the text within those ranges of the string', async () => {
        const sourceCode = "<% foo() %> <% bar %>";

        const start1 = sourceCode.indexOf('foo');
        const end1 = start1 + 5
        const start2 = sourceCode.indexOf('bar');
        const end2 = start2 + 3

        const buffer = new TextBuffer(sourceCode);
        const tree = await parser.parseTextBuffer(buffer, null, {
          includedRanges: [
            {
              startIndex: start1,
              endIndex: end1,
              startPosition: {row: 0, column: start1},
              endPosition: {row: 0, column: end1}
            },
            {
              startIndex: start2,
              endIndex: end2,
              startPosition: {row: 0, column: start2},
              endPosition: {row: 0, column: end2}
            },
          ]
        });

        assert.equal(
          tree.rootNode.toString(),
          '(program (expression_statement (call_expression function: (identifier) arguments: (arguments))) (expression_statement (identifier)))'
        );
      })
    })
  });

  describe('.parseTextBufferSync', () => {
    it('parses the contents of the given text buffer synchronously', () => {
      parser.setLanguage(JavaScript);
      const buffer = new TextBuffer('a + b')
      const tree = parser.parseTextBufferSync(buffer);
      assert.equal(
        tree.rootNode.toString(),
        "(program (expression_statement (binary_expression left: (identifier) right: (identifier))))"
      );
    });

    it('returns null if no language has been set', () => {
      const buffer = new TextBuffer('αβ αβδ')
      const tree = parser.parseTextBufferSync(buffer);
      assert.equal(tree, null);
    })
  });
});
