// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_TYPE_FORWARD_H_
#define JOANA_ANALYZER_TYPE_FORWARD_H_

namespace joana {
namespace analyzer {

#define FOR_EACH_ANALYZE_TYPE(name) \
  V(GenericType)                    \
  V(TypeApplication)                \
  V(TypeName)                       \
  V(TypeParameter)                  \
  V(TypeReference)

#define V(name) class name;
FOR_EACH_ANALYZE_TYPE(V)
#undef V

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_TYPE_FORWARD_H_
