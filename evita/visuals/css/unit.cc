// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <ostream>

#include "evita/visuals/css/unit.h"

namespace visuals {
namespace css {

std::ostream& operator<<(std::ostream& ostream, Unit unit) {
  static const char* const texts[] = {
#define V(name) #name,
      FOR_EACH_CSS_UNIT(V)
#undef V
  };
  const auto& it = std::begin(texts) + static_cast<size_t>(unit);
  if (it < std::begin(texts) || it >= std::end(texts))
    return ostream << "???";
  return ostream << *it;
}

}  // namespace css
}  // namespace visuals