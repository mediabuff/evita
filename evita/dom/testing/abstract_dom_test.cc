// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

#include <vector>

#include "build/build_config.h"

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "common/memory/singleton.h"
#include "evita/base/resource/resource_bundle.h"
#include "evita/dom/converter.h"
#include "evita/dom/global.h"
#include "evita/dom/lock.h"
#include "evita/dom/scheduler/scheduler.h"
#include "evita/dom/script_host.h"
#include "evita/dom/testing/gmock.h"
#include "evita/dom/testing/gtest.h"
#include "evita/dom/testing/mock_io_delegate.h"
#include "evita/dom/testing/mock_view_impl.h"
#include "evita/dom/testing/test_runner.h"
#include "evita/dom/timing/mock_scheduler.h"
#include "evita/dom/view_event_handler_impl.h"
#include "evita/ginx/runner_delegate.h"

#if OS_WIN
#include <objbase.h>
#endif

namespace dom {

using ::testing::_;

namespace {
ginx::Runner* static_runner;
AbstractDomTest* static_current_test;

class RunnerDelegateMock final : public ginx::RunnerDelegate {
 public:
  RunnerDelegateMock() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(RunnerDelegateMock);
};

v8::MaybeLocal<v8::Module> ModuleResolverCallback(
    v8::Local<v8::Context> context,
    v8::Local<v8::String> specifier,
    v8::Local<v8::Module> referrer) {
  NOTREACHED() << "We'll support module import int testing.";
  return v8::Local<v8::Module>();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// AbstractDomTest::RunnerDelegate
//
class AbstractDomTest::RunnerDelegate final
    : public common::Singleton<RunnerDelegate>,
      public ginx::RunnerDelegate {
 public:
  RunnerDelegate() = default;
  ~RunnerDelegate() final = default;

  void set_test_instance(AbstractDomTest* test_instance) {
    test_instance_ = test_instance;
  }

 private:
  // ginx::RunnerDelegate
  v8::Local<v8::ObjectTemplate> GetGlobalTemplate(ginx::Runner* runner) final;
  void UnhandledException(ginx::Runner* runner,
                          const v8::TryCatch& try_catch) final;

  AbstractDomTest* test_instance_;
};

// ginx::RunnerDelegate
v8::Local<v8::ObjectTemplate>
AbstractDomTest::RunnerDelegate::GetGlobalTemplate(ginx::Runner* runner) {
  auto const templ =
      static_cast<ginx::RunnerDelegate*>(test_instance_->script_host_)
          ->GetGlobalTemplate(runner);
  auto const isolate = runner->isolate();
  RunnerDelegateMock temp_delegate;
  ginx::Runner temp_runner(isolate, &temp_delegate);
  auto const context = v8::Context::New(isolate);
  v8::Context::Scope context_scope(context);
  test_instance_->PopulateGlobalTemplate(isolate, templ);

  const auto testing_templ = v8::ObjectTemplate::New(isolate);
  templ->Set(gin::StringToV8(isolate, "testing"), testing_templ);

  GMock::Install(isolate, testing_templ);
  GTest::Install(isolate, testing_templ);
  TestRunner::Install(isolate, templ);
  return templ;
}

void AbstractDomTest::RunnerDelegate::UnhandledException(
    ginx::Runner*,
    const v8::TryCatch& try_catch) {
  test_instance_->exception_ = *v8::String::Utf8Value(try_catch.Exception());
}

//////////////////////////////////////////////////////////////////////
//
// AbstractDomTest::RunnerScope
//
AbstractDomTest::RunnerScope::RunnerScope(AbstractDomTest* test)
    : runner_scope_(test->runner()) {}

AbstractDomTest::RunnerScope::~RunnerScope() {}

//////////////////////////////////////////////////////////////////////
//
// AbstractDomTest::ScriptCallArguments
//
AbstractDomTest::ScriptCallArguments::ScriptCallArguments(v8::Isolate* isolate)
    : isolate_(isolate) {}

AbstractDomTest::ScriptCallArguments::~ScriptCallArguments() {}

void AbstractDomTest::ScriptCallArguments::Populate() {}

//////////////////////////////////////////////////////////////////////
//
// AbstractDomTest
//
AbstractDomTest::AbstractDomTest()
    : mock_io_delegate_(new MockIoDelegate()),
      mock_scheduler_(new MockScheduler()),
      mock_view_impl_(new MockViewImpl()),
      script_host_(nullptr) {
  static_current_test = this;
  static bool is_initialized;
  if (is_initialized)
    return;
#if OS_WIN
  ::CoInitialize(nullptr);
#endif
  base::FilePath pack_path;
  base::PathService::Get(base::DIR_EXE, &pack_path);
  pack_path = pack_path.AppendASCII("evita_resources.pak");
  base::ResourceBundle::GetInstance()->AddDataPackFromPath(pack_path);
  is_initialized = true;
}

AbstractDomTest::~AbstractDomTest() {
  static_current_test = nullptr;
}

v8::Isolate* AbstractDomTest::isolate() const {
  return runner_->isolate();
}

domapi::ViewEventHandler* AbstractDomTest::view_event_handler() const {
  return script_host_->event_handler();
}

base::FilePath AbstractDomTest::BuildPath(
    const std::vector<base::StringPiece>& components) {
  base::FilePath path;
  PathService::Get(base::DIR_SOURCE_ROOT, &path);
  path = path.AppendASCII("src").AppendASCII("evita").AppendASCII("dom");
  for (const auto& component : components)
    path = path.AppendASCII(component);
  return path;
}

bool AbstractDomTest::DoCall(const base::StringPiece& name, const Argv& argv) {
  ginx::Runner::Scope runner_scope(runner_.get());
  auto const isolate = runner_->isolate();
  auto callee = runner_->GetGlobalProperty(name);
  auto const result = runner_->CallAsFunction(callee, v8::Null(isolate), argv);
  return !result.IsEmpty();
}

std::string AbstractDomTest::EvalScript(const base::StringPiece& script_text,
                                        const char* file_name,
                                        int line_number) {
  ginx::Runner::Scope runner_scope(runner_.get());
  auto const isolate = runner_->isolate();
  v8::ScriptOrigin script_origin(gin::StringToV8(isolate, file_name),
                                 v8::Integer::New(isolate, line_number),
                                 v8::Integer::New(isolate, 0));
  v8::ScriptCompiler::Source source(gin::StringToV8(isolate, script_text),
                                    script_origin);
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(true);
  auto script = v8::ScriptCompiler::Compile(runner_->context(), &source);
  if (script.IsEmpty()) {
    UnhandledException(runner_.get(), try_catch);
    return exception_;
  }
  DOM_AUTO_LOCK_SCOPE();
  auto const result = script.ToLocalChecked()->Run();
  if (result.IsEmpty()) {
    UnhandledException(runner_.get(), try_catch);
    return exception_;
  }
  return *v8::String::Utf8Value(result);
}

// static
AbstractDomTest* AbstractDomTest::GetInstance() {
  DCHECK(static_current_test);
  return static_current_test;
}

void AbstractDomTest::PopulateGlobalTemplate(v8::Isolate*,
                                             v8::Local<v8::ObjectTemplate>) {}

void AbstractDomTest::RunFile(const base::FilePath& path) {
  ginx::Runner::Scope runner_scope(runner_.get());
  const auto& file_name = path.LossyDisplayName();
  std::string script_text;
  ASSERT_TRUE(base::ReadFileToString(path, &script_text)) << file_name;
  const int kLineNumber = 0;
  const int kColumnNumber = 0;
  auto* const isolate = runner_->isolate();
  v8::ScriptOrigin script_origin(gin::StringToV8(isolate, file_name),
                                 v8::Integer::New(isolate, kLineNumber),
                                 v8::Integer::New(isolate, kColumnNumber),
                                 v8::False(isolate),  // is_shared_cross_origin
                                 v8::Local<v8::Integer>(),  // script_id
                                 v8::Local<v8::Value>(),    // source_map_url
                                 v8::False(isolate),        // is_opaque
                                 v8::False(isolate),        // is_wasm
                                 v8::True(isolate));        // is_module
  DCHECK(script_origin.Options().IsModule());
  v8::ScriptCompiler::Source source(gin::StringToV8(isolate, script_text),
                                    script_origin);
  auto module = v8::ScriptCompiler::CompileModule(isolate, &source)
                    .FromMaybe(v8::Local<v8::Module>());
  if (module.IsEmpty()) {
    FAIL() << "Failed to compile \"" << file_name << '"';
    return;
  }
  auto context = runner_->context();
  if (!module->Instantiate(context, ModuleResolverCallback)) {
    FAIL() << "Failed to resolve \"" << file_name << '"';
    return;
  }
  DOM_AUTO_LOCK_SCOPE();
  if (module->Evaluate(context).IsEmpty()) {
    FAIL() << "Failed to evaluate \"" << file_name << '"';
    return;
  }
  isolate->RunMicrotasks();
}

void AbstractDomTest::RunFile(
    const std::vector<base::StringPiece>& components) {
  RunFile(BuildPath(components));
}

bool AbstractDomTest::RunScript(const base::StringPiece& script_text,
                                const base::StringPiece& file_name,
                                int line_number) {
  ginx::Runner::Scope runner_scope(runner_.get());
  auto const isolate = runner_->isolate();
  v8::ScriptOrigin script_origin(gin::StringToV8(isolate, file_name),
                                 v8::Integer::New(isolate, line_number),
                                 v8::Integer::New(isolate, 0));
  v8::ScriptCompiler::Source source(gin::StringToV8(isolate, script_text),
                                    script_origin);
  v8::TryCatch try_catch(isolate);
  try_catch.SetVerbose(true);
  auto script = v8::ScriptCompiler::Compile(runner_->context(), &source);
  if (script.IsEmpty()) {
    UnhandledException(runner_.get(), try_catch);
    return false;
  }
  DOM_AUTO_LOCK_SCOPE();
  auto const result = script.ToLocalChecked()->Run();
  if (result.IsEmpty()) {
    UnhandledException(runner_.get(), try_catch);
    return false;
  }
  isolate->RunMicrotasks();
  return true;
}

void AbstractDomTest::RunMessageLoopUntilIdle() {
  scoped_task_environment_.RunUntilIdle();
  mock_scheduler_->RunPendingTasks();
  script_host_->RunMicrotasks();
}

void AbstractDomTest::SetUp() {
  RunnerDelegate::instance()->set_test_instance(this);

  script_host_ = dom::ScriptHost::StartForTesting(
      mock_scheduler_.get(), mock_view_impl_.get(), mock_io_delegate_.get());

  auto const isolate = script_host_->isolate();

  if (static_runner) {
    script_host_->set_testing_runner(static_runner);
    runner_.reset(static_runner);

    // Install test specific JavaScript objects.
    RunnerScope runner_scope(this);
    auto const object_template = v8::ObjectTemplate::New(isolate);
    PopulateGlobalTemplate(isolate, object_template);
    auto const global = runner_->global();
    auto const object = object_template->NewInstance();
    auto const keys = object->GetPropertyNames();
    auto const keys_length = keys->Length();
    for (auto index = 0u; index < keys_length; ++index) {
      auto const key = keys->Get(index);
      global->Set(key, object->Get(key));
    }
    runner_->Run(L"core.Initializer.initialize()", L"*test*");
    return;
  }

  auto const runner = new ginx::Runner(isolate, RunnerDelegate::instance());
  runner_.reset(runner);
  script_host_->set_testing_runner(runner);
  ginx::Runner::Scope runner_scope(runner);
  DOM_AUTO_LOCK_SCOPE();
  Global::LoadGlobalScript(runner);
}

void AbstractDomTest::TearDown() {
  // Discard schedule tasks, e.g. TextDocumentSet::Observer callbacks.
  scoped_task_environment_.RunUntilIdle();
  if (!static_runner || static_runner == runner_.get())
    static_runner = runner_.release();
}

void AbstractDomTest::UnhandledException(ginx::Runner* runner,
                                         const v8::TryCatch& try_catch) {
  static_cast<ginx::RunnerDelegate*>(RunnerDelegate::instance())
      ->UnhandledException(runner, try_catch);
}

}  // namespace dom
