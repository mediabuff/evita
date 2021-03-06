// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_STROKE_STYLE_H_
#define EVITA_GFX_STROKE_STYLE_H_

#include "common/win/scoped_comptr.h"
#include "evita/gfx/forward.h"

namespace gfx {

enum class CapStyle {
  Flat = D2D1_CAP_STYLE_FLAT,
  Round = D2D1_CAP_STYLE_ROUND,
  Square = D2D1_CAP_STYLE_SQUARE,
  Triangle = D2D1_CAP_STYLE_TRIANGLE,
};

enum class DashStyle {
  Solid = D2D1_DASH_STYLE_SOLID,
  Dash = D2D1_DASH_STYLE_DASH,
  Dot = D2D1_DASH_STYLE_DOT,
  DashDot = D2D1_DASH_STYLE_DASH_DOT,
  DashDotDot = D2D1_DASH_STYLE_DASH_DOT_DOT,
  Custom = D2D1_DASH_STYLE_CUSTOM,
};

enum class LineJoin {
  Miter = D2D1_LINE_JOIN_MITER,
  Bevel = D2D1_LINE_JOIN_BEVEL,
  Round = D2D1_LINE_JOIN_ROUND,
  MiterOrBevel = D2D1_LINE_JOIN_MITER_OR_BEVEL,
};

enum class StrokeTransform {
  Normal = D2D1_STROKE_TRANSFORM_TYPE_NORMAL,
  Fixed = D2D1_STROKE_TRANSFORM_TYPE_FIXED,
  Hairline = D2D1_STROKE_TRANSFORM_TYPE_HAIRLINE,
};

class StrokeStyle final {
 public:
  class Builder;

  StrokeStyle(const StrokeStyle& other);
  StrokeStyle(StrokeStyle&& other);
  ~StrokeStyle();

  operator ID2D1StrokeStyle*() const;

  StrokeStyle& operator=(const StrokeStyle& other);
  StrokeStyle& operator=(StrokeStyle&& other);

 private:
  explicit StrokeStyle(common::ComPtr<ID2D1StrokeStyle1>&& platform_style);

  common::ComPtr<ID2D1StrokeStyle1> platform_style_;
};

}  // namespace gfx

#endif  // EVITA_GFX_STROKE_STYLE_H_
