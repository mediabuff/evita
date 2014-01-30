// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/views/content_window.h"

#include "evita/editor/application.h"
#include "evita/vi_Frame.h"

namespace views {

ContentWindow::ContentWindow(
    std::unique_ptr<common::win::NativeWindow>&& native_window)
    : CommandWindow_(std::move(native_window)),
      active_tick_(0) {
}

ContentWindow::ContentWindow(views::WindowId window_id)
    : CommandWindow_(window_id),
      active_tick_(0) {
}

ContentWindow::ContentWindow()
    : ContentWindow(views::kInvalidWindowId) {
}

Frame& ContentWindow::frame() const {
  for (auto runner = static_cast<const Widget*>(this); runner;
       runner = &runner->container_widget()) {
    if (runner->is<Frame>())
      return *const_cast<Frame*>(runner->as<Frame>());
  }
  CAN_NOT_HAPPEN();
}

void ContentWindow::Activate() {
  #if DEBUG_FOCUS
    DEBUG_WIDGET_PRINTF("focus=%d show=%d\n", has_focus(), is_shown());
  #endif
  SetFocus();
}

void ContentWindow::DidKillFocus() {
  CommandWindow_::DidKillFocus();
}
  
void ContentWindow::DidSetFocus() {
  DEFINE_STATIC_LOCAL(uint, global_active_tick, (0));
  CommandWindow_::DidSetFocus();
  ++global_active_tick;
  active_tick_ = global_active_tick;
}

void ContentWindow::WillDestroyWidget() {
  active_tick_ = 0;
}

}  // namespace views
