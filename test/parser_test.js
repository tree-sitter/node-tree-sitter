const Parser = require("..");
const JavaScript = require('tree-sitter-javascript');
const { assert } = require("chai");

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
});
