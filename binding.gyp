{
  "targets": [
    {
      "target_name": "tree_sitter_runtime_binding",
      "dependencies": [
        "vendor/tree-sitter/project.gyp:runtime",
      ],
      "sources": [
        "src/ast_node.cc",
        "src/ast_node_array.cc",
        "src/binding.cc",
        "src/logger.cc",
        "src/document.cc",
        "src/input_reader.cc",
        "src/conversions.cc",
      ],
      "include_dirs": [
        "include",
        "<!(node -e \"require('nan')\")",
      ],
      'conditions': [
        ['OS == "mac"', {
          'xcode_settings': {
            'MACOSX_DEPLOYMENT_TARGET': '10.9',
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
