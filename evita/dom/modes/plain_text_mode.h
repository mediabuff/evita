// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_modes_plain_text_mode_h)
#define INCLUDE_evita_dom_modes_plain_text_mode_h

#include "evita/dom/modes/mode.h"

namespace dom {

class PlainTextMode : public v8_glue::Scriptable<PlainTextMode, Mode> {
  DECLARE_SCRIPTABLE_OBJECT(PlainTextMode);

  public: PlainTextMode(Document* document, text::ModeFactory* mode_factory);
  public: ~PlainTextMode();

  DISALLOW_COPY_AND_ASSIGN(PlainTextMode);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_modes_plain_text_mode_h)
