{
  "targets": [
    {
      "target_name": "tree_sitter_runtime_binding",
      "dependencies": [
        "tree_sitter",
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "defines": [
        "NAPI_DISABLE_CPP_EXCEPTIONS=",
      ],
      "sources": [
        "src/binding.cc",
        "src/conversions.cc",
        "src/language.cc",
        "src/logger.cc",
        "src/node.cc",
        "src/parser.cc",
        "src/query.cc",
        "src/tree.cc",
        "src/tree_cursor.cc",
        "src/util.cc",
      ],
      "include_dirs": [
        "vendor/tree-sitter/lib/include",
        "vendor/superstring",
        "<!(node -e \"require('nan')\")",
        "<!@(node -p \"require('node-addon-api').include\")",
      ],
      'conditions': [
        ['OS == "mac"', {
          'xcode_settings': {
            'MACOSX_DEPLOYMENT_TARGET': '10.9',
          },
        }]
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
      ],
      "cflags": [
        "-std=c99"
      ]
    }
  ],
}
