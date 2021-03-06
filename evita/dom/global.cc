// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/global.h"

#include "evita/dom/components/clipboard/data_transfer.h"
#include "evita/dom/components/clipboard/data_transfer_item.h"
#include "evita/dom/components/clipboard/data_transfer_item_list.h"
#include "evita/dom/components/imaging/image_data.h"
#include "evita/dom/components/win_registry/win_registry.h"
#include "evita/dom/components/win_resource/win_resource.h"
#include "evita/dom/css/css_style_sheet_handle.h"
#include "evita/dom/editor.h"
#include "evita/dom/encodings/text_decoder.h"
#include "evita/dom/encodings/text_encoder.h"
#include "evita/dom/engine/native_script_module.h"
#include "evita/dom/events/composition_event.h"
#include "evita/dom/events/event.h"
#include "evita/dom/events/event_target.h"
#include "evita/dom/events/focus_event.h"
#include "evita/dom/events/form_event.h"
#include "evita/dom/events/keyboard_event.h"
#include "evita/dom/events/mouse_event.h"
#include "evita/dom/events/text_document_event.h"
#include "evita/dom/events/ui_event.h"
#include "evita/dom/events/view_event_target.h"
#include "evita/dom/events/wheel_event.h"
#include "evita/dom/events/window_event.h"
#include "evita/dom/file_path.h"
#include "evita/dom/forms/button_control.h"
#include "evita/dom/forms/checkbox_control.h"
#include "evita/dom/forms/form.h"
#include "evita/dom/forms/form_control.h"
#include "evita/dom/forms/label_control.h"
#include "evita/dom/forms/radio_button_control.h"
#include "evita/dom/forms/text_field_control.h"
#include "evita/dom/forms/text_field_selection.h"
#include "evita/dom/os/directory.h"
#include "evita/dom/os/file.h"
#include "evita/dom/os/process.h"
#include "evita/dom/script_host.h"
#include "evita/dom/text/regular_expression.h"
#include "evita/dom/text/text_document.h"
#include "evita/dom/text/text_mutation_observer.h"
#include "evita/dom/text/text_mutation_record.h"
#include "evita/dom/text/text_range.h"
#include "evita/dom/timers/one_shot_timer.h"
#include "evita/dom/timers/repeating_timer.h"
#include "evita/dom/timers/timer.h"
#include "evita/dom/visuals/node_handle.h"
#include "evita/dom/windows/editor_window.h"
#include "evita/dom/windows/form_window.h"
#include "evita/dom/windows/text_selection.h"
#include "evita/dom/windows/text_window.h"
#include "evita/dom/windows/visual_window.h"
#include "evita/dom/windows/window.h"

namespace dom {

Global::Global() {}

Global::~Global() {}

v8::Local<v8::ObjectTemplate> Global::GetObjectTemplate(v8::Isolate* isolate) {
  if (!object_template_->IsEmpty())
    return object_template_.NewLocal(isolate);

  {
    v8::HandleScope handle_scope(isolate);

    auto global_templ = v8::ObjectTemplate::New(isolate);

    auto context = v8::Context::New(isolate);
    v8::Context::Scope context_scope(context);

#define INSTALL(name) ginx::Installer<name>::Run(isolate, global_templ)

    // Clipboard
    INSTALL(DataTransfer);
    INSTALL(DataTransferItem);
    INSTALL(DataTransferItemList);

    // Components
    INSTALL(ImageData);
    INSTALL(WinRegistry);
    INSTALL(WinResource);

    // Note: super class must be installed before subclass.
    INSTALL(Event);
    INSTALL(TextDocumentEvent);
    INSTALL(FormEvent);
    INSTALL(UiEvent);
    INSTALL(CompositionEvent);
    INSTALL(FocusEvent);
    INSTALL(KeyboardEvent);
    INSTALL(MouseEvent);
    INSTALL(WheelEvent);
    INSTALL(WindowEvent);

    auto const editor_templ = INSTALL(Editor);
    INSTALL(FilePath);
    INSTALL(NativeScriptModule);
    INSTALL(TextRange);
    INSTALL(RegularExpression);

    INSTALL(EventTarget);
    INSTALL(TextDocument);
    INSTALL(ViewEventTarget);
    INSTALL(Form);
    INSTALL(FormControl);
    INSTALL(ButtonControl);
    INSTALL(CheckboxControl);
    INSTALL(LabelControl);
    INSTALL(RadioButtonControl);
    INSTALL(TextFieldControl);
    INSTALL(Window);
    INSTALL(TextWindow);
    INSTALL(EditorWindow);
    INSTALL(FormWindow);
    INSTALL(TextFieldSelection);
    INSTALL(VisualWindow);

    INSTALL(TextMutationObserver);
    INSTALL(TextMutationRecord);

    INSTALL(TextSelection);

    INSTALL(TextDecoder);
    INSTALL(TextEncoder);

    INSTALL(Timer);
    INSTALL(OneShotTimer);
    INSTALL(RepeatingTimer);

    INSTALL(NodeHandle);
    INSTALL(CSSStyleSheetHandle);

#define INSTALL_IN(templ, name)        \
  templ->Set(                          \
      gin::StringToV8(isolate, #name), \
      name::static_wrapper_info()->GetOrCreateConstructorTemplate(isolate))

    // Os
    auto const os_templ = v8::ObjectTemplate::New(isolate);
    global_templ->Set(gin::StringToV8(isolate, "Os"), os_templ);
    INSTALL_IN(os_templ, AbstractFile);
    INSTALL_IN(os_templ, Directory);
    INSTALL_IN(os_templ, File);
    INSTALL_IN(os_templ, Process);

    // Global template is ready now.
    object_template_.Reset(isolate, global_templ);
  }

  return object_template_.NewLocal(isolate);
}

}  // namespace dom
