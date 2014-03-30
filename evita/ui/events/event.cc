// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ui/events/event.h"

#include <algorithm>
#include <vector>

#include "base/logging.h"
#include "common/memory/singleton.h"
#include "common/win/win32_verify.h"
#include "evita/ui/widget.h"

namespace ui {

namespace {

class KeyCodeMapper : public common::Singleton<KeyCodeMapper> {
  DECLARE_SINGLETON_CLASS(KeyCodeMapper);

  private: std::vector<int> graph_keys_;

  private: KeyCodeMapper() : graph_keys_(256) {
    for (auto key_code = 0u; key_code < graph_keys_.size(); ++key_code) {
      auto const char_code = ::MapVirtualKey(key_code, MAPVK_VK_TO_CHAR);
      if (char_code >= 0x20)
        graph_keys_[key_code] = static_cast<int>(char_code);
    }
  }

  public: ~KeyCodeMapper() = default;

  public: int Map(int virtual_key_code);
};

int KeyCodeMapper::Map(int virtual_key_code) {
  if (VK_CONTROL == virtual_key_code)
    return 0;
  if (VK_SHIFT == virtual_key_code)
    return 0;

  auto key_code = graph_keys_[static_cast<size_t>(virtual_key_code)];

  if (!key_code) {
   if (virtual_key_code == VK_CANCEL)
     key_code = VK_PAUSE | 0x100;
   else
     key_code = virtual_key_code | 0x100;
  }

  if (::GetKeyState(VK_CONTROL) < 0) {
    key_code |= static_cast<int>(Modifier::Control);
  } else if (key_code <= 0xFF) {
    // For graphics key, we use WM_CHAR.
    return 0;
  }

  if (::GetKeyState(VK_SHIFT) < 0)
    key_code |= static_cast<int>(Modifier::Shift);
  return key_code;
}

//////////////////////////////////////////////////////////////////////
//
// MouseClickTracker
//
class MouseClickTracker : public common::Singleton<MouseClickTracker> {
  DECLARE_SINGLETON_CLASS(MouseClickTracker);

  // These values match the Windows defaults.
  private: static const auto kDoubleClickTimeMS = 500;
  private: static const auto kDoubleClickWidth = 4;
  private: static const auto kDoubleClickHeight = 4;

  private: enum class State {
    Start,
    Pressed,
    PressedReleased,
    PressedReleasedPressed,
  };

  private: int click_count_;
  private: MouseEvent last_event_;
  private: State state_;

  private: MouseClickTracker();
  public: ~MouseClickTracker();

  public: int click_count() const { return click_count_; }
  private: bool is_repeated_event(const MouseEvent& event) const;
  private: bool is_same_location(const MouseEvent& event) const;
  private: bool is_same_time(const MouseEvent& event) const;

  public: void UpdateState(const MouseEvent& event);

