# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    # .gyp files or targets should set |evita_code| to 1 if they build
    # Evita-specific code, as opposed to external code. This variable is
    # used to control such things as the set of warnings to enable, and
    # whether warnings are treated as errors.
    'evita_code%': 0,
  }, # variables

  'target_defaults': {
    'target_conditions': [
      ['evita_code==1', {
        'include_dirs': [
          '<(DEPTH)',
        ], # include_dirs
        'conditions': [
            ['OS=="win"', {
                'defines': [
                  'NOMINMAX',
                  # See SDK version in include/shared/sdkddkver.h
                  '_WIN32_WINNT=0x0602', # _WIN32_WINNT_WIN8
                  '_WINDOWS',
                  'WIN32',
                  'WIN32_LEAN_AND_MEAN',
                  'WINVER=0x0602', # _WIN32_WINNT_WIN8
                ], # defines

                # Precompiled header
                # See gyp/pylib/gyp/msvs_settings.py for details
                #'msvs_precompiled_header': '<(DEPTH)/build/precomp.h',
                #'msvs_precompiled_source': '<(DEPTH)/build/precomp.cc',
                #'sources': ['<(DEPTH)/build/precomp.cc'],

                'msvs_settings': {
                  'VCCLCompilerTool': {
                    'ExceptionHandling': '0',
                    'RuntimeTypeInfo': 'false', # GR-
                    'WarningLevel': 'all', # /Wall
                    'WarnAsError': 'true', # /WX
                    'AdditionalOptions': [
                      #'/Gr', # Uses the __fastcall calling convention (x86
                      # only).
                      '/analyze-',
                      '/arch:SSE2',
                      '/errorReport:prompt',
                      '/fp:except-',
                      '/fp:fast',
                    ],
                  }, # VCCLCompilerTool
                  'VCLinkerTool': {
                    'AdditionalDependencies': [
                      'kernel32.lib',
                      'advapi32.lib',
                      'user32.lib',
                      'ws2_32.lib', # for libxml
                    ], # AdditionalDependencies
                  }, # VCLinkerTool
                  'target_conditions': [
                    ['_type=="executable"', {
                        'VCManifestTool': {
                          'EmbedManifest': 'true',
                         },
                      }],
                  ], # target_conditions
                }, # msvs_settings
                'msvs_disabled_warnings': [
                  # TODO(yosi): We should not disable warning C4350.
                  # L1 C4350: behavior change: 'member1' called instead of
                  # 'member2' An rvalue cannot be bound to a non-const
                  # reference. In previous versions of Visual C++, it was
                  # possible to bind an rvalue to a non-const reference in a
                  # direct initialization. This code now gives a warning.
                  4350,
                  # All C4355: 'this' : used in base member initializer list
                  4355,
                  # L4 C4514: 'function' : unreferenced inline function has
                  # been removed
                  4514,
                  # L4 C4668: 'symbol' is not defined as a preprocessor macro,
                  # replacing with '0' for 'directives'
                  4668,
                  # L4 C4820: 'bytes' bytes padding added after construct
                  # 'member_name'
                  4820,
                ], # msvs_disabled_warnings
            }], # OS=="win"
        ], # conditions
    }], # evita_code==1
  ], # target_conditions
 } # target_defaults
}

