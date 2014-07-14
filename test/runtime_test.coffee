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
parser = ts.loadParserLib(libPath, grammar.name)

describe "documents", ->
  doc = null

  beforeEach ->
    doc = new ts.Document()
    doc.setParser(parser)

  it "reads the entire input", ->
    reader = new SpyReader("see spot run. spot runs fast.", 3)
    doc.setInput(reader)
    assert.deepEqual(reader.chunksRead, [
      "see", " sp", "ot ", "run",
      ". s", "pot", " ru", "ns ", "fas", "t.", "", ""
    ])

  it "parses the input", ->
    reader = new SpyReader("see spot run. spot runs fast.", 3)
    doc.setInput(reader)
    assert.equal(doc.toString(), "Document: (paragraph (sentence (word) (word) (word)) (sentence (word) (word) (word)))")
