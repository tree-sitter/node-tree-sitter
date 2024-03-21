const Parser = require("..");
const HTML = require('tree-sitter-html');
const JavaScript = require('tree-sitter-javascript');
const JSON = require('tree-sitter-json');
const Rust = require('tree-sitter-rust');
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

        parser.setLogger((_msg, _params) => {
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

  describe(".printDotGraphs", () => {
    beforeEach(() => {
      parser.setLanguage(JavaScript);
    });

    it("prints a dot graph to the output file", () => {
      if (process.platform === "win32") {
        return;
      }

      const hasZeroIndexedRow = s => s.indexOf("position: 0,") !== -1;

      const tmp = require("tmp");
      const debugGraphFile = tmp.fileSync({ postfix: ".dot" });
      parser.printDotGraphs(true, debugGraphFile.fd);
      parser.parse("const zero = 0");

      const fs = require('fs');
      const logReader = fs.readFileSync(debugGraphFile.name, 'utf8').split('\n');
      for (let line of logReader) {
        assert.strictEqual(hasZeroIndexedRow(line), false, `Graph log output includes zero-indexed row: ${line}`);
      }

      debugGraphFile.removeCallback();
    });
  });

  describe(".parse", () => {
    beforeEach(() => {
      parser.setLanguage(JavaScript);
    });

    it("parses a simple string", () => {
      parser.setLanguage(Rust);
      const tree = parser.parse(`
        struct Stuff {}
        fn main() {}
      `);
      const rootNode = tree.rootNode;
      assert.equal(rootNode.type, "source_file");

      assert.equal(
        rootNode.toString(),
        "(source_file (struct_item name: (type_identifier) body: (field_declaration_list)) " +
        "(function_item name: (identifier) parameters: (parameters) body: (block)))"
      );

      const structNode = rootNode.firstChild;
      assert.equal(structNode.type, "struct_item");
    });

    it("reads from the given input", () => {
      const parts = ["first", "_", "second", "_", "third"];
      const tree = parser.parse((_index) => parts.shift());
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
              startPosition: { row: 0, column: start1 },
              endPosition: { row: 0, column: end1 }
            },
            {
              startIndex: start2,
              endIndex: end2,
              startPosition: { row: 0, column: start2 },
              endPosition: { row: 0, column: end2 }
            },
          ]
        });

        assert.equal(
          tree.rootNode.toString(),
          '(program (expression_statement (call_expression function: (identifier) arguments: (arguments))) (expression_statement (identifier)))'
        );
      })
    })

    describe('invalid chars at eof', () => {
      it('shows in the parse tree', () => {
        parser.setLanguage(JSON);
        const tree = parser.parse('\xdf');
        assert.equal(tree.rootNode.toString(), "(document (ERROR (UNEXPECTED 223)))");
      });
    });

    describe('unexpected null characters', () => {
      it('has a null character in the source', () => {
        parser.setLanguage(JavaScript);
        const tree = parser.parse('var \0 something;')
        assert.equal(
          tree.rootNode.toString(),
          "(program (variable_declaration (ERROR (UNEXPECTED '\\0')) (variable_declarator name: (identifier))))"
        );
      });
    });

    it("parses an empty file with a reused tree", () => {
      parser.setLanguage(Rust);

      let tree = parser.parse("");
      parser.parse("", tree);

      tree = parser.parse("\n ");
      parser.parse("\n ", tree);
    });

    describe('parsing with a timeout', () => {
      it('stops after a certain number of microseconds', () => {
        parser.setLanguage(JSON);

        let startTime = performance.now() * 1000;
        parser.setTimeoutMicros(1000);
        let tree = parser.parse((offset, _) => offset === 0 ? " [" : ",0");
        assert.equal(tree, null);
        assert.isBelow(performance.now() * 1000 - startTime, 2000);

        startTime = performance.now() * 1000;
        parser.setTimeoutMicros(5000);
        tree = parser.parse((offset, _) => offset === 0 ? " [" : ",0");
        assert.equal(tree, null);
        assert.isAbove(performance.now() * 1000 - startTime, 100);
        assert.isBelow(performance.now() * 1000 - startTime, 10000);

        parser.setTimeoutMicros(0);
        tree = parser.parse((offset, _) => offset > 5000 ? "" : offset == 5000 ? "]" : ",0");
        assert.equal(tree.rootNode.firstChild.type, "array");
      });
    });

    describe('parsing with a timeout and reset', () => {
      it('stops after a certain number of microseconds and resets the parser', () => {
        parser.setLanguage(JSON);

        parser.setTimeoutMicros(5);
        let tree = parser.parse(
          '["ok", 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32]'
        );
        assert.equal(tree, null);

        // Without calling reset, the parser continues from where it left off, so
        // it does not see the changes to the beginning of the source code.
        parser.setTimeoutMicros(0);
        tree = parser.parse(
          '[null, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32]'
        );
        assert.equal(tree.rootNode.firstNamedChild.firstNamedChild.type, "string");

        parser.setTimeoutMicros(5);
        tree = parser.parse(
          '[\"ok\", 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32]'
        );
        assert.equal(tree, null);

        // By calling reset, we force the parser to start over from scratch so
        // that it sees the changes to the beginning of the source code.
        parser.setTimeoutMicros(0);
        parser.reset();
        tree = parser.parse(
          '[null, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32]'
        );
        assert.equal(tree.rootNode.firstNamedChild.firstNamedChild.type, "null");
      });
    });

    describe('parsing with a timeout and implicit reset', () => {
      it('stops after a certain number of microseconds and resets the parser', () => {
        parser.setLanguage(JavaScript);

        parser.setTimeoutMicros(5);
        let tree = parser.parse(
          '[\"ok\", 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32]'
        );
        assert.equal(tree, null);

        // Changing the parser's language implicitly resets, discarding
        // the previous partial parse.
        parser.setLanguage(JSON);
        parser.setTimeoutMicros(0);
        tree = parser.parse(
          '[null, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32]'
        );
        assert.equal(tree.rootNode.firstNamedChild.firstNamedChild.type, "null");
      });
    });

    describe('parsing with a timeout and no completion', () => {
      it('stops after a certain number of microseconds and returns before completion', () => {
        parser.setLanguage(JavaScript);

        parser.setTimeoutMicros(5);
        let tree = parser.parse(
          '[\"ok\", 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32]'
        );
        assert.equal(tree, null);
      });
    });

    describe('one included range', () => {
      it('parses the text within a range', () => {
        parser.setLanguage(HTML);
        const sourceCode = "<span>hi</span><script>console.log('sup');</script>";
        const htmlTree = parser.parse(sourceCode);
        const scriptContentNode = htmlTree.rootNode.child(1).child(1);
        assert.equal(scriptContentNode.type, "raw_text");

        parser.setLanguage(JavaScript);
        assert.deepEqual(parser.getIncludedRanges(), [{
          startIndex: 0,
          endIndex: 2147483647,
          startPosition: { row: 0, column: 0 },
          endPosition: { row: 4294967295, column: 2147483647 },
        }]);
        const ranges = [{
          startIndex: scriptContentNode.startIndex,
          endIndex: scriptContentNode.endIndex,
          startPosition: scriptContentNode.startPosition,
          endPosition: scriptContentNode.endPosition
        }];
        const jsTree = parser.parse(
          sourceCode,
          null,
          { includedRanges: ranges }
        );
        assert.deepEqual(parser.getIncludedRanges(), ranges);

        assert.equal(
          jsTree.rootNode.toString(),
          "(program (expression_statement (call_expression " +
          "function: (member_expression object: (identifier) property: (property_identifier)) " +
          "arguments: (arguments (string (string_fragment))))))"
        );
        assert.deepEqual(jsTree.rootNode.startPosition, { row: 0, column: sourceCode.indexOf('console') });
      });
    });

    describe('multiple included ranges', () => {
      it('parses the text within multiple ranges', () => {
        parser.setLanguage(JavaScript);
        const sourceCode = "html `<div>Hello, ${name.toUpperCase()}, it's <b>${now()}</b>.</div>`";
        const jsTree = parser.parse(sourceCode);
        const templateStringNode = jsTree.rootNode.descendantForIndex(sourceCode.indexOf('`<'), sourceCode.indexOf('>`'));
        assert.equal(templateStringNode.type, "template_string");

        const openQuoteNode = templateStringNode.child(0);
        const interpolationNode1 = templateStringNode.child(2);
        const interpolationNode2 = templateStringNode.child(4);
        const closeQuoteNode = templateStringNode.child(6);

        parser.setLanguage(HTML);
        const htmlRanges = [
          {
            startIndex: openQuoteNode.endIndex,
            startPosition: openQuoteNode.endPosition,
            endIndex: interpolationNode1.startIndex,
            endPosition: interpolationNode1.startPosition
          },
          {
            startIndex: interpolationNode1.endIndex,
            startPosition: interpolationNode1.endPosition,
            endIndex: interpolationNode2.startIndex,
            endPosition: interpolationNode2.startPosition
          },
          {
            startIndex: interpolationNode2.endIndex,
            startPosition: interpolationNode2.endPosition,
            endIndex: closeQuoteNode.startIndex,
            endPosition: closeQuoteNode.startPosition
          },
        ];
        const htmlTree = parser.parse(sourceCode, null, { includedRanges: htmlRanges });

        assert.equal(
          htmlTree.rootNode.toString(),
          "(document (element" +
          " (start_tag (tag_name))" +
          " (text)" +
          " (element (start_tag (tag_name)) (end_tag (tag_name)))" +
          " (text)" +
          " (end_tag (tag_name))))"
        );
        assert.deepEqual(htmlTree.getIncludedRanges(), htmlRanges);

        const divElementNode = htmlTree.rootNode.child(0);
        const helloTextNode = divElementNode.child(1);
        const bElementNode = divElementNode.child(2);
        const bStartTagNode = bElementNode.child(0);
        const bEndTagNode = bElementNode.child(1);

        assert.equal(helloTextNode.type, "text");
        assert.equal(helloTextNode.startIndex, sourceCode.indexOf('Hello'));
        assert.equal(helloTextNode.endIndex, sourceCode.indexOf(' <b>'));

        assert.equal(bStartTagNode.type, "start_tag");
        assert.equal(bStartTagNode.startIndex, sourceCode.indexOf('<b>'));
        assert.equal(bStartTagNode.endIndex, sourceCode.indexOf('${now()}'));

        assert.equal(bEndTagNode.type, "end_tag");
        assert.equal(bEndTagNode.startIndex, sourceCode.indexOf('</b>'));
        assert.equal(bEndTagNode.endIndex, sourceCode.indexOf('.</div>'));
      });
    });

    describe('an included range containing mismatched positions', () => {
      it('parses the text within the range', () => {
        const sourceCode = "<div>test</div>{_ignore_this_part_}";

        parser.setLanguage(HTML);

        const endIndex = sourceCode.indexOf('{_ignore_this_part_');

        const rangeToParse = {
          startIndex: 0,
          startPosition: { row: 10, column: 12 },
          endIndex,
          endPosition: { row: 10, column: 12 + endIndex },
        };

        const htmlTree = parser.parse(sourceCode, null, { includedRanges: [rangeToParse] });

        assert.deepEqual(htmlTree.getIncludedRanges()[0], rangeToParse);

        assert.equal(
          htmlTree.rootNode.toString(),
          "(document (element (start_tag (tag_name)) (text) (end_tag (tag_name))))"
        );
      });
    });

    describe('parsing error in invalid included ranges', () => {
      it('throws an exception', () => {
        const ranges = [
          { startIndex: 23, endIndex: 29, startPosition: { row: 0, column: 23 }, endPosition: { row: 0, column: 29 } },
          { startIndex: 0, endIndex: 5, startPosition: { row: 0, column: 0 }, endPosition: { row: 0, column: 5 } },
          { startIndex: 50, endIndex: 60, startPosition: { row: 0, column: 50 }, endPosition: { row: 0, column: 60 } },
        ];
        parser.setLanguage(JavaScript);
        assert.throws(() => parser.parse('var a = 1;', null, { includedRanges: ranges }), /Overlapping ranges/);
      });
    });

    describe('test parsing with external scanner that uses included range boundaries', () => {
      it('parses the text within the range', () => {
        const sourceCode = 'a <%= b() %> c <% d() %>';
        const range1StartByte = sourceCode.indexOf(' b() ');
        const range1EndByte = range1StartByte + ' b() '.length;
        const range2StartByte = sourceCode.indexOf(' d() ');
        const range2EndByte = range2StartByte + ' d() '.length;

        parser.setLanguage(JavaScript);
        const tree = parser.parse(sourceCode, null, {
          includedRanges: [
            {
              startIndex: range1StartByte,
              endIndex: range1EndByte,
              startPosition: { row: 0, column: range1StartByte },
              endPosition: { row: 0, column: range1EndByte }
            },
            {
              startIndex: range2StartByte,
              endIndex: range2EndByte,
              startPosition: { row: 0, column: range2StartByte },
              endPosition: { row: 0, column: range2EndByte }
            },
          ]
        });

        const root = tree.rootNode;
        const statement1 = root.firstChild;
        const statement2 = root.child(1);

        assert.equal(
          root.toString(),
          "(program" +
          " (expression_statement (call_expression function: (identifier) arguments: (arguments)))" +
          " (expression_statement (call_expression function: (identifier) arguments: (arguments))))"
        );

        assert.equal(statement1.startIndex, sourceCode.indexOf("b()"));
        assert.equal(statement1.endIndex, sourceCode.indexOf(" %> c"));
        assert.equal(statement2.startIndex, sourceCode.indexOf("d()"));
        assert.equal(statement2.endIndex, sourceCode.length - " %>".length);
      });
    });

    describe('parsing with a newly excluded range', () => {
      it('parses code after an edit', () => {
        let sourceCode = '<div><span><%= something %></span></div>';

        parser.setLanguage(HTML);
        let firstTree = parser.parse(sourceCode);

        // Insert code at the beginning of the document.
        const prefix = 'a very very long line of plain text. ';
        firstTree.edit({
          startIndex: 0,
          oldEndIndex: 0,
          newEndIndex: prefix.length,
          startPosition: { row: 0, column: 0 },
          oldEndPosition: { row: 0, column: 0 },
          newEndPosition: { row: 0, column: prefix.length },
        });
        sourceCode = prefix + sourceCode;

        // Parse the HTML again, this time *excluding* the template directive
        // (which has moved since the previous parse).
        const directiveStart = sourceCode.indexOf('<%=');
        const directiveEnd = sourceCode.indexOf('</span>');
        const sourceCodeEnd = sourceCode.length;
        const tree = parser.parse(sourceCode, firstTree, {
          includedRanges: [
            {
              startIndex: 0,
              endIndex: directiveStart,
              startPosition: { row: 0, column: 0 },
              endPosition: { row: 0, column: directiveStart },
            },
            {
              startIndex: directiveEnd,
              endIndex: sourceCodeEnd,
              startPosition: { row: 0, column: directiveEnd },
              endPosition: { row: 0, column: sourceCodeEnd },
            },
          ]
        });

        assert.equal(
          tree.rootNode.toString(),
          "(document (text) (element" +
          " (start_tag (tag_name))" +
          " (element (start_tag (tag_name)) (end_tag (tag_name)))" +
          " (end_tag (tag_name))))"
        );

        assert.deepEqual(
          tree.getChangedRanges(firstTree),
          [
            // The first range that has changed syntax is the range of the newly-inserted text.
            {
              startIndex: 0,
              endIndex: prefix.length,
              startPosition: { row: 0, column: 0 },
              endPosition: { row: 0, column: prefix.length },
            },
            // Even though no edits were applied to the outer `div` element,
            // its contents have changed syntax because a range of text that
            // was previously included is now excluded.
            {
              startIndex: directiveStart,
              endIndex: directiveEnd,
              startPosition: { row: 0, column: directiveStart },
              endPosition: { row: 0, column: directiveEnd },
            },
          ]
        );
      });
    });

    describe('parsing with a newly included range', () => {
      function simpleRange(startIndex, endIndex) {
        return {
          startIndex,
          endIndex,
          startPosition: { row: 0, column: startIndex },
          endPosition: { row: 0, column: endIndex },
        };
      }

      it('parses code after an edit', () => {
        const sourceCode = '<div><%= foo() %></div><span><%= bar() %></span><%= baz() %>';
        const range1Start = sourceCode.indexOf(' foo');
        const range2Start = sourceCode.indexOf(' bar');
        const range3Start = sourceCode.indexOf(' baz');
        const range1End = range1Start + 7;
        const range2End = range2Start + 7;
        const range3End = range3Start + 7;

        // Parse only the first code directive as JavaScript
        parser.setLanguage(JavaScript);
        const tree = parser.parse(sourceCode, null, {
          includedRanges: [simpleRange(range1Start, range1End)]
        });
        assert.equal(
          tree.rootNode.toString(),
          "(program" +
          " (expression_statement (call_expression function: (identifier) arguments: (arguments))))"
        );

        // Parse both the first and third code directives as JavaScript, using the old tree as a
        // reference.
        const tree2 = parser.parse(sourceCode, tree, {
          includedRanges: [
            simpleRange(range1Start, range1End),
            simpleRange(range3Start, range3End)
          ]
        });
        assert.equal(
          tree2.rootNode.toString(),
          "(program" +
          " (expression_statement (call_expression function: (identifier) arguments: (arguments)))" +
          " (expression_statement (call_expression function: (identifier) arguments: (arguments))))"
        );
        assert.deepEqual(
          tree2.getChangedRanges(tree),
          [simpleRange(range1End, range3End)]
        );

        const tree3 = parser.parse(sourceCode, tree, {
          includedRanges: [
            simpleRange(range1Start, range1End),
            simpleRange(range2Start, range2End),
            simpleRange(range3Start, range3End),
          ]
        });
        assert.equal(
          tree3.rootNode.toString(),
          "(program" +
          " (expression_statement (call_expression function: (identifier) arguments: (arguments)))" +
          " (expression_statement (call_expression function: (identifier) arguments: (arguments)))" +
          " (expression_statement (call_expression function: (identifier) arguments: (arguments))))"
        );
        assert.deepEqual(
          tree3.getChangedRanges(tree2),
          [simpleRange(range2Start + 1, range2End - 1)]
        );
      });
    });
  });
});
