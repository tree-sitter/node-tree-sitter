var Document = require("./binding").Document,
    StringInput = require("./string_input");

Document.prototype.setInputString = function(string) {
  this.setInput(new StringInput(string));
};

var previousSetInput = Document.prototype.setInput;
Document.prototype.setInput = function(input) {
  if (typeof(input.seek) != "function")
    throw new TypeError("Input must implement 'seek'");
  if (typeof(input.read) != "function")
    throw new TypeError("Input must implement 'read'");
  return previousSetInput.call(this, input);
};

module.exports = Document;
