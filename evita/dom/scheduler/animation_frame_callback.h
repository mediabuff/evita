// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_SCHEDULER_ANIMATION_FRAME_CALLBACK_H_
#define EVITA_DOM_SCHEDULER_ANIMATION_FRAME_CALLBACK_H_

#include "base/callback.h"
#include "base/location.h"
#include "base/time/time.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// AnimationFrameCallback
//
class AnimationFrameCallback final {
 public:
  using Callback = base::Callback<void(const base::TimeTicks&)>;

  AnimationFrameCallback(const base::Location& posted_from,
                         const Callback& callback);
  ~AnimationFrameCallback();

  void Run(const base::TimeTicks& time);

 private:
  Callback callback_;

  // The site this PendingTask was posted from.
  base::Location posted_from_;
};

}  // namespace dom

#endif  // EVITA_DOM_SCHEDULER_ANIMATION_FRAME_CALLBACK_H_
