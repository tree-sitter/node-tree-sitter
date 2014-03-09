var tmp = require("temp"),
    fs = require("fs"),
    path = require("path"),
    sh = require('execSync');

var rootDir = path.join(__dirname, ".."),
    headerDir = path.join(rootDir, "vendor", "tree_sitter", "include"),
    tsLibPath = path.join(rootDir, "build", "Release", "tree_sitter.a");

module.exports = function(name, code) {
  var srcFile = tmp.openSync(),
      srcPath = srcFile.path,
      objPath = srcFile.path + ".o",
      libPath = srcFile.path + ".so";

  fs.writeFileSync(srcPath, code);
  var code1 = sh.run("gcc -x c -fPIC -I " + headerDir + " -c " + srcPath + " -o " + objPath);
  var code2 = sh.run("gcc -shared -Wl " + objPath + " " + tsLibPath + " -o " + libPath);

  return libPath;
};
