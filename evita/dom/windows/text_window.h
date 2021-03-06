// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_WINDOWS_TEXT_WINDOW_H_
#define EVITA_DOM_WINDOWS_TEXT_WINDOW_H_

#include <memory>

#include "evita/dom/windows/window.h"

#include "evita/dom/windows/rect.h"
#include "evita/dom/windows/scroll_bar.h"
#include "evita/gc/member.h"
#include "evita/text/models/buffer_mutation_observer.h"
#include "evita/text/models/marker_set_observer.h"
#include "evita/text/models/selection_change_observer.h"
#include "evita/ui/animation/animation_frame_handler.h"
#include "evita/ui/controls/scroll_bar_observer.h"

namespace css {
class StyleSheet;
}

namespace layout {
class TextView;
}

namespace text {
class Buffer;
class MarkerSet;
}

namespace dom {
class CSSStyleSheetHandle;
class ExceptionState;
class ScrollBar;
class TextDocument;
class TextRange;
class TextSelection;

namespace bindings {
class TextWindowClass;
}

// The |TextWindow| is DOM world representative of UI world TextWidget, aka
// TextWindow.
class TextWindow final : public ginx::Scriptable<TextWindow, Window>,
                         public ScrollBarOwner,
                         public text::BufferMutationObserver,
                         public text::MarkerSetObserver,
                         public text::SelectionChangeObserver,
                         public ui::ScrollBarObserver {
  DECLARE_SCRIPTABLE_OBJECT(TextWindow);
  using FloatPoint = gfx::FloatPoint;

 public:
  class Caret;

  ~TextWindow() final;

 private:
  friend class bindings::TextWindowClass;

  TextWindow(ScriptHost* script_host,
             TextRange* selection_range,
             css::StyleSheet* style_sheet);

  text::Buffer* buffer() const;

  text::Offset ComputeEndOfLine(text::Offset offset);
  text::Offset ComputeScreenMotion(int count,
                                   const gfx::FloatPoint& point,
                                   text::Offset offset);
  text::Offset ComputeStartOfLine(text::Offset offset);
  text::Offset ComputeWindowLineMotion(int count,
                                       const gfx::FloatPoint& point,
                                       text::Offset offset);
  text::Offset ComputeWindowMotion(int count, text::Offset offset);
  void DidBeginAnimationFrame(const base::TimeTicks& time);
  bool LargeScroll(int x_count, int y_count);
  bool SmallScroll(int x_count, int y_count);
  void RequestAnimationFrame();
  void UpdateBounds();
  void UpdateScrollBar();

  // bindings
  TextWindow(ScriptHost* script_host,
             TextRange* selection_range,
             CSSStyleSheetHandle* style_sheet_handle);
  TextWindow(ScriptHost* script_host, TextRange* selection_range);
  TextDocument* document() const;
  TextSelection* selection() const { return selection_; }
  float zoom() const;
  void set_zoom(float new_zoom, ExceptionState* exception_state);

  text::Offset ComputeMotion(int method);
  text::Offset ComputeMotion(int method, text::Offset position);
  text::Offset ComputeMotion(int method, text::Offset position, int count);
  text::Offset ComputeMotion(int method,
                             text::Offset position,
                             int count,
                             const gfx::FloatPoint& point);
  text::Offset HitTestPoint(float x, float y);
  gfx::FloatRect HitTestTextPosition(text::Offset position);
  base::string16 MarkerAt(text::Offset offset,
                          ExceptionState* exception_state) const;
  void MakeSelectionVisible();
  TextWindow* NewTextWindow(TextRange* range);
  void Reconvert(const base::string16& text);
  void Scroll(int direction);
  static void SetDefaultStyleSheet(CSSStyleSheetHandle* handle);
  void SetMarker(text::Offset start,
                 text::Offset end,
                 const base::string16& marker,
                 ExceptionState* exception_state);

  // ScrollBarOwner
  void DidChangeScrollBar() final;
  void DidReleaseScrollBar() final;
  void DidPressScrollBar() final;

  // text::BufferMutationObserver
  void DidChangeStyle(const text::StaticRange& range) final;
  void DidDeleteAt(const text::StaticRange& range) final;
  void DidInsertBefore(const text::StaticRange& range) final;

  // text::MarkerSetObserver
  void DidChangeMarker(const text::StaticRange& range) final;

  // text::SelectionChangeObserver
  void DidChangeSelection() final;

  // ui::ScrollBarObserver
  void DidClickLineDown() final;
  void DidClickLineUp() final;
  void DidClickPageDown() final;
  void DidClickPageUp() final;
  void DidMoveThumb(int value) final;

  // ViewEventTarget
  bool HandleMouseEvent(const domapi::MouseEvent& api_event) final;

  // Window
  void DidChangeBounds() final;
  void DidHideWindow() final;
  void DidKillFocus() final;
  void DidSetFocus() final;
  void DidShowWindow() final;
  void ForceUpdateWindow() final;

  const std::unique_ptr<Caret> caret_;
  bool is_waiting_animation_frame_ = false;
  const std::unique_ptr<text::MarkerSet> markers_;
  const gc::Member<TextSelection> selection_;
  const std::unique_ptr<layout::TextView> text_view_;
  const std::unique_ptr<ScrollBar> vertical_scroll_bar_;

  DISALLOW_COPY_AND_ASSIGN(TextWindow);
};

}  // namespace dom

#endif  // EVITA_DOM_WINDOWS_TEXT_WINDOW_H_
