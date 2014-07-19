assert = require("assert")
ts = require("..")
{ seq, error, choice, sym, string, pattern, repeat, blank } = ts.rules

commaSep = (rule) ->
  choice(
    seq(rule, repeat(seq ',', rule)),
    blank())

grammar = ts.grammar
  name: 'the_json_grammar'
  rules:
    value: -> choice @object, @array, @string, @number, @null,
    object: -> seq '{', commaSep(error(@string, ':', @value)), '}'
    array: -> seq '[', commaSep(error(@value)), ']'
    string: -> /"([^"]|\\")+"/,
    number: -> /\d+/
    null: -> 'null'

describe "building grammars", ->
  it "returns a grammar with the given name", ->
    assert.equal(grammar.name, "the_json_grammar")

  it "converts @property references to symbol rules", ->
    assert.deepEqual(
      grammar.rules.value,
      choice(sym("object"), sym("array"), sym("string"), sym("number"), sym("null")))

  it "converts strings to string rules", ->
    assert.deepEqual(
      grammar.rules.null,
      string('null'))

  it "converts regexps to pattern rules", ->
    assert.deepEqual(
      grammar.rules.string,
      pattern('"([^"]|\\\\")+"'))
    assert.deepEqual(
      grammar.rules.number,
      pattern('\\d+'))

  describe "compiling the grammar", ->
    it "returns the C parsing code", ->
      code = ts.compile(grammar)