  DISALLOW_COPY_AND_ASSIGN(MouseClickTracker);
};

MouseClickTracker::MouseClickTracker()
    : click_count_(0), state_(State::Start) {
}

MouseClickTracker::~MouseClickTracker() {
}

bool MouseClickTracker::is_repeated_event(const MouseEvent& event) const {
  return is_same_location(event) && is_same_time(event);
}

bool MouseClickTracker::is_same_location(const MouseEvent& event) const {
  if (last_event_.target() != event.target())
    return false;
  Rect rect(last_event_.screen_location().x - kDoubleClickWidth / 2,
            last_event_.screen_location().y - kDoubleClickHeight / 2,
            last_event_.screen_location().x + kDoubleClickWidth / 2,
            last_event_.screen_location().y + kDoubleClickHeight / 2);
  return rect.Contains(event.screen_location());
}

bool MouseClickTracker::is_same_time(const MouseEvent& event) const {
  auto const diff = event.time_stamp() - last_event_.time_stamp();
  return diff.InMilliseconds() <= kDoubleClickTimeMS;
}

void MouseClickTracker::UpdateState(const MouseEvent& event) {
  if (event.event_type() != EventType::MousePressed &&
      event.event_type() != EventType::MouseReleased) {
    return;
  }
  click_count_ = 0;
  auto const state = state_;
  state_ = State::Start;
  switch (state) {
    case State::Start:
      if (event.event_type() == EventType::MousePressed) {
        last_event_ = event;
        state_ = State::Pressed;
      }
      break;
    case State::Pressed:
      if (last_event_.target() != event.target())
        return;
      if (event.event_type() == EventType::MouseReleased) {
        click_count_ = 1;
        if (is_repeated_event(event))
          state_ = State::PressedReleased;
      }
      break;
    case State::PressedReleased:
      if (event.event_type() == EventType::MousePressed) {
        if (is_repeated_event(event)) {
          state_ = State::PressedReleasedPressed;
          break;
        }
        last_event_ = event;
        state_ = State::Pressed;
      }
      break;
    case State::PressedReleasedPressed:
      if (last_event_.target() != event.target())
        return;
      if (event.event_type() == EventType::MouseReleased &&
          is_repeated_event(event)) {
        click_count_ = 2;
      }
      break;
  }
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Event
//
Event::Event(EventType event_type)
    : event_type_(event_type), time_stamp_(base::Time::Now()) {
}

Event::Event() : Event(EventType::Invalid) {
}

Event::~Event() {
}

//////////////////////////////////////////////////////////////////////
//
// KeyboardEvent
//
KeyboardEvent::KeyboardEvent(EventType event_type, LPARAM lParam)
    : Event(event_type), raw_key_code_(0),
      repeat_(HIWORD(lParam) & KF_REPEAT) {
}

KeyboardEvent::KeyboardEvent()
    : KeyboardEvent(EventType::Invalid, 0) {
}

KeyboardEvent::~KeyboardEvent() {
}

KeyboardEvent KeyboardEvent::Create(uint32_t message, WPARAM wParam,
                                    LPARAM lParam) {
  if (message == WM_CHAR) {
    auto event = KeyboardEvent(EventType::KeyPressed, lParam);
    event.raw_key_code_ = static_cast<int>(wParam);
    return event.raw_key_code_ < 0x20 ? KeyboardEvent() : event;
  }

  if (message == WM_KEYDOWN) {
    auto event = KeyboardEvent(EventType::KeyPressed, lParam);
    event.raw_key_code_ = KeyCodeMapper::instance()->
        Map(static_cast<int>(wParam));
    return event.raw_key_code_ ? event : KeyboardEvent();
  }

  return KeyboardEvent();
}

//////////////////////////////////////////////////////////////////////
//
// MouseEvent
//
MouseEvent::MouseEvent(EventType event_type, const Point& screen_point,
                       const Point& client_point)
    : Event(event_type),
      alt_key_(false),
      button_(0),
      buttons_(0),
      click_count_(0),
      client_point_(client_point),
      control_key_(false),
      screen_point_(screen_point),
      shift_key_(false),
      target_(nullptr) {
}

MouseEvent::MouseEvent(EventType event_type, Button button,
                       uint32_t flags, const Point& point)
    : Event(event_type),
      alt_key_(false),
      button_(button),
      buttons_(ConvertToButtons(flags)),
      click_count_(0),
      client_point_(point),
      control_key_(flags & MK_CONTROL),
      screen_point_(point),
      shift_key_(flags & MK_SHIFT),
      target_(nullptr) {
}

MouseEvent::MouseEvent(EventType event_type, Button button, Widget* widget,
                       WPARAM wParam, LPARAM lParam)
    : MouseEvent(event_type, button, GET_KEYSTATE_WPARAM(wParam),
                 MAKEPOINTS(lParam)) {
  WIN32_VERIFY(::MapWindowPoints(widget->AssociatedHwnd(), HWND_DESKTOP,
                                 &screen_point_, 1));
  target_ = widget;
  MouseClickTracker::instance()->UpdateState(*this);
  if (event_type == EventType::MouseReleased)
    click_count_ = MouseClickTracker::instance()->click_count();
}

MouseEvent::MouseEvent()
    : MouseEvent(EventType::Invalid, kNone, 0u, Point()) {
}

MouseEvent::~MouseEvent() {
}

int MouseEvent::ConvertToButtons(uint32_t flags) {
  int buttons = 0;
  if (flags & MK_LBUTTON)
    buttons |= 1 << MouseEvent::kLeft;
  if (flags & MK_MBUTTON)
    buttons |= 1 << MouseEvent::kMiddle;
  if (flags & MK_RBUTTON)
    buttons |= 1 << MouseEvent::kRight;
  if (flags & MK_XBUTTON1)
    buttons |= 1 << MouseEvent::kOther1;
  if (flags & MK_XBUTTON2)
    buttons |= 1 << MouseEvent::kOther2;
  return buttons;
}

// Note: Windows sends message in below sequence for double click:
//  1 WM_LBUTTONDOWN
//  2 WM_LBUTTONUP
//  3 WM_LBUTTONDBLCLK
//  4 WM_LBUTTONUP
MouseEvent MouseEvent::Create(Widget* widget, uint32_t message, WPARAM wParam,
                              LPARAM lParam) {
  switch (message) {
    // Left button
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
        return MouseEvent(EventType::MousePressed, kLeft, widget, wParam,
                          lParam);
    case WM_LBUTTONUP:
        return MouseEvent(EventType::MouseReleased, kLeft, widget, wParam,
                          lParam);

    // Middle button
    case WM_MBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
        return MouseEvent(EventType::MousePressed, kMiddle, widget, wParam,
                          lParam);
    case WM_MBUTTONUP:
        return MouseEvent(EventType::MouseReleased, kMiddle, widget, wParam,
                          lParam);

    // Move
    case WM_MOUSEMOVE:
      return MouseEvent(EventType::MouseMoved, kNone, widget, wParam,
                        lParam);

    // Right button
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
        return MouseEvent(EventType::MousePressed, kRight, widget, wParam,
                          lParam);
    case WM_RBUTTONUP:
        return MouseEvent(EventType::MouseReleased, kRight, widget, wParam,
                          lParam);

    // X button
    case WM_XBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
      if (HIWORD(wParam) == XBUTTON1) {
        return MouseEvent(EventType::MousePressed, kOther1, widget,
                          wParam, lParam);
      }
      if (HIWORD(wParam) == XBUTTON2) {
        return MouseEvent(EventType::MousePressed, kOther2, widget,
                          wParam, lParam);
      }
      break;
    case WM_XBUTTONUP:
      if (HIWORD(wParam) == XBUTTON1) {
        return MouseEvent(EventType::MouseReleased, kOther1, widget,
                          wParam, lParam);
      }
      if (HIWORD(wParam) == XBUTTON2) {
        return MouseEvent(EventType::MouseReleased, kOther2, widget,
                          wParam, lParam);
      }
      break;
  }
  return MouseEvent();
}

//////////////////////////////////////////////////////////////////////
//
// MouseWheelEvent
//
MouseWheelEvent::MouseWheelEvent(Widget* widget, WPARAM wParam, LPARAM lParam)
    : MouseEvent(EventType::MouseWheel, kNone, GET_KEYSTATE_WPARAM(wParam),
                 MAKEPOINTS(lParam)),
      delta_(GET_WHEEL_DELTA_WPARAM(wParam)) {
  WIN32_VERIFY(::MapWindowPoints(HWND_DESKTOP, widget->AssociatedHwnd(),
                                 &client_point_, 1));
}

MouseWheelEvent::~MouseWheelEvent() {
}

}  // namespace ui
