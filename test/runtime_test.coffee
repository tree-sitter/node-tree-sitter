assert = require("assert")
SpyReader = require "./helpers/spy_reader"
ts = require("..")
{ repeat } = ts.rules

grammar = ts.grammar
  name: 'trivial_grammar'
  start: 'phrase'
  rules:
    phrase: -> repeat(@word)
    word: -> /\w+/

describe "documents", ->
  it "works", ->
    doc = new ts.Document()
    code = ts.compile(grammar)
    libPath = ts.buildParser("trivial_grammar", code)

    parser = ts.loadParserLib(libPath, "trivial_grammar");
    doc.setParser(parser)

    reader = new SpyReader("the input text", 3)
    doc.setInput(reader)

    assert.equal("hi", doc.toString())
    assert.deepEqual(reader.chunksRead, [
      "the", " in", "put", " te", "xt"
    ])
