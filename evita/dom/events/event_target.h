// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_event_target_h)
#define INCLUDE_evita_dom_events_event_target_h

#include <memory>
#include <vector>

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/dom/events/event_target_id.h"
#include "evita/dom/time_stamp.h"
#include "evita/v8_glue/function_template.h"
#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

class Event;
using v8_glue::Optional;

class EventTarget : public v8_glue::Scriptable<EventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(EventTarget);

  class EventTargetIdMapper;
  friend class EventTargetIdMapper;

  public: typedef std::vector<EventTarget*> EventPath;

  private: class EventListenerMap;

  private: int event_target_id_;
  private: std::unique_ptr<EventListenerMap> event_listener_map_;

  protected: EventTarget();
  protected: virtual ~EventTarget();

  public: EventTargetId event_target_id() const { return event_target_id_; }
  public: EventTargetId id() const { return event_target_id(); }

  public: void AddEventListener(const base::string16& type,
                                v8::Handle<v8::Object> callback,
                                Optional<bool> capture);
  public: virtual EventPath BuildEventPath() const;
  public: virtual bool DispatchEvent(Event* event);
  public: static EventTarget* FromEventTargetId(EventTargetId event_target_id);
  private: void InvokeEventListeners(Event* event);
  public: static void ResetForTesting();
  public: void RemoveEventListener(const base::string16& type,
                                   v8::Handle<v8::Object> callback,
                                   Optional<bool> capture);
  DISALLOW_COPY_AND_ASSIGN(EventTarget);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_event_target_h)
