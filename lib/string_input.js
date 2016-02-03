function StringInput(string, bufferSize) {
  this.position = 0;
  this.string = string;
  this.bufferSize = Number.isFinite(bufferSize) ? bufferSize : null;
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
