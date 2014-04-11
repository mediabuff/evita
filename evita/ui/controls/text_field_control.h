// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_controls_text_field_control_h)
#define INCLUDE_evita_ui_controls_text_field_control_h

#include "evita/ui/controls/control.h"

#include <memory>

#include "base/strings/string16.h"
#include "evita/gfx_base.h"
#include "evita/ui/caret.h"

namespace ui {

class TextFieldControl final : public Control, public Caret::Owner {
  private: class Renderer;
  public: struct Selection {
    size_t anchor_offset;
    size_t focus_offset;

    Selection();
    ~Selection();

    bool operator==(const Selection& other) const;
    bool operator!=(const Selection& other) const;

    bool collapsed() const;
    size_t end() const;
    size_t start() const;
  };

  private: const std::unique_ptr<Renderer> renderer_;

  public: TextFieldControl(ControlController* controller,
                           const Selection& selection,
                           const base::string16& text, const Style& style);
  public: virtual ~TextFieldControl();

  public: void set_style(const Style& style);
  public: void set_selection(const Selection& new_selection);
  public: void set_text(const base::string16& text);

  public: int MapPointToOffset(const gfx::PointF& point) const;

  // ui::Caret::Owner
  private: virtual void UpdateCaret(gfx::Graphics* gfx) override;

  // ui::Widget
  private: virtual void DidKillFocus(ui::Widget* focused_window) override;
  private: virtual void DidResize() override;
  private: virtual void DidSetFocus(ui::Widget* last_focused) override;
  private: virtual void OnDraw(gfx::Graphics* gfx) override;

  DISALLOW_COPY_AND_ASSIGN(TextFieldControl);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_controls_text_field_control_h)
