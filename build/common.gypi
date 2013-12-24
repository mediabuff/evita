# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'variables': {
      'variables': {
        'clang': 0,
        'component%': 'shared_library', # or 'static_library'
        'target_arch%': 'ia32', # 'ia32', 'x64'
      }, # variables
      'clang': '<(clang)',
      'component%': '<(component)',
      'target_arch%': '<(target_arch)',
    }, # variables
    'clang%': '<(clang)',
    'component%': '<(component)',
    'target_arch%': '<(target_arch)',

    # Relative path to icu.gyp from this file.
    'icu_gyp_path%': '<(DEPTH)/third_party/icu/icu.gyp',
    'icu_use_data_file_flag%': 0,
    'os_posix': 0,
    'target_arch%': '<(target_arch)',
    'v8_enable_i18n_support': 1,
    'v8_optimized_debug%': '(<v8_optimized_debug)',

    # TODO(yosi) Should remove following variables.
    #'win_release_RuntimeLibrary%': 2, # /MD
    #'win_debug_RuntimeLibrary%': 3, # /MDd
  }, # variables

  'includes': [
    'common_chromium.gypi',
    'common_evita.gypi',
    'common_third_party.gypi',
  ], # includes

  'target_defaults': {
    'variables': {
      'chromium_code%': '<(chromium_code)',
      'evita_code%': '<(evita_code)',
     }, # variables
     'conditions': [
       ['OS=="win" and component=="shared_library"', {
          'variables': {
            'win_release_RuntimeLibrary%': 2, # /MD
            'win_debug_RuntimeLibrary%': 3, # /MDd
          }
       }, {
          'variables': {
            'win_release_RuntimeLibrary%': 0, # /MT
            'win_debug_RuntimeLibrary%': 1, # /MTd
          }
      }], # OS=="win" and component=="shared_library"
      ['component=="shared_library"', {
        'defines': ['COMPONENT_BUILD'],
      }] # component=="shared_library"
    ], # conditions

    'msvs_cygwin_dirs': ['<(DEPTH)/third_party/cygwin'],
    'msvs_cygwin_shell': 0,

    'default_configuration': 'Debug',
    'configurations': {
      'Common_Base': {
        'abstract': 1,

        'msvs_configuration_attributes': {
          'OutputDirectory': '<(DEPTH)\\..\$(ConfigurationName)',
          'IntermediateDirectory': '<(DEPTH)\\..\$(ConfigurationName)\\$(ProjectName)',
          'CharacterSet': '1',
        },
      }, # Common_Base

      'x86_Base': {
        'abstract': 1,
        'msvs_settings': {
          'VCLinkerTool': {
            'AdditionalOptions': [
              '/safeseh',
              #'/dynamicbase',
              '/nxcompat',
            ], # AdditionalOptions
            'TargetMachine': '1',
          },
        }, # msvs_settings
      }, # x86_Base

      'x64_Base': {
        'abstract': 1,
        'msvs_settings': {
          'VCLinkerTool': {
            'AdditionalOptions': [
              # safeseh is not compatible with x64
              '/dynamicbase',
              '/ignore:4199',
              '/ignore:4221',
              '/nxcompat',
            ],
          },
        },
      }, # x64_Base

      'Debug_Base': {
        'abstract': 1,
        'msvs_settings': {
          'VCCLCompilerTool': {
            'AdditionalOptions': [
              #'/GF-', # Enables string pooling.
              #'/GS', # Buffer security check
              #'/Gm-', # Enables minimal rebuild.
              #'/Gy-', # Enables function-level linking.
              #'/MTd', # Creates a debug multithreaded executable file using LIBCMTD.lib.
              #'/Oy-', # Omits frame pointer (x86 only).
              #'/Zi', # Generates complete debugging information.
            ],
            'Optimization': '0', # /Od
            'PreprocessorDefinitions': ['_DEBUG'],
            'RuntimeLibrary': '<(win_debug_RuntimeLibrary)',
            'conditions': [
              ['evita_code==1', {
                'AdditionalOptions': [
                  '/RTC1', # Enables run-time error checking.
                ],
              }],
            ], # conditions
          }, # VCCLCompilerTool
          'VCLinkerTool': {
            'GenerateDebugInformation': 'true',
            'GenerateMapFile': 'true',
          }, # VCLinkerTool
        },
      }, # Debug_Base

      'Release_Base': {
        'abstract': 1,
        'msvs_settings': {
          # Note: GYP add /FS(Forces writes to the program database (PDB)
          # file to be serialized through MSPDBSRV.EXE.) for MSVS=|2013|
          # or # |2013e|.
          'VCCLCompilerTool': {
            'AdditionalOptions': [
              '/GF', # Enables string pooling.
              '/GL', # Enables whole program optimization.
              '/GS-', # Buffer security check
              '/Gm-', # Enables minimal rebuild.
              '/MT', # Creates a multithreaded executable file using LIBCMT.lib.
              '/Oi', # Generates intrinsic functions.
              '/Oy', # Omits frame pointer (x86 only).
            ],
            'EnableIntrinsicFunctions': 'true', # /Gy
            'EnableFiberSafeOptimizations': 'true', # /GT
            'FavorSizeOrSpeed': '2', # /Os Favors small code.
            'PreprocessorDefinitions': ['NDEBUG'],
            'Optimization': '3',
            'RuntimeLibrary': '<(win_release_RuntimeLibrary)',
            'WholeProgramOptimization': 'true', # /Ox
          }, # VCCLCompilerTool
          'VCLinkerTool': {
            'EnableCOMDATFolding': '2',
            'GenerateMapFile': 'true',
            'OptimizeReferences': '2',
          }, # VCLinkerTool
        },
      }, # Release_Base

      # Concrete configurations
      'Debug' : {
        'inherit_from': ['Common_Base', 'x86_Base', 'Debug_Base'],
      },

      'Release' : {
        'inherit_from': ['Common_Base', 'x86_Base', 'Release_Base'],
      },
      'Debug_x64' : {
        'inherit_from': ['Common_Base', 'x64_Base', 'Debug_Base'],
      },
      'Release_x64' : {
        'inherit_from': ['Common_Base', 'x64_Base', 'Release_Base'],
      },
    }, # configurations
  }, # target_defaults

  'conditions': [
    ['OS=="win"', {
      'target_defaults': {
        'msvs_settings': {
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'kernel32.lib',
            ], # AdditionalDependencies
            'FixBaseAddress': '1',
            'GenerateDebugInformation': 'true',
            'ImportLibrary': '$(OutDir)\\lib\\$(TargetName).lib',
            'MapFileName': '$(OutDir)\\$(TargetName).map',
            'SubSystem': 1, # /SUBSYSTEM:CONSOLE
          }, # VCLinkerTool
        }, # msvs_settings
      }, # 'target_defaults'
    }], # OS=="win"
  ], # 'conditions',
}
