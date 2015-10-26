function StringInput(string) {
  this.position = 0;
  this.string = string;
};

StringInput.prototype.seek = function(n) {
  this.position = n;
};

StringInput.prototype.read = function() {
  var result = this.string.slice(this.position);
  this.position = this.string.length;
  return result;
};

module.exports = StringInput;
