// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/visuals/layout/size_calculator.h"

#include "base/logging.h"
#include "base/trace_event/trace_event.h"
#include "evita/gfx/base/geometry/float_rect.h"
#include "evita/visuals/fonts/text_format.h"
#include "evita/visuals/imaging/image_bitmap.h"
#include "evita/visuals/layout/border.h"
#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/box_visitor.h"
#include "evita/visuals/layout/flow_box.h"
#include "evita/visuals/layout/image_box.h"
#include "evita/visuals/layout/margin.h"
#include "evita/visuals/layout/padding.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/shape_box.h"
#include "evita/visuals/layout/text_box.h"

namespace visuals {

namespace {

// TODO(eval1749) We should move |IsDisplayOutsideInline()| to |css::Display|.
bool IsDisplayOutsideInline(const css::Display& display) {
  return display.is_inline() || display.is_inline_block();
}

//////////////////////////////////////////////////////////////////////
//
// ExtrinsicSizeVisitor
//
class ExtrinsicSizeVisitor final : public BoxVisitor {
 public:
  ExtrinsicSizeVisitor() = default;
  ~ExtrinsicSizeVisitor() final = default;

  gfx::FloatSize ComputeExtrinsicSize(const Box& box);

 private:
  void ComputeWithSimpleMethod(const Box& box);
  void ReturnSize(float width, float height);
  void ReturnSize(const gfx::FloatSize& size);

#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  gfx::FloatSize result_;

  DISALLOW_COPY_AND_ASSIGN(ExtrinsicSizeVisitor);
};

gfx::FloatSize ExtrinsicSizeVisitor::ComputeExtrinsicSize(const Box& box) {
  Visit(const_cast<Box*>(&box));
  return result_;
}

void ExtrinsicSizeVisitor::ComputeWithSimpleMethod(const Box& box) {
  if (box.height().is_length() && box.width().is_length()) {
    ReturnSize(box.width().as_length().value(),
               box.height().as_length().value());
    return;
  }
  const auto& intrinsic_size = SizeCalculator().ComputeIntrinsicSize(box);
  if (box.height().is_length())
    return ReturnSize(intrinsic_size.width(), box.height().as_length().value());
  if (box.width().is_length())
    return ReturnSize(box.width().as_length().value(), intrinsic_size.height());
  ReturnSize(intrinsic_size);
}

void ExtrinsicSizeVisitor::ReturnSize(float width, float height) {
  ReturnSize(gfx::FloatSize(std::ceil(width), std::ceil(height)));
}

void ExtrinsicSizeVisitor::ReturnSize(const gfx::FloatSize& size) {
  result_ = gfx::FloatSize(std::ceil(size.width()), std::ceil(size.height()));
}

// BoxVisitor
void ExtrinsicSizeVisitor::VisitFlowBox(FlowBox* box) {
  ComputeWithSimpleMethod(*box);
}

void ExtrinsicSizeVisitor::VisitImageBox(ImageBox* box) {
  ComputeWithSimpleMethod(*box);
}

void ExtrinsicSizeVisitor::VisitRootBox(RootBox* box) {
  ComputeWithSimpleMethod(*box);
}

void ExtrinsicSizeVisitor::VisitShapeBox(ShapeBox* box) {
  ComputeWithSimpleMethod(*box);
}

void ExtrinsicSizeVisitor::VisitTextBox(TextBox* box) {
  ComputeWithSimpleMethod(*box);
}

//////////////////////////////////////////////////////////////////////
//
// IntrinsicSizeVisitor
//
class IntrinsicSizeVisitor final : public BoxVisitor {
 public:
  IntrinsicSizeVisitor() = default;
  ~IntrinsicSizeVisitor() final = default;

  gfx::FloatSize ComputeIntrinsicSize(const Box& box);

  // Shortcut
  gfx::FloatSize ComputePreferredSize(const Box& box);

 private:
  gfx::FloatSize SizeOfHorizontalFlowBox(const FlowBox& flow_box);
  void ReturnSize(const gfx::FloatSize& size);
  gfx::FloatSize SizeOfVerticalFlowBox(const FlowBox& flow_box);

#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  gfx::FloatSize result_;

