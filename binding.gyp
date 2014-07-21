{
  "targets": [
    {
      "target_name": "tree_sitter_binding",
      "dependencies": [
        "vendor/tree-sitter/project.gyp:tree_sitter",
      ],
      "sources": [
        "src/ast_node.cc",
        "src/ast_node_array.cc",
        "src/binding.cc",
        "src/compile.cc",
        "src/document.cc",
        "src/input_reader.cc",
        "src/parser.cc",
      ],
      'conditions': [
        ['OS == "mac"', {
          'xcode_settings': {
            'MACOSX_DEPLOYMENT_TARGET': '10.7',
          },
        }]
      ],
      "cflags": [
        "-std=c++0x",
      ],
      'xcode_settings': {
        'CLANG_CXX_LANGUAGE_STANDARD': 'c++11',
      },
    },
  ],
}
