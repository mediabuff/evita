// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/document_window.h"

#include "base/bind.h"
#include "evita/dom/document.h"
#include "evita/dom/selection.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// DocumentWindowClass
//
class DocumentWindowClass :
    public v8_glue::DerivedWrapperInfo<DocumentWindow, Window> {

  public: DocumentWindowClass(const char* name)
      : BaseClass(name) {
  }
  public: ~DocumentWindowClass() = default;

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("document", &DocumentWindow::document)
        .SetProperty("selection", &DocumentWindow::selection);
  }

  DISALLOW_COPY_AND_ASSIGN(DocumentWindowClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// DocumentWindow
//
DEFINE_SCRIPTABLE_OBJECT(DocumentWindow, DocumentWindowClass);

DocumentWindow::DocumentWindow(Selection* selection)
    : selection_(selection) {
}

DocumentWindow::~DocumentWindow() {
}

Document* DocumentWindow::document() const {
  return selection_->document();
}

}  // namespace dom