  DISALLOW_COPY_AND_ASSIGN(IntrinsicSizeVisitor);
};

gfx::FloatSize IntrinsicSizeVisitor::ComputeIntrinsicSize(const Box& box) {
  Visit(const_cast<Box*>(&box));
  return result_;
}

gfx::FloatSize IntrinsicSizeVisitor::ComputePreferredSize(const Box& box) {
  return SizeCalculator().ComputePreferredSize(box);
}

void IntrinsicSizeVisitor::ReturnSize(const gfx::FloatSize& size) {
  result_ = size;
}

gfx::FloatSize IntrinsicSizeVisitor::SizeOfHorizontalFlowBox(
    const FlowBox& flow_box) {
  TRACE_EVENT0("visuals", "IntrinsicSizeVisitor::SizeOfHorizontalFlowBox");
  DCHECK(flow_box.first_child()) << flow_box;
  auto size = gfx::FloatSize();
  for (const auto& child : flow_box.child_boxes()) {
    if (!child->position().is_static())
      continue;
    const auto& child_border = child->border();
    const auto& child_padding = child->padding();
    const auto& child_size = ComputePreferredSize(*child) +
                             child_border.size() + child_padding.size();
    size = gfx::FloatSize(size.width() + child_size.width(),
                          std::max(size.height(), child_size.height()));
  }
  return size;
}

gfx::FloatSize IntrinsicSizeVisitor::SizeOfVerticalFlowBox(
    const FlowBox& flow_box) {
  TRACE_EVENT0("visuals", "IntrinsicSizeVisitor::SizeOfVerticalFlowBox");
  DCHECK(flow_box.first_child()) << flow_box;
  auto size = gfx::FloatSize();
  for (const auto& child : flow_box.child_boxes()) {
    if (!child->position().is_static())
      continue;
    const auto& child_border = child->border();
    const auto& child_padding = child->padding();
    const auto& child_size = ComputePreferredSize(*child) +
                             child_border.size() + child_padding.size();
    size = gfx::FloatSize(std::max(size.width(), child_size.width()),
                          size.height() + child_size.height());
  }
  return size;
}

// BoxVisitor
void IntrinsicSizeVisitor::VisitFlowBox(FlowBox* flow_box) {
  const auto first_child = flow_box->first_child();
  if (!first_child)
    return ReturnSize(gfx::FloatSize());
  if (IsDisplayOutsideInline(first_child->display()))
    return ReturnSize(SizeOfHorizontalFlowBox(*flow_box));
  return ReturnSize(SizeOfVerticalFlowBox(*flow_box));
}

void IntrinsicSizeVisitor::VisitImageBox(ImageBox* box) {
  ReturnSize(box->bitmap().size());
}

void IntrinsicSizeVisitor::VisitRootBox(RootBox* box) {
  NOTREACHED() << "NYI IntrinsicSizeVisitor for RootBox";
}

void IntrinsicSizeVisitor::VisitShapeBox(ShapeBox* box) {
  ReturnSize(gfx::FloatSize(box->size().as_length().value(),
                            box->size().as_length().value()));
}

void IntrinsicSizeVisitor::VisitTextBox(TextBox* box) {
  if (box->data().empty())
    return ReturnSize(gfx::FloatSize());
  const auto& cached_size = box->preferred_size();
  if (!cached_size.IsEmpty())
    return ReturnSize(cached_size);
  TRACE_EVENT0("visuals", "IntrinsicSizeVisitor::VisitTextBox");
  const auto& text_format = BoxEditor().EnsureTextFormat(box);
  const auto& size = text_format.ComputeMetrics(box->data());
  BoxEditor().SetPreferredSize(box, size);
  ReturnSize(size);
}

//////////////////////////////////////////////////////////////////////
//
// PreferredSizeVisitor
//
class PreferredSizeVisitor final : public BoxVisitor {
 public:
  PreferredSizeVisitor() = default;
  ~PreferredSizeVisitor() final = default;

  gfx::FloatSize ComputePreferredSize(const Box& box);

 private:
  void ReturnSize(const gfx::FloatSize& size);

#define V(name) void Visit##name(name* box) final;
  FOR_EACH_VISUAL_BOX(V)
#undef V

  gfx::FloatSize result_;

  DISALLOW_COPY_AND_ASSIGN(PreferredSizeVisitor);
};

gfx::FloatSize PreferredSizeVisitor::ComputePreferredSize(const Box& box) {
  Visit(const_cast<Box*>(&box));
  return result_;
}

void PreferredSizeVisitor::ReturnSize(const gfx::FloatSize& size) {
  result_ = size;
}

// BoxVisitor
void PreferredSizeVisitor::VisitFlowBox(FlowBox* box) {
  ReturnSize(SizeCalculator().ComputeExtrinsicSize(*box));
}

void PreferredSizeVisitor::VisitImageBox(ImageBox* box) {
  ReturnSize(SizeCalculator().ComputeExtrinsicSize(*box));
}

void PreferredSizeVisitor::VisitRootBox(RootBox* box) {
  ReturnSize(box->viewport_size());
}

void PreferredSizeVisitor::VisitShapeBox(ShapeBox* box) {
  ReturnSize(SizeCalculator().ComputeExtrinsicSize(*box));
}

void PreferredSizeVisitor::VisitTextBox(TextBox* box) {
  ReturnSize(SizeCalculator().ComputeExtrinsicSize(*box));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// SizeCalculator
//
SizeCalculator::SizeCalculator() {}
SizeCalculator::~SizeCalculator() {}

gfx::FloatSize SizeCalculator::ComputeExtrinsicSize(const Box& box) const {
  return ExtrinsicSizeVisitor().ComputeExtrinsicSize(box);
}

gfx::FloatSize SizeCalculator::ComputeIntrinsicSize(const Box& box) const {
  return IntrinsicSizeVisitor().ComputeIntrinsicSize(box);
}

gfx::FloatSize SizeCalculator::ComputePreferredSize(const Box& box) const {
  return PreferredSizeVisitor().ComputePreferredSize(box);
}

}  // namespace visuals
