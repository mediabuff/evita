# This file is used to manage the dependencies of the Evita src repo. It is
# used by gclient to determine what version of each dependency to check out, and
# where.

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'github_git': 'https://github.com',

  'base_revision': 'debb89a9613f31ee1aed22ced049801c983a5ebb',
  'build_revision': 'be729974cabf69294b1fd5244294e97db9fbf7f7',
  'buildtools_revision': 'df36429e1847e38fda6a7413ba1aa2cbafbc4a34',
  'ced_revision': '94c367a1fe3a13207f4b22604fcfd1d9f9ddf6d9',
  'depot_tools_revision': '4810aa703e61eb49aa0b5c5ca8379d9b5cdf6889',
  'googletest_revision': '7f8fefabedf2965980585be8c2bff97458f28e0b',
  'grit_revision': '3753363e0608ada9cc10ac908cf7ff33ec304c9a',
  'gyp_revision': 'd61a9397e668fa9843c4aa7da9e79460fe590bfb',
  'icu_revision': '02218e5349740696eada56a1ba4db988f41b3f51',
  'idl_parser_revision': '5fb1ebf49d235b5a70c9f49047e83b0654031eb7',
  'instrumented_libraries_revision': 'e07d437dc8b65ca96ebd3b7d4aa303cd2ec0ec45',
  'jinja2_revision': 'd34383206fa42d52faa10bb9931d6d538f3a57e0',
  'markupsafe_revision': '8f45f5cfa0009d2a70589bcda0349b8cb2b72783',
  'modp_b64_revision': '28e3fbba4cb4ec3ffd85b53d0a3904525d08f5a6',
  'ply_revision': '4a6baf95860033d4c69d3e3087696b30c687622c',
  'testing_revision': '5414bae87f43211483bb7ee09f251a189eb28bad',
  'tools_clang_revision': '723b25997f0aab45fe1776a0f74a14782e350f8f',
  'tools_win_revision': '8878b12abb3c73d8a9c2c4ba7385c8b51d1ed406',
  'v8_revision': 'd0225ccd551272ffab2b9cce98e36b2fcd59275c', # 6.4.200.1
  'zlib_revision': 'd6c529516088eb603ea77742ccfe3b3f1dadf489',

  # github
  'autopep8_revision': '93fff80de7dd0819b36281bc0868e5b17c81fb7f', # 1.3.2
  'pep8_revision': '8053c7c1d5597b062c58c5991231b62e11b435e9', # 2.2.0
}

deps = {
  # From chromium_git
  'src/base':
    Var('chromium_git') + '/chromium/src/base' + '@' +  Var('base_revision'),

  'src/build':
    Var('chromium_git') + '/chromium/src/build' + '@' +  Var('build_revision'),

  'src/buildtools':
    Var('chromium_git') + '/chromium/buildtools.git' + '@' +  Var('buildtools_revision'),

  'src/testing':
    Var('chromium_git') + '/chromium/src/testing' + '@' + Var('testing_revision'),

  'src/third_party/ced/src':
    Var('chromium_git') + '/external/github.com/google/compact_enc_det.git' + '@' + Var('ced_revision'),

 'src/third_party/googletest/src':
    Var('chromium_git') + '/external/github.com/google/googletest.git' + '@' + Var('googletest_revision'),

  'src/tools/grit':
    Var('chromium_git') + '/chromium/src/tools/grit' + '@' + Var('grit_revision'),

  'src/tools/gyp':
    Var('chromium_git') + '/external/gyp.git' + '@' + Var('gyp_revision'),

  'src/third_party/icu':
    Var('chromium_git') + '/chromium/deps/icu.git' + '@' + Var('icu_revision'),

  'src/tools/idl_parser':
    Var('chromium_git') + '/chromium/src/tools/idl_parser' + '@' + Var('idl_parser_revision'),

  'src/third_party/depot_tools':
    Var('chromium_git') + '/chromium/tools/depot_tools.git' + '@' + Var('depot_tools_revision'),

  'src/third_party/instrumented_libraries':
    Var('chromium_git') + '/chromium/src/third_party/instrumented_libraries.git' + '@' + Var('instrumented_libraries_revision'),

  'src/third_party/jinja2':
    Var('chromium_git') + '/chromium/src/third_party/jinja2.git' + '@' + Var('jinja2_revision'),

  'src/third_party/markupsafe':
    Var('chromium_git') + '/chromium/src/third_party/markupsafe.git' + '@' + Var('markupsafe_revision'),

  'src/third_party/modp_b64':
    Var('chromium_git') + '/chromium/src/third_party/modp_b64.git' + '@' + Var('modp_b64_revision'),

  'src/third_party/ply':
    Var('chromium_git') + '/chromium/src/third_party/ply' + '@' +  Var('ply_revision'),

  'src/tools/clang':
    Var('chromium_git') + '/chromium/src/tools/clang' + '@' +  Var('tools_clang_revision'),

  'src/tools/win':
    Var('chromium_git') + '/chromium/src/tools/win' + '@' +  Var('tools_win_revision'),

  'src/v8':
    Var('chromium_git') + '/v8/v8.git' + '@' +  Var('v8_revision'),

  'src/third_party/zlib':
    Var('chromium_git') + '/chromium/src/third_party/zlib' + '@' +  Var('zlib_revision'),

  # From github
  'src/third_party/autopep8':
    Var('github_git') + '/hhatto/autopep8.git' + '@' + Var('autopep8_revision'),

  'src/third_party/pep8':
    Var('github_git') + '/PyCQA/pep8.git' + '@' + Var('pep8_revision'),
}

hooks = [
  {
    # Update LASTCHANGE.
    'name': 'lastchange',
    'pattern': '.',
    'action': ['python', 'src/build/util/lastchange.py',
               '-o', 'src/build/util/LASTCHANGE'],
  },

  # Pull GN binaries. This needs to be before running GYP below.
  {
    'name': 'gn_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'src/buildtools/win/gn.exe.sha1',
    ],
  },
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'src/buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    # Pull clang if needed or requested via GYP_DEFINES.
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'action': ['python', 'src/tools/clang/scripts/update.py', '--if-needed'],
  },
  {
    'pattern': '.',
    'action': ['src\\evita\\build\\gn_evita.cmd']
  },
  # Pull rc binaries using checked-in hashes.
  {
    'name': 'rc_win',
    'pattern': '.',
    'condition': 'checkout_win and host_os == "win"',
    'action': [ 'python',
                'src/third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-browser-clang/rc',
                '-s', 'src/build/toolchain/win/rc/win/rc.exe.sha1',
    ],
  },
]

recursedeps = [
  # buildtools provides clang_format, libc++, and libc++abi
  'src/buildtools',
]
