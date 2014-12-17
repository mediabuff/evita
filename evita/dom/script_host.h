// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_script_host_h)
#define INCLUDE_evita_dom_script_host_h

#include <string>
#include <memory>
#include <vector>

#include "base/callback_forward.h"
#include "base/location.h"
#include "base/strings/string16.h"
#include "evita/dom/public/script_host_state.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/v8_glue/isolate_holder.h"
#include "evita/v8_glue/runner_delegate.h"
#include "evita/v8_glue/scoped_persistent.h"
#include "evita/v8_glue/v8.h"

namespace base {
//template<typename T> class Callback;
class MessageLoop;
}

namespace domapi {
class IoDelegate;
}

namespace v8_glue {
class Runner;
}

namespace dom {

class Event;
class EventTarget;
class ViewDelegate;
class ViewEventHandlerImpl;

//////////////////////////////////////////////////////////////////////
//
// SuppressMessageBoxScope
//
class SuppressMessageBoxScope {
  public: SuppressMessageBoxScope();
  public: ~SuppressMessageBoxScope();
};

//////////////////////////////////////////////////////////////////////
//
// ScriptHost
//
class ScriptHost final : public v8_glue::RunnerDelegate {
  private: v8_glue::IsolateHolder isolate_holder_;
  private: std::unique_ptr<ViewEventHandlerImpl> event_handler_;
  private: domapi::IoDelegate* io_delegate_;
  // A |MessageLoop| where script runs on. We don't allow to run script other
  // than this message loop.
  private: base::MessageLoop* message_loop_for_script_;
  private: std::unique_ptr<v8_glue::Runner> runner_;
  private: domapi::ScriptHostState state_;
  private: bool testing_;
  private: v8_glue::Runner* testing_runner_;
  private: ViewDelegate* view_delegate_;

  private: ScriptHost(ViewDelegate* view_delegate,
                      domapi::IoDelegate* io_deleage);
  public: ~ScriptHost() final;

  public: ViewEventHandlerImpl* event_handler() const {
    return event_handler_.get();
  }
  public: static ScriptHost* instance();
  public: domapi::IoDelegate* io_delegate() const { return io_delegate_; }
  public: v8::Isolate* isolate() const;
  public: v8_glue::Runner* runner() const;
  public: void set_testing_runner(v8_glue::Runner* runner);
  public: ViewDelegate* view_delegate() const;

  public: void CallClassEventHandler(EventTarget* target, Event* event);
  public: void DidStartViewHost();
  public: void PlatformError(const char* name);
  public: void PostTask(const tracked_objects::Location& from_here,
                        const base::Closure& task);
  public: void ResetForTesting();
  public: void RunMicrotasks();
  public: static ScriptHost* Start(ViewDelegate* view_delegate,
                                         domapi::IoDelegate* io_delegate);
  public: static ScriptHost* StartForTesting(
      ViewDelegate* view_delegate, domapi::IoDelegate* io_delegate);
  public: void ThrowError(const std::string& message);
  public: void ThrowRangeError(const std::string& message);
  public: void ThrowException(v8::Handle<v8::Value> exception);
  public: void WillDestroyHost();

  // v8_glue::RunnerDelegate
  private: v8::Handle<v8::ObjectTemplate>
      GetGlobalTemplate(v8_glue::Runner* runner) final;
  private: void UnhandledException(v8_glue::Runner* runner,
                                   const v8::TryCatch& try_catch) final;

  DISALLOW_COPY_AND_ASSIGN(ScriptHost);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_script_host_h)
