// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_v8_glue_script_runner_h)
#define INCLUDE_evita_v8_glue_script_runner_h

#include <vector>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "base/memory/weak_ptr.h"
#include "evita/v8_glue/context_holder.h"

namespace v8_glue {

class RunnerDelegate;

class Runner : public gin::ContextHolder {
  public: typedef std::vector<v8::Handle<v8::Value>> Args;

  private: class CurrentRunnerScope {
    private: v8::Isolate* isolate_;

    public: CurrentRunnerScope(Runner* runner);
    public: ~CurrentRunnerScope();
  };

  public: class EscapableHandleScope {
    private: v8::Locker isolate_locker_;
    private: v8::Isolate::Scope isolate_scope_;
    private: v8::EscapableHandleScope handle_scope_;
    private: v8::Context::Scope context_scope_;
    private: CurrentRunnerScope current_runner_scope_;
    private: Runner* runner_;

    public: explicit EscapableHandleScope(Runner* runner);
    public: ~EscapableHandleScope();

    public: template<typename T> v8::Local<T> Escape(v8::Local<T> value) {
      return handle_scope_.Escape<T>(value);
    }

    DISALLOW_COPY_AND_ASSIGN(EscapableHandleScope);
  };
  friend class EscapableHandleScope;

  public: class Scope {
    private: v8::Locker isolate_locker_;
    private: v8::Isolate::Scope isolate_scope_;
    private: v8::HandleScope handle_scope_;
    private: v8::Context::Scope context_scope_;
    private: CurrentRunnerScope current_runner_scope_;
    private: Runner* runner_;

    public: explicit Scope(Runner* runner);
    public: ~Scope();

    DISALLOW_COPY_AND_ASSIGN(Scope);
  };
  friend class Scope;

  private: int call_depth_;
  private: RunnerDelegate* delegate_;
  #if defined(_DEBUG)
  private: int in_scope_;
  #endif
  private: base::WeakPtrFactory<Runner> weak_factory_;

  public: explicit Runner(v8::Isolate* isolate, RunnerDelegate* delegate);
  public: ~Runner();

  public: static Runner* current_runner(v8::Isolate* isolate);
  public: v8::Handle<v8::Object> global() const;

  public: v8::Handle<v8::Value> Call(v8::Handle<v8::Value> callee,
      v8::Handle<v8::Value> receiver, const Args& args);
  private: bool CheckCallDepth();
  public: v8::Handle<v8::Value> CallAsConstructor(
      v8::Handle<v8::Value> callee, const Args& args);
  public: template<typename... Params> v8::Handle<v8::Value>
      Call(v8::Handle<v8::Value> callee, v8::Handle<v8::Value> receiver,
           Params... params);
  public: template<typename... Params> v8::Handle<v8::Value>
      CallAsConstructor(v8::Handle<v8::Value> callee, Params... params);
  public: v8::Handle<v8::Value> GetGlobalProperty(
      const base::StringPiece& name);
  public: base::WeakPtr<Runner> GetWeakPtr();
  public: void HandleTryCatch(const v8::TryCatch& try_catch);
  public: v8::Handle<v8::Value> Run(const base::string16& script_text,
      const base::string16& script_name);
  public: v8::Handle<v8::Value> Run(v8::Handle<v8::Script> script);

   DISALLOW_COPY_AND_ASSIGN(Runner);
};

template<typename... Params> v8::Handle<v8::Value>
Runner::Call(v8::Handle<v8::Value> callee, v8::Handle<v8::Value> receiver,
             Params... params) {
  return Call(callee, receiver, Args{params...});
}

template<typename... Params> v8::Handle<v8::Value>
Runner::CallAsConstructor(v8::Handle<v8::Value> callee, Params... params) {
  return CallAsConstructor(callee, Args{params...});
}

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_script_runner_h)
