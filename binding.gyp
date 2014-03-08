{
  "targets": [
    {
      "target_name": "tree_sitter_binding",
      "dependencies": [
        "vendor/tree_sitter/tree_sitter.gyp:tree_sitter",
      ],
      "sources": [
        "src/binding.cc",
        "src/compile.cc",
        "src/document.cc",
        "src/parse_config.cc",
      ],
      'conditions': [
        ['OS == "mac"', {
          'xcode_settings': {
            'MACOSX_DEPLOYMENT_TARGET': '10.7',
          },
        }]
      ]
    },
  ],
}
