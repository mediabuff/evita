// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/screen_text_block.h"

#include "base/logging.h"
#include "evita/views/text/render_cell.h"
#include "evita/views/text/render_selection.h"
#include "evita/views/text/render_text_block.h"
#include "evita/views/text/render_text_line.h"

#define DEBUG_DRAW 0

namespace views {
namespace rendering {

namespace {

std::unique_ptr<gfx::Bitmap> CopyFromRenderTarget(const gfx::Graphics* gfx,
                                                  const gfx::RectF rect) {
  auto bitmap = std::make_unique<gfx::Bitmap>(*gfx);
  auto const rect_u = gfx::RectU(static_cast<uint32_t>(rect.left),
                                 static_cast<uint32_t>(rect.top),
                                 static_cast<uint32_t>(rect.right),
                                 static_cast<uint32_t>(rect.bottom));
  DCHECK_EQ(static_cast<float>(rect_u.width()), rect.width());
  DCHECK_EQ(static_cast<float>(rect_u.height()), rect.height());
  #if DEBUG_DRAW
    DVLOG(0) << "CopyFromRenderTarget " << rect;
  #endif
  auto const hr = (*bitmap)->CopyFromRenderTarget(nullptr, *gfx, &rect_u);
  if (FAILED(hr)) {
     return std::unique_ptr<gfx::Bitmap>();
  }
  return std::move(bitmap);
}

inline void FillRect(const gfx::Graphics& gfx, const gfx::RectF& rect,
                     gfx::ColorF color) {
  gfx::Brush fill_brush(gfx, color);
  gfx.FillRectangle(fill_brush, rect);
}

void AddRect(std::vector<gfx::RectF>& rects, const gfx::RectF& rect) {
  if (rects.empty() || rects.back().bottom != rect.top) {
    rects.push_back(rect);
  } else {
    auto& last = rects.back();
    last.right = std::max(last.right, rect.right);
    last.bottom = rect.bottom;
  }
}

} // namespace

typedef std::list<TextLine*>::const_iterator FormatLineIterator;

//////////////////////////////////////////////////////////////////////
//
// ScreenTextBlock::RenderContext
//
class ScreenTextBlock::RenderContext {
  private: const gfx::ColorF bgcolor_;
  private: mutable std::vector<gfx::RectF> copy_rects_;
  private: mutable std::vector<gfx::RectF> dirty_rects_;
  private: const std::list<TextLine*>& format_lines_;
  private: const gfx::Graphics* gfx_;
  private: const gfx::RectF rect_;
  private: const std::vector<TextLine*>& screen_lines_;
  private: const ScreenTextBlock* screen_text_block_;
  private: mutable std::vector<gfx::RectF> skip_rects_;

  public: RenderContext(const ScreenTextBlock* screen_text_block,
                        const TextBlock* format_text_block,
                        gfx::ColorF bgcolor);
  public: ~RenderContext() = default;

  private: void Copy(float dst_top, float dst_bottom, float src_top) const;
  private: void DrawDirtyRect(const gfx::RectF& rect, float red, float green,
                              float blue) const;
  public: void FillBottom(const TextLine* line) const;
  private: void FillRight(const TextLine* line) const;
  private: FormatLineIterator FindFirstMismatch() const;
  private: FormatLineIterator FindLastMismatch() const;
  private: std::vector<TextLine*>::const_iterator FindCopyable(
      TextLine* line) const;
  public: void Finish();
  public: bool Render();
  private: void RestoreDirtyRect(const gfx::RectF& rect) const;
  public: FormatLineIterator TryCopy(
      const FormatLineIterator& format_line_start,
      const FormatLineIterator& format_line_end) const;

