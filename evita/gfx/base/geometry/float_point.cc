// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <tuple>

#include "evita/gfx/base/geometry/float_point.h"

#include "evita/gfx/base/geometry/float_size.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// FloatPoint
//
FloatPoint::FloatPoint(const std::pair<float, float> pair)
    : FloatPoint(pair.first, pair.second) {}

FloatPoint::FloatPoint(float x, float y) : x_(x), y_(y) {}

FloatPoint::FloatPoint(const FloatPoint& other)
    : FloatPoint(other.x_, other.y_) {}

FloatPoint::FloatPoint() {}
FloatPoint::~FloatPoint() {}

FloatPoint& FloatPoint::operator=(const FloatPoint& other) {
  x_ = other.x_;
  y_ = other.y_;
  return *this;
}

bool FloatPoint::operator==(const FloatPoint& other) const {
  return x_ == other.x_ && y_ == other.y_;
}

bool FloatPoint::operator!=(const FloatPoint& other) const {
  return !operator==(other);
}

bool FloatPoint::operator<(const FloatPoint& other) const {
  return std::tie(y_, x_) < std::tie(other.y_, other.x_);
}

FloatPoint FloatPoint::operator+(const FloatSize& size) const {
  return FloatPoint(x_ + size.width(), y_ + size.height());
}

FloatPoint FloatPoint::operator-(const FloatSize& size) const {
  return FloatPoint(x_ - size.width(), y_ - size.height());
}

std::ostream& operator<<(std::ostream& ostream, const FloatPoint& point) {
  return ostream << '(' << point.x() << ',' << point.y() << ')';
}

}  // namespace gfx
