// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_common_win_size_h)
#define INCLUDE_common_win_size_h

#include "common/common_export.h"

namespace common {
namespace win {

struct COMMON_EXPORT Size : SIZE {
  Size() {
    cx = cy = 0;
  }

  Size(int cx, int cy) {
    this->cx = cx;
    this->cy = cy;
  }

  operator bool() const { return !is_empty(); }
  bool is_empty() const { return cx <= 0 || cx <= 0; }
};

#define DEBUG_SIZE_FORMAT "%dx%d"
#define DEBUG_SIZE_ARG(mp_size) \
 (mp_size).cx, (mp_size).cy

} // namespace win
} // namespace common

#endif //!defined(INCLUDE_common_win_size_h)
