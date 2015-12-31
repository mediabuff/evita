// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/model/box.h"

#include "evita/visuals/model/box_visitor.h"
#include "evita/visuals/model/text_box.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_builder.h"

namespace visuals {

namespace {

//////////////////////////////////////////////////////////////////////
//
// ActualStyleVisitor
//
class ActualStyleVisitor final : public BoxVisitor {
 public:
  ActualStyleVisitor() = default;
  ~ActualStyleVisitor() final = default;

  std::unique_ptr<css::Style> Compute(const Box& box);

 private:
#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  css::StyleBuilder builder_;

  DISALLOW_COPY_AND_ASSIGN(ActualStyleVisitor);
};

#define FOR_EACH_PROPERTY_INITIAL_IS_AUTO(V) \
  V(Bottom, bottom)                          \
  V(Left, left)                              \
  V(Right, right)                            \
  V(Top, top)

std::unique_ptr<css::Style> ActualStyleVisitor::Compute(const Box& box) {
  if (box.background().HasValue())
    builder_.SetBackground(box.background());
  if (box.border().HasValue())
    builder_.SetBorder(box.border());
  if (box.is_display_none())
    builder_.SetDisplay(css::Display::None());
  if (box.margin().HasValue())
    builder_.SetMargin(box.margin());
  if (box.padding().HasValue())
    builder_.SetPadding(box.padding());

  // CSS Position
  if (!box.position().is_static())
    builder_.SetPosition(box.position());

#define V(Property, property)    \
  if (!box.property().is_auto()) \
    builder_.Set##Property(box.property());
  FOR_EACH_PROPERTY_INITIAL_IS_AUTO(V)
#undef V

  Visit(box);
  return std::move(builder_.Build());
}

void ActualStyleVisitor::VisitBlockBox(BlockBox* block) {}

void ActualStyleVisitor::VisitLineBox(LineBox* line) {}

void ActualStyleVisitor::VisitTextBox(TextBox* text) {
  builder_.SetColor(css::Color(text->color()));
}

}  // namespace

std::unique_ptr<css::Style> Box::ComputeActualStyle() const {
  ActualStyleVisitor builder;
  return builder.Compute(*this);
}

}  // namespace visuals
