// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_window_h)
#define INCLUDE_evita_dom_window_h

#include <unordered_set>
#include <vector>

#include "evita/dom/events/event_target.h"
#include "evita/dom/window_id.h"
#include "evita/v8_glue/nullable.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

// |Window| is correspond to |Widget| in DOM world.
class Window : public v8_glue::Scriptable<Window, EventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(Window)

  private: class WindowIdMapper;
  friend class WindowIdMapper;

  public: enum State {
    kDestroyed = -2,
    kDestroying,
    kNotRealized,
    kRealizing,
    kRealized,
  };
  static_assert(!kNotRealized, "Window::State::kNotRealized should be zero.");

  private: std::unordered_set<Window*> child_windows_;
  private: int focus_tick_;
  private: Window* parent_window_;
  private: State state_;

  protected: Window();
  public: virtual ~Window();

  public: std::vector<Window*> child_windows() const;
  public: int focus_tick() const { return focus_tick_; }
  public: WindowId id() const { return event_target_id(); }
  public: v8_glue::Nullable<Window> parent_window() const {
    return parent_window_;
  }
  public: State state() const { return state_; }
  public: WindowId window_id() const { return event_target_id(); }

  public: void AddWindow(Window* window);
  public: void ChangeParentWindow(Window* new_parent_window);
  public: void Destroy();
  public: static void DidDestroyWidget(WindowId window_id);
  public: static void DidKillFocus(WindowId window_id);
  public: static void DidRealizeWidget(WindowId window_id);
  public: static void DidSetFocus(WindowId window_id);
  public: void Focus();
  public: static Window* FromWindowId(WindowId window_id);
  public: bool IsDescendantOf(Window* other) const;
  public: void Realize();
  public: void RemoveWindow(Window* window);
  public: static void ResetForTesting();

  DISALLOW_COPY_AND_ASSIGN(Window);
};

}  // namespace dom

#include <ostream>

std::ostream& operator<<(std::ostream& ostream, const dom::Window& window);
std::ostream& operator<<(std::ostream& ostream, const dom::Window* window);

#endif //!defined(INCLUDE_evita_dom_window_h)
