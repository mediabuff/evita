// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_EVENTS_VIEW_EVENT_HANDLER_IMPL_H_
#define EVITA_DOM_EVENTS_VIEW_EVENT_HANDLER_IMPL_H_

#include "evita/dom/public/view_event_handler.h"

namespace dom {

class Event;
class EventTarget;
class ScriptHost;

class ViewEventHandlerImpl final : public domapi::ViewEventHandler {
 public:
  explicit ViewEventHandlerImpl(ScriptHost* host);
  ~ViewEventHandlerImpl() final;

  void CallClassEventHandler(EventTarget* event_target, Event* event);

 private:
  void DispatchEventWithInLock(EventTarget* event_target, Event* event);

  // domapi::ViewEventHandler
  void DidChangeWindowBounds(WindowId window_id,
                             int left,
                             int top,
                             int right,
                             int bottom) final;
  void DidChangeWindowVisibility(WindowId window_id,
                                 domapi::Visibility visibility) final;
  void DidDestroyWidget(WindowId window_id) final;
  void DidDropWidget(WindowId source_id, WindowId target_id) final;
  void DidRealizeWidget(WindowId window_id) final;
  void DidStartViewHost() final;
  void DispatchFocusEvent(const domapi::FocusEvent& event) final;
  void DispatchKeyboardEvent(const domapi::KeyboardEvent& event) final;
  void DispatchMouseEvent(const domapi::MouseEvent& event) final;
  void DispatchTextCompositionEvent(
      const domapi::TextCompositionEvent& event) final;
  void DispatchWheelEvent(const domapi::WheelEvent& event) final;
  void OpenFile(WindowId window_id, const base::string16& file_name) final;
  void QueryClose(WindowId window_id) final;
  void RunCallback(base::Closure callback) final;
  void WillDestroyHost() final;

  ScriptHost* host_;

  DISALLOW_COPY_AND_ASSIGN(ViewEventHandlerImpl);
};

}  // namespace dom

#endif  // EVITA_DOM_EVENTS_VIEW_EVENT_HANDLER_IMPL_H_
