var rules = require("./rules");

module.exports = function(grammarHash) {
  var result = {
    name: grammarHash.name,
    version: grammarHash.version,
    rules: {}
  };

  var ruleNames = Object.keys(grammarHash.rules)
  var builder = new RuleBuilder(ruleNames);

  ruleNames.forEach(function(ruleName) {
    var ruleFn = grammarHash.rules[ruleName];
    result.rules[ruleName] = rules.normalize(ruleFn.call(builder));
  });

  return result;
}

function RuleBuilder(ruleNames) {
  var self = this;
  ruleNames.forEach(function(name) {
    self[name] = rules.sym(name);
  });
}
