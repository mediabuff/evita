// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_DEFERRED_H_
#define EVITA_DOM_PUBLIC_DEFERRED_H_

#include "base/callback_forward.h"
#include "base/strings/string16.h"

namespace domapi {

template <typename ResolveType, typename RejectType = int>
struct Deferred final {
  base::Callback<void(RejectType)> reject;
  base::Callback<void(ResolveType)> resolve;
  int sequence_num;
};

using IntegerPromise = domapi::Deferred<int>;
using StringPromise = domapi::Deferred<base::string16>;

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_DEFERRED_H_
