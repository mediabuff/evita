// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/button_control.h"

#include "evita/dom/events/form_event.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// ButtonControlClass
//
class ButtonControlClass :
    public v8_glue::DerivedWrapperInfo<ButtonControl, FormControl> {

  public: ButtonControlClass(const char* name)
      : BaseClass(name) {
  }
  public: ~ButtonControlClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &ButtonControlClass::NewButtonControl);
  }

  private: static ButtonControl* NewButtonControl() {
    return new ButtonControl();
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder.SetProperty("text",
        &ButtonControl::text, &ButtonControl::set_text);
  }

  DISALLOW_COPY_AND_ASSIGN(ButtonControlClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ButtonControl
//
DEFINE_SCRIPTABLE_OBJECT(ButtonControl, ButtonControlClass);

ButtonControl::ButtonControl() {
}

ButtonControl::~ButtonControl() {
}

void ButtonControl::set_text(const base::string16& new_text) {
  if (text_ == new_text)
    return;
  text_ = new_text;
  NotifyControlChange();
}

}  // namespace dom
