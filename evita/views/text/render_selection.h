// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_render_selection_h)
#define INCLUDE_evita_views_text_render_selection_h

#include "evita/css/style.h"
#include "evita/gfx/color_f.h"

namespace views {
namespace rendering {

struct Selection {
  bool active;
  text::Posn start;
  text::Posn end;
};

struct TextSelection {
  gfx::ColorF color;
  text::Posn start;
  text::Posn end;

  TextSelection(const gfx::ColorF& color, text::Posn start, text::Posn end);
  TextSelection();
  ~TextSelection();

  bool operator==(const TextSelection& other) const;
  bool operator!=(const TextSelection& other) const;
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_render_selection_h)