  DISALLOW_COPY_AND_ASSIGN(RenderContext);
};

ScreenTextBlock::RenderContext::RenderContext(
    const ScreenTextBlock* screen_text_block,
    const TextBlock* format_text_block, gfx::ColorF bgcolor)
    : bgcolor_(bgcolor),  format_lines_(format_text_block->lines()),
      gfx_(screen_text_block->gfx_), rect_(screen_text_block->rect_),
      screen_text_block_(screen_text_block),
      screen_lines_(screen_text_block->lines_) {
}

void ScreenTextBlock::RenderContext::Copy(float dst_top, float dst_bottom,
                                          float src_top) const {
  auto const src_bottom = src_top + dst_bottom - dst_top;

  auto const height = std::min(
      std::min(rect_.bottom, dst_bottom) - dst_top,
      std::min(rect_.bottom, src_bottom) - src_top);

  gfx::RectF dst_rect(rect_.left, dst_top, rect_.right, dst_top + height);
  gfx::RectF src_rect(0.0f, src_top - screen_lines_.front()->top(),
                      rect_.width(),
                      src_top + height - screen_lines_.front()->top());
  DCHECK_EQ(dst_rect.size(), src_rect.size());
  #if DEBUG_DRAW
    DVLOG(0) << "Copy to " << dst_rect << " from " << src_rect;
  #endif
  (*gfx_)->DrawBitmap(*screen_text_block_->bitmap_, dst_rect, 1.0f,
                      D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                      src_rect);
}

void ScreenTextBlock::RenderContext::DrawDirtyRect(
    const gfx::RectF& rect, float red, float green, float blue) const {
  RestoreDirtyRect(rect);
  gfx_->FillRectangle(gfx::Brush(*gfx_, red, green, blue, 0.1f), rect);
  gfx_->DrawRectangle(gfx::Brush(*gfx_, red, green, blue, 0.2f), rect, 0.5f);
}

void ScreenTextBlock::RenderContext::FillBottom(const TextLine* line) const {
  gfx::RectF rect(rect_);;
  rect.top = line->bottom();
  if (rect.empty())
    return;
  FillRect(*gfx_, rect, bgcolor_);
}

void ScreenTextBlock::RenderContext::FillRight(const TextLine* line) const {
  gfx::RectF rect(line->rect());
  rect.left  = rect_.left + line->GetWidth();
  rect.right = rect_.right;
  if (rect.empty())
    return;
  FillRect(*gfx_, rect, bgcolor_);
}

FormatLineIterator ScreenTextBlock::RenderContext::FindFirstMismatch() const {
  auto screen_line_runner = screen_lines_.cbegin();
  for (auto format_line_runner = format_lines_.cbegin();
       format_line_runner != format_lines_.cend();
       ++format_line_runner) {
    if (screen_line_runner == screen_lines_.cend())
      return format_line_runner;
    auto const format_line = (*format_line_runner);
    auto const screen_line = (*screen_line_runner);
    if (format_line->rect() != screen_line->rect() ||
        !format_line->Equal(screen_line)) {
      return format_line_runner;
    }
    AddRect(skip_rects_, format_line->rect());
  }
  return format_lines_.cend();
}

FormatLineIterator ScreenTextBlock::RenderContext::FindLastMismatch() const {
#if 0
  auto screen_line_runner = screen_lines_.crbegin();
  auto format_last_match = format_lines_.crbegin();
  for (auto format_line_runner = format_lines_.crbegin();
       format_line_runner != format_lines_.crend();
       ++format_line_runner) {
    auto const format_line = (*format_line_runner);
    if (format_line->top() >= rect_.bottom)
      continue;
    if (screen_line_runner == screen_lines_.crend())
      break;
    auto const screen_line = (*screen_line_runner);
    if (format_line->rect() != screen_line->rect() ||
        !format_line->Equal(screen_line)) {
      break;
    }
    AddRect(skip_rects_, format_line->rect());
    format_last_match = format_line_runner;
    ++screen_line_runner;
  }

  return format_last_match == format_lines_.crbegin() ? format_lines.end() :
      std::find(format_lines_.cbegin(), format_lines_.cend(),
                *format_last_match);
#else
  return format_lines_.end();
#endif
}

std::vector<TextLine*>::const_iterator
    ScreenTextBlock::RenderContext::FindCopyable(TextLine* format_line) const {
  for (auto runner = screen_lines_.begin(); runner != screen_lines_.end();
       ++runner) {
    if ((*runner)->Equal(format_line) &&
        (*runner)->rect().bottom <= rect_.bottom) {
      return runner;
    }
  }
  return screen_lines_.end();
}

void ScreenTextBlock::RenderContext::Finish() {
  // Draw dirty rectangles for debugging.
  gfx::Graphics::AxisAlignedClipScope clip_scope(*gfx_, rect_);
  for (auto rect : copy_rects_) {
    DrawDirtyRect(rect, 0.0f, 0.0f, 1.0f);
  }
  for (auto rect : dirty_rects_) {
    DrawDirtyRect(rect, 1.0f, 0.0f, 1.0f);
  }
}

bool ScreenTextBlock::RenderContext::Render() {
  auto dirty = false;

  auto const format_line_start = FindFirstMismatch();
  if (format_line_start != format_lines_.end()) {
    auto const format_line_end = FindLastMismatch();
    gfx::Graphics::AxisAlignedClipScope clip_scope(*gfx_, rect_);
    for (auto format_line_runner = format_line_start;
         format_line_runner != format_line_end;
         ++format_line_runner) {
      format_line_runner = TryCopy(format_line_runner, format_line_end);
      if (format_line_runner == format_line_end)
        break;
      auto const format_line = *format_line_runner;
      format_line->Render(*gfx_);
      FillRight(format_line);
      AddRect(dirty_rects_, format_line->rect());
      dirty = true;
    }
  }

  // Erase dirty rectangle markers.
  for (auto rect : skip_rects_) {
    RestoreDirtyRect(rect);
  }
  return dirty;
}

void ScreenTextBlock::RenderContext::RestoreDirtyRect(
    const gfx::RectF& rect) const {
  if (!screen_text_block_->bitmap_ || !*screen_text_block_->bitmap_)
    return;
  gfx_->DrawBitmap(*screen_text_block_->bitmap_,
                   gfx::RectF(rect_.left, rect.top, rect_.right, rect.bottom),
                   gfx::RectF(0.0f, rect.top - rect_.top, rect_.width(),
                              rect.bottom - rect_.top));
}

FormatLineIterator ScreenTextBlock::RenderContext::TryCopy(
    const FormatLineIterator& format_current,
    const FormatLineIterator& format_end) const {
  if (!screen_text_block_->bitmap_)
    return format_current;

  auto const screen_end = screen_lines_.end();
  auto format_runner = format_current;
  while (format_runner != format_end) {
    // TODO(yosi) Should we search longest match? How?
    auto const format_start = format_runner;
    auto const screen_start = FindCopyable(*format_start);
    if (screen_start == screen_lines_.end())
      return format_runner;

    auto const dst_top = (*format_start)->top();
    auto const src_top = (*screen_start)->top();

    auto const skip = dst_top == src_top;
    if (skip)
      AddRect(skip_rects_, (*format_start)->rect());
    else
      AddRect(copy_rects_, (*format_start)->rect());

    auto dst_bottom = (*format_start)->bottom();
    ++format_runner;
    auto screen_runner = screen_start;
    ++screen_runner;

    while (format_runner != format_end && screen_runner != screen_end) {
      auto const format_line = *format_runner;
      if (!format_line->Equal(*screen_runner))
        break;
      if (skip)
        AddRect(skip_rects_, format_line->rect());
      else
        AddRect(copy_rects_, format_line->rect());
      dst_bottom = format_line->bottom();
      ++format_runner;
      ++screen_runner;
    }

    if (!skip)
      Copy(dst_top, dst_bottom, src_top);
  }
  return format_runner;
}

//////////////////////////////////////////////////////////////////////
//
// ScreenTextBlock
//
ScreenTextBlock::ScreenTextBlock()
    : dirty_(true), gfx_(nullptr) {
}

ScreenTextBlock::~ScreenTextBlock() {
}

void ScreenTextBlock::Render(const TextBlock* text_block, gfx::ColorF bgcolor) {
  RenderContext render_context(this, text_block, bgcolor);
  dirty_ = render_context.Render();
  if (!dirty_)
    return;

  Reset();
  // TODO(yosi) We should use existing TextLine's in ScreenTextBlock.
  for (auto line : text_block->lines()) {
    lines_.push_back(line->Copy());
  }
  render_context.FillBottom(lines_.back());

  bitmap_ = CopyFromRenderTarget(gfx_, gfx::RectF(
      gfx::PointF(rect_.left, lines_.front()->rect().top),
      gfx::SizeF(rect_.width(), lines_.back()->rect().bottom)));

  render_context.Finish();
  dirty_ = false;
}

void ScreenTextBlock::Reset() {
  dirty_ = true;
  for (auto line : lines_) {
    delete line;
  }
  lines_.clear();
  bitmap_.reset();
}

void ScreenTextBlock::SetGraphics(const gfx::Graphics* gfx) {
  Reset();
  gfx_ = gfx;
}

void ScreenTextBlock::SetRect(const gfx::RectF& rect) {
  Reset();
  rect_ = rect;
}

// gfx::Graphics::Observer
void ScreenTextBlock::ShouldDiscardResources() {
  Reset();
}

} // namespace rendering
} // namespace views
