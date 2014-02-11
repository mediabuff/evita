// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_api_callback_h)
#define INCLUDE_evita_dom_public_api_callback_h

namespace dom {
class Buffer;
}

namespace domapi {

struct LoadFileCallbackData {
  dom::Buffer* buffer;
  int code_page;
  int error_code;
  FileTime last_write_time;
  NewlineMode newline_mode;
  bool readonly;
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_api_callback_h)
