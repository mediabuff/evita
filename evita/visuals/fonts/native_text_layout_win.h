// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_FONTS_NATIVE_TEXT_LAYOUT_WIN_H_
#define EVITA_VISUALS_FONTS_NATIVE_TEXT_LAYOUT_WIN_H_

#include <dwrite.h>

#include <wrl/client.h>
#include <memory>

#include "base/macros.h"
#include "base/strings/string16.h"

namespace gfx {
class FloatPoint;
class FloatRect;
class FloatSize;
}

namespace visuals {

class NativeTextFormat;

//////////////////////////////////////////////////////////////////////
//
// NativeTextLayout
//
class NativeTextLayout final {
 public:
  NativeTextLayout(const NativeTextFormat& text_format,
                   const base::string16& text,
                   const gfx::FloatSize& size);
  NativeTextLayout(const NativeTextLayout& other);
  NativeTextLayout(NativeTextLayout&& other);
  ~NativeTextLayout();

  bool operator==(const NativeTextLayout& other) const;
  bool operator!=(const NativeTextLayout& other) const;

  const Microsoft::WRL::ComPtr<IDWriteTextLayout>& get() const {
    return value_;
  }

  gfx::FloatSize GetMetrics() const;
  size_t HitTestPoint(const gfx::FloatPoint& point) const;
  gfx::FloatRect HitTestTextPosition(size_t offset) const;

 private:
  Microsoft::WRL::ComPtr<IDWriteTextLayout> value_;

  DISALLOW_ASSIGN(NativeTextLayout);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_FONTS_NATIVE_TEXT_LAYOUT_WIN_H_
