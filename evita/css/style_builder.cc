// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/style_builder.h"

#include "evita/css/properties.h"
#include "evita/css/style.h"
#include "evita/css/style_editor.h"
#include "evita/css/values.h"

namespace css {

//////////////////////////////////////////////////////////////////////
//
// StyleBuilder
//
StyleBuilder::StyleBuilder(std::unique_ptr<Style> style)
    : style_(std::move(style)) {}
StyleBuilder::StyleBuilder() : style_(new Style()) {}
StyleBuilder::~StyleBuilder() {}

std::unique_ptr<Style> StyleBuilder::Build() {
  return std::move(style_);
}

StyleBuilder& StyleBuilder::Set(PropertyId id, const Length& length) {
  StyleEditor().Set(style_.get(), id, length);
  return *this;
}

StyleBuilder& StyleBuilder::Set(PropertyId id, const ColorValue& color) {
  StyleEditor().Set(style_.get(), id, color);
  return *this;
}

StyleBuilder& StyleBuilder::Set(PropertyId id, const Value& value) {
  StyleEditor().Set(style_.get(), id, value);
  return *this;
}

StyleBuilder& StyleBuilder::SetBorder(const ColorValue& color, float width) {
  SetBorderBottomStyle(BorderBottomStyle::Solid());
  SetBorderBottomWidth(width);
  SetBorderBottomColor(color);
  SetBorderLeftStyle(BorderLeftStyle::Solid());
  SetBorderLeftWidth(width);
  SetBorderLeftColor(color);
  SetBorderRightStyle(BorderRightStyle::Solid());
  SetBorderRightWidth(width);
  SetBorderRightColor(color);
  SetBorderTopStyle(BorderTopStyle::Solid());
  SetBorderTopWidth(width);
  SetBorderTopColor(color);
  return *this;
}

StyleBuilder& StyleBuilder::SetColor(float red,
                                     float green,
                                     float blue,
                                     float alpha) {
  return SetColor(ColorValue(red, green, blue, alpha));
}

StyleBuilder& StyleBuilder::SetPadding(float width) {
  SetPaddingBottom(width);
  SetPaddingLeft(width);
  SetPaddingRight(width);
  SetPaddingTop(width);
  return *this;
}

#define V(Name, name, type, text)                                  \
  StyleBuilder& StyleBuilder::Set##Name(const ColorValue& color) { \
    return Set(PropertyId::Name, color);                           \
  }
FOR_EACH_VISUAL_CSS_COLOR_PROPERTY(V)
#undef V

#define V(Name, name, type, text)                       \
  StyleBuilder& StyleBuilder::Set##Name(float length) { \
    return Set(PropertyId::Name, Length(length));       \
  }
FOR_EACH_VISUAL_CSS_LENGTH_PROPERTY(V)
#undef V

#define V(Name, name, type, text)                    \
  StyleBuilder& StyleBuilder::Set##Name(type name) { \
    StyleEditor().Set##Name(style_.get(), name);     \
    return *this;                                    \
  }

FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V

}  // namespace css
