// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/display/public/display_items.h"

#include "evita/visuals/fonts/text_layout.h"

namespace visuals {

namespace {
int last_display_item_id_;
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// DisplayItem
//
DisplayItem::DisplayItem() : id_(++last_display_item_id_) {}
DisplayItem::~DisplayItem() {}

bool DisplayItem::operator==(const DisplayItem& other) const {
  if (this == &other)
    return true;
  return EqualsTo(other);
}

bool DisplayItem::operator!=(const DisplayItem& other) const {
  return !operator==(other);
}

//////////////////////////////////////////////////////////////////////
//
// BeginClipDisplayItem
//
BeginClipDisplayItem::BeginClipDisplayItem(const gfx::FloatRect& bounds)
    : bounds_(bounds) {}
BeginClipDisplayItem::~BeginClipDisplayItem() {}

bool BeginClipDisplayItem::EqualsTo(const DisplayItem& other) const {
  if (this == &other)
    return false;
  const auto& item = other.as<BeginClipDisplayItem>();
  if (!item)
    return false;
  return bounds_ == item->bounds_;
}

//////////////////////////////////////////////////////////////////////
//
// BeginTransformDisplayItem
//
BeginTransformDisplayItem::BeginTransformDisplayItem(
    const gfx::FloatMatrix3x2& matrix)
    : matrix_(matrix) {}
BeginTransformDisplayItem::~BeginTransformDisplayItem() {}

bool BeginTransformDisplayItem::EqualsTo(const DisplayItem& other) const {
  if (this == &other)
    return false;
  const auto& item = other.as<BeginTransformDisplayItem>();
  if (!item)
    return false;
  return matrix_ == item->matrix_;
}

//////////////////////////////////////////////////////////////////////
//
// ClearDisplayItem
//
ClearDisplayItem::ClearDisplayItem(const gfx::FloatColor& color)
    : color_(color) {}
ClearDisplayItem::~ClearDisplayItem() {}

bool ClearDisplayItem::EqualsTo(const DisplayItem& other) const {
  if (this == &other)
    return false;
  const auto& item = other.as<ClearDisplayItem>();
  if (!item)
    return false;
  return color_ == item->color_;
}

//////////////////////////////////////////////////////////////////////
//
// DrawBitmapDisplayItem
//
DrawBitmapDisplayItem::DrawBitmapDisplayItem(const gfx::FloatRect& destination,
                                             const ImageBitmap& bitmap,
                                             const gfx::FloatRect& source,
                                             float opacity)
    : bitmap_(bitmap),
      destination_(destination),
      opacity_(opacity),
      source_(source) {}

DrawBitmapDisplayItem::~DrawBitmapDisplayItem() {}

bool DrawBitmapDisplayItem::EqualsTo(const DisplayItem& other) const {
  if (this == &other)
    return false;
  const auto& item = other.as<DrawBitmapDisplayItem>();
  if (!item)
    return false;
  return bitmap_ == item->bitmap_ && destination_ == item->destination_ &&
         opacity_ == item->opacity_ && source_ == item->source_;
}

//////////////////////////////////////////////////////////////////////
//
// DrawLineDisplayItem
//
DrawLineDisplayItem::DrawLineDisplayItem(const gfx::FloatPoint& point1,
                                         const gfx::FloatPoint& point2,
                                         const gfx::FloatColor& color,
                                         float thickness)
    : color_(color), point1_(point1), point2_(point2), thickness_(thickness) {}
DrawLineDisplayItem::~DrawLineDisplayItem() {}

bool DrawLineDisplayItem::EqualsTo(const DisplayItem& other) const {
  if (this == &other)
    return false;
  const auto& item = other.as<DrawLineDisplayItem>();
  if (!item)
    return false;
  return color_ == item->color_ && point1_ == item->point1_ &&
         point2_ == item->point2_ && thickness_ == item->thickness_;
}

//////////////////////////////////////////////////////////////////////
//
// DrawRectDisplayItem
//
DrawRectDisplayItem::DrawRectDisplayItem(const gfx::FloatRect& bounds,
                                         const gfx::FloatColor& color,
                                         float thickness)
    : bounds_(bounds), color_(color), thickness_(thickness) {}
DrawRectDisplayItem::~DrawRectDisplayItem() {}

bool DrawRectDisplayItem::EqualsTo(const DisplayItem& other) const {
  if (this == &other)
    return false;
  const auto& item = other.as<DrawRectDisplayItem>();
  if (!item)
    return false;
  return bounds_ == item->bounds_ && color_ == item->color_ &&
         thickness_ == item->thickness_;
}

//////////////////////////////////////////////////////////////////////
//
// DrawTextDisplayItem
//
DrawTextDisplayItem::DrawTextDisplayItem(const gfx::FloatRect& bounds,
                                         const gfx::FloatColor& color,
                                         float baseline,
                                         const TextLayout& text_layout,
                                         const base::string16& text)
    : baseline_(baseline),
      bounds_(bounds),
      color_(color),
      text_(text),
      text_layout_(std::move(text_layout)) {}

DrawTextDisplayItem::~DrawTextDisplayItem() {}

bool DrawTextDisplayItem::EqualsTo(const DisplayItem& other) const {
  if (this == &other)
    return false;
  const auto& item = other.as<DrawTextDisplayItem>();
  if (!item)
    return false;
  return baseline_ == item->baseline_ && bounds_ == item->bounds_ &&
         color_ == item->color_ && text_layout_ == item->text_layout_;
}

//////////////////////////////////////////////////////////////////////
//
// EndClipDisplayItem
//
EndClipDisplayItem::EndClipDisplayItem() {}
EndClipDisplayItem::~EndClipDisplayItem() {}

bool EndClipDisplayItem::EqualsTo(const DisplayItem& other) const {
  if (this == &other)
    return false;
  return other.is<EndClipDisplayItem>();
}

//////////////////////////////////////////////////////////////////////
//
// EndTransformDisplayItem
//
EndTransformDisplayItem::EndTransformDisplayItem() {}
EndTransformDisplayItem::~EndTransformDisplayItem() {}

bool EndTransformDisplayItem::EqualsTo(const DisplayItem& other) const {
  if (this == &other)
    return false;
  return other.is<EndTransformDisplayItem>();
}

//////////////////////////////////////////////////////////////////////
//
// FillRectDisplayItem
//
FillRectDisplayItem::FillRectDisplayItem(const gfx::FloatRect& bounds,
                                         const gfx::FloatColor& color)
    : bounds_(bounds), color_(color) {}
FillRectDisplayItem::~FillRectDisplayItem() {}

bool FillRectDisplayItem::EqualsTo(const DisplayItem& other) const {
  if (this == &other)
    return false;
  const auto& item = other.as<FillRectDisplayItem>();
  if (!item)
    return false;
  return bounds_ == item->bounds_ && color_ == item->color_;
}

}  // namespace visuals
