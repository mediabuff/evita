// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "common/win/rect.h"

#include <algorithm>

namespace common {
namespace win {

Rect Rect::Intersect(const Rect& other) const {
  return Rect(std::max(left, other.left), std::max(top, other.top),
              std::min(right, other.right), std::min(bottom, other.bottom));
}

} // namespace win
} // namespace common
