assert = require("assert")
SpyReader = require "./helpers/spy_reader"
ts = require("..")
{ repeat, seq } = ts.rules

grammar = ts.grammar
  name: 'trivial_grammar'
  rules:
    paragraph: -> repeat(@sentence)
    sentence: -> seq repeat(@word), "."
    word: -> /\w+/

cCode = ts.compile(grammar)
libPath = ts.buildParser(grammar.name, cCode)
parser = ts.loadParser(libPath, grammar.name)

describe "documents", ->
  document = null
  reader = null

  beforeEach ->
    document = new ts.Document()
    document.setParser(parser)

    reader = new SpyReader("see spot run. spot runs fast.", 3)
    document.setInput(reader)

  it "reads the entire input", ->
    assert.deepEqual(reader.chunksRead, [
      "see", " sp", "ot ", "run",
      ". s", "pot", " ru", "ns ", "fas", "t.", "", ""
    ])

  it "parses the input", ->
    assert.equal(document.toString(), "Document: (paragraph (sentence (word) (word) (word)) (sentence (word) (word) (word)))")

  describe "the parse tree", ->
    it "exposes the root node", ->
      paragraph = document.rootNode()
      assert.equal(paragraph.name(), "paragraph")

    it "exposes the root node's children", ->
      paragraph = document.rootNode()
      assert.equal(paragraph.children.length, 2)
      assert.equal(paragraph.children[0].name(), "sentence")
      assert.equal(paragraph.children[1].name(), "sentence")
