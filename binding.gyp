{
  "targets": [
    {
      "target_name": "tree_sitter_runtime_binding",
      "dependencies": [
        "vendor/tree-sitter/project.gyp:runtime",
      ],
      "sources": [
        "src/binding.cc",
        "src/conversions.cc",
        "src/logger.cc",
        "src/node.cc",
        "src/parser.cc",
        "src/tree.cc",
        "src/tree_cursor.cc",
      ],
      "include_dirs": [
        "include",
        "vendor/superstring",
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
