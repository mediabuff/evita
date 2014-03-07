// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/os/file.h"

#include "base/memory/ref_counted.h"
#include "base/strings/string16.h"
#include "evita/dom/converter.h"
#include "evita/dom/public/io_delegate.h"
#include "evita/dom/script_controller.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/promise_callback.h"
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/script_callback.h"
#include "gin/array_buffer.h"
#include "v8_strings.h"

namespace dom {
namespace os {
struct FileError {
  int error_code;
  FileError(int error_code) : error_code(error_code) {
  }
};
}  // namespace os
}  // namespace dom

namespace gin {
template<>
struct Converter<dom::os::FileError> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    const dom::os::FileError& error) {
    auto const runner = v8_glue::Runner::current_runner(isolate);
    auto const os_file_error_ctor = runner->global()->
        Get(dom::v8Strings::Os.Get(isolate))->ToObject()->
        Get(dom::v8Strings::File.Get(isolate))->ToObject()->
        Get(dom::v8Strings::Error.Get(isolate));
    return runner->CallAsConstructor(os_file_error_ctor,
        v8::Integer::New(isolate, error.error_code));
  }
};
}  // namespace gin

namespace dom {
namespace os {

namespace {

//////////////////////////////////////////////////////////////////////
//
// FileClass
//
class FileClass : public v8_glue::WrapperInfo {
  private: typedef v8_glue::WrapperInfo BaseClass;

  public: FileClass(const char* name)
      : BaseClass(name) {
  }
  public: ~FileClass() = default;

  private: static File* NewFile() {
    ScriptController::instance()->ThrowError(
        "Cannot create an instance of File.");
    return nullptr;
  }

  private: static v8::Handle<v8::Object> OpenFile(
      const base::string16& file_name,
      v8_glue::Optional<base::string16> opt_mode);
  private: static void QueryFileStatus(const base::string16& filename,
                                        v8::Handle<v8::Function> callback);

  // v8_glue::WrapperInfo
  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto const templ = v8_glue::CreateConstructorTemplate(isolate,
        &FileClass::NewFile);
  return v8_glue::FunctionTemplateBuilder(isolate, templ)
      .SetMethod("open", &FileClass::OpenFile)
      .SetMethod("stat_", &FileClass::QueryFileStatus)
      .Build();
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetMethod("close", &File::Close)
        .SetMethod("read", &File::Read)
        .SetMethod("write", &File::Write);
  }

  DISALLOW_COPY_AND_ASSIGN(FileClass);
};

//////////////////////////////////////////////////////////////////////
//
// FileIoCallback
//
class FileIoCallback : public v8_glue::PromiseCallback {
  public: FileIoCallback(v8_glue::Runner* runner)
    : v8_glue::PromiseCallback(runner) {
  }

  public: ~FileIoCallback() = default;

  public: void Run(int num_transfered, int error_code) {
    if (error_code) {
      Reject(FileError(error_code));
      return;
    }
    Resolve(num_transfered);
  }

  DISALLOW_COPY_AND_ASSIGN(FileIoCallback);
};

//////////////////////////////////////////////////////////////////////
//
// OpenFileCallback
//
class OpenFileCallback : public v8_glue::PromiseCallback {
  public: OpenFileCallback(v8_glue::Runner* runner)
    : v8_glue::PromiseCallback(runner) {
  }

  public: void Run(domapi::IoHandle* handle, int error_code) {
    if (error_code) {
      Reject(FileError(error_code));
      return;
    }
    Resolve(new File(handle));
  }

