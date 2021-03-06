// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text_window.h"

#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/trace_event/trace_event.h"
#include "evita/dom/public/float_point.h"
#include "evita/dom/public/float_rect.h"
#include "evita/dom/public/float_size.h"
#include "evita/dom/public/text_area_display_item.h"
#include "evita/dom/public/text_composition_data.h"
#include "evita/dom/public/view_events.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/color_f.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/gfx/rect_f.h"
#include "evita/metrics/time_scope.h"
#include "evita/text/models/buffer.h"
#include "evita/text/models/selection.h"
#include "evita/text/paint/public/caret.h"
#include "evita/text/paint/public/selection.h"
#include "evita/text/paint/public/view.h"
#include "evita/text/paint/view_paint_cache.h"
#include "evita/text/paint/view_painter.h"
#include "evita/ui/base/ime/text_composition.h"
#include "evita/ui/base/ime/text_input_client.h"
#include "evita/ui/compositor/compositor.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/events/event.h"
#include "evita/views/metrics_view.h"
#include "evita/visuals/display/display_item_list_processor.h"
#include "evita/visuals/display/public/display_item_list.h"

namespace views {

namespace {

gfx::PointF ToPointF(const gfx::FloatPoint& point) {
  return gfx::PointF(point.x(), point.y());
}

gfx::SizeF ToSizeF(const gfx::FloatSize& size) {
  return gfx::SizeF(size.width(), size.height());
}

gfx::RectF ToRectF(const gfx::FloatRect& rect) {
  return gfx::RectF(ToPointF(rect.origin()), ToSizeF(rect.size()));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextWindow::DragController
//
class TextWindow::DragController final {
 public:
  DragController(const DragController& other) = delete;
  explicit DragController(TextWindow* owner);
  ~DragController() = default;
  DragController& operator=(const DragController& other) = delete;

  // Returns true if |event| is processed.
  bool OnMouseMoved(const ui::MouseEvent& event);
  // Returns true if |event| is processed.
  bool OnMousePressed(const ui::MouseEvent& event);
  // Returns true if |event| is processed.
  bool OnMouseReleased(const ui::MouseEvent& event);

 private:
  enum class State { None, Dragging };

  MetricsView& metrics_view() { return *owner_.metrics_view_; }

  gfx::Size anchor_;
  TextWindow& owner_;
  State state_ = State::None;
};

TextWindow::DragController::DragController(TextWindow* owner)
    : owner_(*owner) {}

bool TextWindow::DragController::OnMouseMoved(const ui::MouseEvent& event) {
  if (state_ != State::Dragging)
    return false;
  metrics_view().SetBounds(
      gfx::Rect(event.location() - anchor_, metrics_view().bounds().size()));
  return true;
}

bool TextWindow::DragController::OnMousePressed(const ui::MouseEvent& event) {
  if (!event.is_left_button() || event.click_count() != 0)
    return false;
  if (!metrics_view().bounds().Contains(event.location()))
    return false;
  DCHECK_EQ(state_, State::None);
  state_ = State::Dragging;
  anchor_ = event.location() - metrics_view().origin();
  owner_.SetCursor(::LoadCursor(nullptr, IDC_SIZEALL));
  owner_.SetCapture();
  return true;
}

bool TextWindow::DragController::OnMouseReleased(const ui::MouseEvent& event) {
  if (state_ != State::Dragging)
    return false;
  owner_.ReleaseCapture();
  state_ = State::None;
  owner_.SetCursor(nullptr);
  return true;
}

//////////////////////////////////////////////////////////////////////
//
// TextWindow
//
TextWindow::TextWindow(WindowId window_id)
    : CanvasContentWindow(window_id),
      drag_controller_(new DragController(this)),
      metrics_view_(new MetricsView()) {
  AppendChild(metrics_view_);
}

TextWindow::~TextWindow() {}

void TextWindow::Paint(std::unique_ptr<TextAreaDisplayItem> display_item) {
  if (!visible() || !canvas()->UpdateReadyState())
    return;
  TRACE_EVENT_WITH_FLOW0("view", "TextWindow::Paint", window_id(),
                         TRACE_EVENT_FLAG_FLOW_IN);
  metrics_view_->RecordTime();
  MetricsView::TimingScope timing_scope(metrics_view_);
  if (!display_item->paint_view()->caret().is_none()) {
    ui::TextInputClient::Get()->set_caret_bounds(
        display_item->paint_view()->caret().bounds());
  }
  gfx::Canvas::DrawingScope drawing_scope(canvas());
  // Paint text area
  view_paint_cache_ = paint::ViewPainter(*display_item->paint_view())
                          .Paint(canvas(), std::move(view_paint_cache_));

  auto display_item_list = std::move(display_item->display_item_list());
  // Paint scroll bar
  visuals::DisplayItemListProcessor processor;
  processor.Paint(canvas(), std::move(display_item_list));

  NotifyUpdateContent();
}

void TextWindow::UpdateBounds() {
  DCHECK(!bounds().empty());
  view_paint_cache_.reset();
  auto const canvas_bounds = GetContentsBounds();
  if (canvas())
    canvas()->SetBounds(canvas_bounds);

  auto const vertical_scroll_bar_width =
      static_cast<float>(::GetSystemMetrics(SM_CXVSCROLL));

  auto const text_block_bounds = gfx::RectF(
      canvas_bounds.size() - gfx::SizeF(vertical_scroll_bar_width, 0.0f));

  scroll_bar_bounds_ =
      gfx::RectF(text_block_bounds.top_right(), canvas_bounds.bottom_right());

  // Place metrics view at bottom right of text block.
  auto const metrics_view_size = gfx::SizeF(metrics_view_->bounds().width(),
                                            metrics_view_->bounds().height());
  auto const metrics_view_bounds = gfx::RectF(
      text_block_bounds.bottom_right() - metrics_view_size - gfx::SizeF(3, 3),
      metrics_view_size);
  metrics_view_->SetBounds(gfx::ToEnclosingRect(metrics_view_bounds));
}

// gfx::CanvasObserver
void TextWindow::DidRecreateCanvas() {
  view_paint_cache_.reset();
}

// ui::LayerOwnerDelegate
void TextWindow::DidRecreateLayer(ui::Layer* old_layer) {
  CanvasContentWindow::DidRecreateLayer(old_layer);
  if (!canvas())
    return;
  old_layer->AppendLayer(metrics_view_->RecreateLayer().release());
  layer()->AppendLayer(metrics_view_->layer());
}

// ui::TextInputDelegate
void TextWindow::DidCommitComposition(const ui::TextComposition& composition) {
  DispatchTextCompositionEvent(domapi::EventType::TextCompositionCommit,
                               composition);
}

void TextWindow::DidFinishComposition() {
  DispatchTextCompositionEvent(domapi::EventType::TextCompositionEnd,
                               ui::TextComposition());
}

void TextWindow::DidStartComposition() {
  DispatchTextCompositionEvent(domapi::EventType::TextCompositionStart,
                               ui::TextComposition());
}

void TextWindow::DidUpdateComposition(const ui::TextComposition& composition) {
  DispatchTextCompositionEvent(domapi::EventType::TextCompositionUpdate,
                               composition);
}

ui::Widget* TextWindow::GetClientWindow() {
  return this;
}

// ui::Widget
void TextWindow::DidChangeBounds() {
  CanvasContentWindow::DidChangeBounds();
  UpdateBounds();
}

void TextWindow::DidHide() {
  CanvasContentWindow::DidHide();
  view_paint_cache_.reset();
}

void TextWindow::DidKillFocus(ui::Widget* will_focus_widget) {
  CanvasContentWindow::DidKillFocus(will_focus_widget);
  ui::TextInputClient::Get()->CommitComposition(this);
  ui::TextInputClient::Get()->CancelComposition(this);
  ui::TextInputClient::Get()->set_delegate(nullptr);
}

void TextWindow::DidRealize() {
  UpdateBounds();
  CanvasContentWindow::DidRealize();
  layer()->AppendLayer(metrics_view_->layer());
}

void TextWindow::DidSetFocus(ui::Widget* last_focused) {
  // Note: It is OK to set focus to hidden window.
  CanvasContentWindow::DidSetFocus(last_focused);
  ui::TextInputClient::Get()->set_delegate(this);
}

void TextWindow::DidShow() {
  CanvasContentWindow::DidShow();
  canvas()->AddObserver(this);
}

void TextWindow::OnMouseMoved(const ui::MouseEvent& event) {
  if (drag_controller_->OnMouseMoved(event))
    return;
  CanvasContentWindow::OnMouseMoved(event);
}

void TextWindow::OnMousePressed(const ui::MouseEvent& event) {
  if (drag_controller_->OnMousePressed(event))
    return;
  CanvasContentWindow::OnMousePressed(event);
}

void TextWindow::OnMouseReleased(const ui::MouseEvent& event) {
  if (drag_controller_->OnMouseReleased(event))
    return;
  CanvasContentWindow::OnMouseReleased(event);
}

}  // namespace views
