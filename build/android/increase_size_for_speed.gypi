# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file is meant to be included to optimize a target for speed
# rather than for size on Android.
# This is used in some carefully tailored targets and is not meant
# to be included everywhere. Before adding the template to another target,
# please ask in chromium-dev@. See crbug.com/411909

{
  'configurations': {
    'Release': {
      'target_conditions': [
        ['_toolset=="target"', {
          'conditions': [
            ['OS=="android"', {
              'cflags!': ['-Os'],
              'cflags': ['-O2'],
            }],
          ],
        }],
      ],
    },
  },
}
