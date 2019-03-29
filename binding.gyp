{
  "targets": [
    {
      "target_name": "tree_sitter_runtime_binding",
      "dependencies": ["tree_sitter"],
      "sources": [
        "src/binding.cc",
        "src/conversions.cc",
        "src/language.cc",
        "src/logger.cc",
        "src/node.cc",
        "src/parser.cc",
        "src/tree.cc",
        "src/tree_cursor.cc",
      ],
      "include_dirs": [
        "vendor/tree-sitter/lib/include",
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
    {
      "target_name": "tree_sitter",
      'type': 'static_library',
      "sources": [
        "vendor/tree-sitter/lib/src/lib.c"
      ],
      "include_dirs": [
        "vendor/tree-sitter/lib/src",
        "vendor/tree-sitter/lib/include",
        "vendor/tree-sitter/lib/utf8proc",
      ],
      "cflags": [
        "-std=c99"
      ]
    }
  ],
}
