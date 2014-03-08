function blank() {
  return {
    type: "BLANK"
  };
};

function choice() {
  return {
    type: "CHOICE",
    members: normalizeList(arguments)
  };
};

function error(value) {
  return {
    type: "ERROR",
    value: value
  };
}

function pattern(value) {
  return {
    type: "PATTERN",
    value: value
  };
};

function repeat(rule) {
  return {
    type: "REPEAT",
    value: rule
  };
};

function seq() {
  return {
    type: "SEQ",
    members: normalizeList(arguments)
  };
};

function string(value) {
  return {
    type: "STRING",
    value: value
  };
};

function sym(name) {
  return {
    type: "SYMBOL",
    name: name
  };
};

function normalizeList(list) {
  var result = [];
  for (var i = 0; i < list.length; i++) {
    result.push(normalize(list[i]));
  }
  return result;
}

function normalize(value) {
  switch (value.constructor) {
  case String:
    return string(value);
  case RegExp:
    return pattern(value.toString().slice(1, -1));
  default:
    return value;
  }
};

module.exports = {
  sym: sym,
  blank: blank,
  choice: choice,
  error: error,
  string: string,
  pattern: pattern,
  repeat: repeat,
  normalize: normalize,
  seq: seq
};
