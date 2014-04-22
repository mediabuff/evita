// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_document_event_init_h)
#define INCLUDE_evita_dom_events_document_event_init_h

#include "evita/bindings/EventInit.h"

#include "evita/gc/member.h"

namespace dom {

class Window;

class DocumentEventInit : public EventInit {
  private: gc::Member<Window> view_;

  public: explicit DocumentEventInit(Window* view);
  public: DocumentEventInit();
  public: virtual ~DocumentEventInit();

  public: Window* view() const { return view_.get(); }
  public: void set_view(Window* view) { view_ = view; }

  // dom::Dictionary
  private: virtual HandleResult HandleKeyValue(
      v8::Handle<v8::Value> key, v8::Handle<v8::Value> value) override;
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_document_event_init_h)
