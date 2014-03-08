var tmp = require("temp"),
    fs = require("fs"),
    path = require("path"),
    sh = require('execSync');

var headerDir = path.join(__dirname, "..", "vendor", "tree_sitter", "include");
var tsLibPath = path.join(__dirname, "..", "build", "Release", "tree_sitter.a");

module.exports = function(name, code) {
  var srcFile = tmp.openSync({prefix: "ts_parser_" + name, suffix: ".c"}),
      srcPath = srcFile.path,
      objPath = srcFile.path + ".o",
      libPath = srcFile.path + ".so";

  fs.writeFileSync(srcPath, code);
  var code1 = sh.run("gcc -I " + headerDir + " -fPIC -c " + srcPath + " -o " + objPath);
  var code2 = sh.run("g++ -shared -stdlib=libc++ -Wl " + objPath + " " + tsLibPath + " -o " + libPath);

  return libPath;
};
