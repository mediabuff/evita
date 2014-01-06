// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/console.h"

#include "evita/dom/buffer.h"
#include "evita/dom/document.h"
#include "evita/v8_glue/converter.h"

namespace dom {

namespace {
Document* GetOrCreateDocument(const base::string16& name) {
  if (auto const document = Document::Find(name))
    return document;
  return new Document(name);
}

void Log(const std::vector<base::string16>& messages) {
  auto const document = GetOrCreateDocument(L"*console log*");
  auto const buffer = document->buffer();
  for (auto message : messages) {
    buffer->Insert(buffer->GetEnd(), message.c_str());
  }
  buffer->Insert(buffer->GetEnd(), L"\n");
}

//////////////////////////////////////////////////////////////////////
//
// ConsoleWrapperInfo
//
class ConsoleWrapperInfo : public v8_glue::WrapperInfo {
  public: ConsoleWrapperInfo() : v8_glue::WrapperInfo("Console") {
  }
  public: ~ConsoleWrapperInfo() = default;

  public: virtual AbstractScriptable* singleton() const override {
    return Console::instance();
  }

  public: virtual const char* singleton_name() const override {
    return "console";
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder.SetMethod("log", Log);
  }
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Console
//
v8_glue::WrapperInfo* Console::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(ConsoleWrapperInfo, wrapper_info, ());
  return &wrapper_info;
}

}  // namespace dom