  DISALLOW_COPY_AND_ASSIGN(OpenFileCallback);
};

//////////////////////////////////////////////////////////////////////
//
// QueryFileStatusCallback
//
class QueryFileStatusCallback
    : public base::RefCounted<QueryFileStatusCallback> {
  private: v8_glue::ScopedPersistent<v8::Function> function_;
  private: base::WeakPtr<v8_glue::Runner> runner_;

  public: QueryFileStatusCallback(v8_glue::Runner* runner,
                                  v8::Handle<v8::Function> function)
      : function_(runner->isolate(), function), runner_(runner->GetWeakPtr()) {
  }

  public: ~QueryFileStatusCallback() = default;

  public: void Run(const domapi::QueryFileStatusCallbackData& data) {
    if (!runner_)
      return;
    v8_glue::Runner::Scope runner_scope(runner_.get());
    auto const isolate = runner_->isolate();
    auto const function = function_.NewLocal(isolate);
    auto const js_data = v8::Object::New(isolate);
    js_data->Set(v8Strings::errorCode.Get(isolate),
                 gin::ConvertToV8(isolate, data.error_code));
    js_data->Set(v8Strings::isDir.Get(isolate),
                 gin::ConvertToV8(isolate, data.is_directory));
    js_data->Set(v8Strings::isSymLink.Get(isolate),
                 gin::ConvertToV8(isolate, data.is_symlink));
    js_data->Set(v8Strings::lastModificationDate.Get(isolate),
                 gin::ConvertToV8(isolate, data.last_write_time));
    js_data->Set(v8Strings::readonly.Get(isolate),
                 gin::ConvertToV8(isolate, data.readonly));
    runner_->Call(function, v8::Undefined(isolate), js_data);
    js_data->Set(v8Strings::size.Get(isolate),
                 gin::ConvertToV8(isolate, data.file_size));
  }

  DISALLOW_COPY_AND_ASSIGN(QueryFileStatusCallback);
};

v8::Handle<v8::Object> FileClass::OpenFile(const base::string16& file_name,
    v8_glue::Optional<base::string16> opt_mode) {
  auto const runner = ScriptController::instance()->runner();
  v8_glue::Runner::EscapableHandleScope runner_scope(runner);
  auto const file_io_callback = make_scoped_refptr(
      new OpenFileCallback(runner));
  ScriptController::instance()->io_delegate()->OpenFile(file_name,
      opt_mode.is_supplied ? opt_mode.value : base::string16(),
      base::Bind(&OpenFileCallback::Run, file_io_callback));
  return runner_scope.Escape(file_io_callback->GetPromise(runner->isolate()));
}

void FileClass::QueryFileStatus(const base::string16& filename,
                                v8::Handle<v8::Function> callback) {
  auto const runner = ScriptController::instance()->runner();
  auto const query_file_status_callback = make_scoped_refptr(
      new QueryFileStatusCallback(runner, callback));
  ScriptController::instance()->io_delegate()->QueryFileStatus(
      filename, base::Bind(&QueryFileStatusCallback::Run,
                           query_file_status_callback));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// File
//
DEFINE_SCRIPTABLE_OBJECT(File, FileClass);

File::File(domapi::IoHandle* handle)
    : closed_(false), handle_(handle) {
}

File::~File() {
  Close();
}

void File::Close() {
  if (closed_)
    return;
  closed_ = true;
  ScriptController::instance()->io_delegate()->CloseFile(handle_);
}

v8::Handle<v8::Object> File::Read(
    const gin::ArrayBufferView& array_buffer_view) {
  auto const runner = ScriptController::instance()->runner();
  v8_glue::Runner::EscapableHandleScope runner_scope(runner);
  auto const file_io_callback = make_scoped_refptr(
      new FileIoCallback(runner));
  ScriptController::instance()->io_delegate()->ReadFile(
      handle_, array_buffer_view.bytes(), array_buffer_view.num_bytes(),
      base::Bind(&FileIoCallback::Run, file_io_callback));
  return runner_scope.Escape(file_io_callback->GetPromise(runner->isolate()));
}

v8::Handle<v8::Object> File::Write(
    const gin::ArrayBufferView& array_buffer_view) {
  auto const runner = ScriptController::instance()->runner();
  v8_glue::Runner::EscapableHandleScope runner_scope(runner);
  auto const file_io_callback = make_scoped_refptr(
      new FileIoCallback(runner));
  ScriptController::instance()->io_delegate()->WriteFile(
      handle_, array_buffer_view.bytes(), array_buffer_view.num_bytes(),
      base::Bind(&FileIoCallback::Run, file_io_callback));
  return runner_scope.Escape(file_io_callback->GetPromise(runner->isolate()));
}

}  // namespace os
}  // namespace dom
