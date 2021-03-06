// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ginx_{{interface_name}}.h"

{% for include_path in include_paths %}
#include "{{include_path}}"
{% endfor %}

namespace dom {
namespace bindings {
{#//////////////////////////////////////////////////////////////////////
 //
 // emit_arguments
 //   signature
 //     .call_with_list
 //     .is_raises_exception
 //     .parameters
 #}
{% macro emit_arguments(signature) %}
{%- set delimiter = '' %}
{%- for call_with in signature.call_with_list %}
{{ delimiter }}{% set delimiter = ', ' %}
{%-   if call_with == 'Runner' %}{{ 'script_host->runner()' }}
{%-   elif call_with == 'ScriptHost' %}{{ 'script_host' }}
{%-   elif call_with == 'ViewDelegate' %}{{ 'script_host->view_delegate()' }}
{%-   else %}
  #error Invalid CallWith value "{{call_with}}".
{%-   endif %}
{%- endfor %}
{%- if signature.call_with_list|count %}
{%-   set delimiter = ', '%}
{%- endif %}
{%- for parameter in signature.parameters -%}
{{ delimiter }}{% set delimiter = ', ' %}param{{ loop.index0 }}
{%- endfor %}
{%- if signature.parameters|count %}
{%-   set delimiter = ', '%}
{%- endif %}
{% if signature.is_raises_exception %}
{{ delimiter }}{% set delimiter = ', ' %}{{'&exception_state'}}
{%- endif %}
{%- endmacro %}
{#
 # Example:
 #  Type0 param0;
 #  const auto arg0 = info[0];
 #  if (!gin::ConvertFromV8(isolate, arg0, &param0)) {
 #    exception_state.ThrowArgumentError("display_type", arg0, 0);
 #    return nullptr;
 #  }
 #  Type1 param1;
 #  const auto arg1 = info[1];
 #  if (!gin::ConvertFromV8(isolate, arg1, &param1)) {
 #    exception_state.ThrowArgumentError("display_type", arg1, 1);
 #    return nullptr;
 #  }
 #  return new Example(param1, param2);
 #}
{% macro emit_constructor_signature(ctor_name, signature, indent='  ') %}
{% if signature.parameters %}
{%-  for parameter in signature.parameters %}
{{indent}}{{parameter.from_v8_type}} param{{ loop.index0 }};
{{indent}}const auto arg{{ loop.index0 }} = info[{{ loop.index0 }}];
{{indent}}if (!gin::ConvertFromV8(isolate, arg{{ loop.index0 }}, &param{{ loop.index0 }})) {
{{indent}}  exception_state.ThrowArgumentError("{{parameter.display_type}}", arg{{ loop.index0 }}, {{ loop.index0 }});
{{indent}}  return nullptr;
{{indent}}}
{% endfor %}
{% endif %}
{{indent}}return {{ctor_name}}({{ emit_arguments(signature) }});
{%- endmacro %}
{#######################################################################
 #
 # emit_method
 #}
{% macro emit_method(method) %}
void {{class_name}}::{{method.cc_name}}(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
{{ emit_prologue('MethodCall', method, method.name) }}
{% if not method.is_static %}
  {{interface_name}}* impl = nullptr;
  if (!gin::ConvertFromV8(isolate, info.This(), &impl)) {
    exception_state.ThrowReceiverError(info.This());
    return;
  }
{% endif %}
{% if method.dispatch == 'single' %}
  if (info.Length() != {{method.min_arity}}) {
    exception_state.ThrowArityError({{method.min_arity}}, {{method.min_arity}}, info.Length());
    return;
  }
{{ emit_method_signature(method.signature) }}
{% elif method.dispatch == 'arity' %}
  switch (info.Length()) {
{%  for signature in method.signatures %}
    case {{ signature.parameters | count }}: {
{{    emit_method_signature(signature, indent='      ') }}
      return;
    }
{%  endfor %}
  }
  exception_state.ThrowArityError({{ method.min_arity }}, {{ method.max_arity }}, info.Length());
{% else %}
#error "Unsupported dispatch type {{method.dispatch}}"
{% endif %}
}
{% endmacro %}
{#######################################################################
 #
 # emit_method_signature
 #}
{%- macro emit_method_signature(signature, indent='  ') %}
{% if signature.parameters %}

{%-  for parameter in signature.parameters %}
{{indent}}{{parameter.from_v8_type}} param{{ loop.index0 }};
{{indent}}const auto arg{{ loop.index0 }} = info[{{ loop.index0 }}];
{{indent}}if (!gin::ConvertFromV8(isolate, arg{{ loop.index0 }}, &param{{ loop.index0 }})) {
{{indent}}  exception_state.ThrowArgumentError("{{parameter.display_type}}", arg{{ loop.index0 }}, {{ loop.index0 }});
{{indent}}  return;
{{indent}}}
{% endfor %}
{% endif %}
{% if signature.is_static %}
{%   set callee = interface_name + '::' + signature.cc_name %}
{% else %}
{%   set callee = 'impl->' + signature.cc_name %}
{% endif %}
{%- if signature.to_v8_type == 'void' %}
{{indent}}{{callee}}({{ emit_arguments(signature) }});
{%- else %}
{{indent}}{{signature.to_v8_type}} value = {{callee}}({{ emit_arguments(signature) }});
{% if signature.is_raises_exception %}
{{indent}}if (exception_state.is_thrown())
{{indent}}  return;
{% endif %}
{%-     if signature.can_fast_return %}
{{indent}}info.GetReturnValue().Set(value);
{%-     else %}
{{indent}}v8::Local<v8::Value> v8_value;
{{indent}}if (!gin::TryConvertToV8(isolate, value, &v8_value))
{{indent}}  return;
{{indent}}info.GetReturnValue().Set(v8_value);
{%-     endif %}
{%- endif %}
{%- endmacro %}
{#//////////////////////////////////////////////////////////////////////
 //
 // emit_prologue
 //  situation       One of Construction, MethodCall, PropertyGet, PropertySet
 //  model           A data model which has 'call_with_list'.
 //  property_name   A name of property.
 #}
{%- macro emit_prologue(situation, model, property_name=None) %}
{%-  set indent = '  ' %}
{{indent}}const auto isolate = info.GetIsolate();
{{indent}}const auto context = isolate->GetCurrentContext();
{{indent}}ExceptionState exception_state(ExceptionState::Situation::{{situation}}, context, "{{interface_name}}"
{%- if property_name is defined %}
, "{{property_name}}"
{%- endif %});
{% if model.use_call_with %}
{{indent}}const auto script_host = ginx::Runner::From(context)->user_data<ScriptHost>();
{% endif %}
{%- endmacro %}
//////////////////////////////////////////////////////////////////////
//
// {{class_name}}
//
{{class_name}}::{{class_name}}(const char* name)
{%- if interface_parent -%}
{{ '\n  : BaseClass(name) ' }}
{%- else -%}
{{ '\n  : ginx::WrapperInfo(name) ' }}
{%- endif -%} {
}

{{class_name}}::~{{class_name}}() {
}

{% if constructor.dispatch != 'none' %}
void {{class_name}}::Construct{{interface_name}}(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (!ginx::internal::IsValidConstructCall(info))
    return;
  ginx::internal::FinishConstructCall(info, New{{interface_name}}(info));
}

//////////////////////////////////////////////////////////////////////
//
// {{interface_name}} constructor dispatcher. This function calls one of
// followings:
{% for signature in constructor.signatures %}
//   {{loop.index}} {{interface_name}}(
{%-     for parameter in signature.parameters -%}
         {{parameter.display_type}} {{parameter.cc_name}}
         {%- if not loop.last %}{{', '}}{% endif %}
{%-     endfor %})
{% endfor %}
//
{{interface_name}}* {{class_name}}::New{{interface_name}}(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  {{ emit_prologue('Construction', constructor) }}
{% if constructor.dispatch == 'single' %}
  if (info.Length() != {{constructor.min_arity}}) {
    exception_state.ThrowArityError({{constructor.min_arity}}, {{constructor.min_arity}}, info.Length());
    return nullptr;
  }
{{ emit_constructor_signature(constructor.cc_name, constructor.signature) }}
{% elif constructor.dispatch == 'arity' %}
  switch (info.Length()) {
{%  for signature in constructor.signatures %}
    case {{ signature.parameters | count }}: {
{{    emit_constructor_signature(constructor.cc_name, signature, indent='      ') }}
    }
{%  endfor %}
  }
  exception_state.ThrowArityError({{ constructor.min_arity }}, {{ constructor.max_arity }}, info.Length());
  return nullptr;
{% else %}
#error "Unsupported dispatch type {{constructor.dispatch}}"
{% endif %}
}

{% endif %}
{#############################################################
 #
 # Static attributes
 #}
{% for attribute in attributes if attribute.is_static %}
{%  if loop.first %}
// Static attributes
{%  endif %}
void {{class_name}}::Get_{{attribute.cc_name}}(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  {{ emit_prologue('PropertyGet', attribute, attribute.name) }}
  if (info.Length() != 0) {
    exception_state.ThrowArityError(0, 0, info.Length());
    return;
  }
{% if attribute.getter_raises_exception %}
  {{attribute.to_v8_type}} value = {{interface_name}}::{{attribute.cc_name}}(&exception_state);
  if (exception_state.is_thrown())
    return;
{% else %}
  {{attribute.to_v8_type}} value = {{interface_name}}::{{attribute.cc_name}}();
{% endif %}
{%  if attribute.can_fast_return %}
  info.GetReturnValue().Set(value);
{%  else %}
  v8::Local<v8::Value> v8_value;
  if (!gin::TryConvertToV8(isolate, value, &v8_value))
    return;
  info.GetReturnValue().Set(v8_value);
{%  endif %}
}

{%  if not attribute.is_read_only %}
void {{class_name}}::Set_{{attribute.cc_name}}(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
{{ emit_prologue('PropertySet', attribute, attribute.name) }}
  if (info.Length() != 1) {
    exception_state.ThrowArityError(1, 1, info.Length());
    return;
  }
  {{attribute.from_v8_type}} new_value;
  if (!gin::ConvertFromV8(isolate, info[0], &new_value)) {
    exception_state.ThrowArgumentError("{{attribute.display_type}}", info.info[0], 0);
    return;
  }
{% if attribute.setter_raises_exception %}
  {{interface_name}}::set_{{attribute.cc_name}}(new_value, &exception_state);
{% else %}
  {{interface_name}}::set_{{attribute.cc_name}}(new_value);
{% endif %}
}

{%  endif %}
{% endfor %}
{#############################################################
 #
 # Static methods
 #}
{% for method in methods if method.is_static %}
{%  if loop.first %}
// Static methods
{%  endif %}
{{emit_method(method)}}
{% endfor %}
{#############################################################
 #
 # Instance attributes
 #}
{% for attribute in attributes if not attribute.is_static %}
{%  if loop.first %}
// Instance attributes
{%  endif %}
void {{class_name}}::Get_{{attribute.cc_name}}(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
{{ emit_prologue('PropertyGet', attribute, attribute.name) }}
  {{interface_name}}* impl = nullptr;
  if (!gin::ConvertFromV8(isolate, info.This(), &impl)) {
    exception_state.ThrowReceiverError(info.This());
    return;
  }
  {{attribute.to_v8_type}} value = impl->{{attribute.cc_name}}({{ emit_arguments(attribute.getter_signature) }});
{% if attribute.getter_signature.is_raises_exception %}
  if (exception_state.is_thrown())
    return;
{% endif %}
{%    if attribute.can_fast_return %}
  info.GetReturnValue().Set(value);
{%    else %}
  v8::Local<v8::Value> v8_value;
  if (!gin::TryConvertToV8(isolate, value, &v8_value))
    return;
  info.GetReturnValue().Set(v8_value);
{%    endif %}
}

{%  if not attribute.is_read_only %}
void {{class_name}}::Set_{{attribute.cc_name}}(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
{{ emit_prologue('PropertySet', attribute, attribute.name) }}
  if (info.Length() != 1) {
    exception_state.ThrowArityError(1, 1, info.Length());
    return;
  }
  {{interface_name}}* impl = nullptr;
  if (!gin::ConvertFromV8(isolate, info.This(), &impl)) {
    exception_state.ThrowReceiverError(info.This());
    return;
  }
  {{attribute.from_v8_type}} param0;
  if (!gin::ConvertFromV8(isolate, info[0], &param0)) {
    exception_state.ThrowArgumentError("{{attribute.display_type}}", info[0], 0);
    return;
  }
  impl->set_{{attribute.cc_name}}({{ emit_arguments(attribute.setter_signature) }});
}
{% endif %}
{% endfor %}
{#############################################################
 #
 # Instance methods
 #}
{% for method in methods if not method.is_static %}
{%  if loop.first %}
// Instance methods
{%  endif %}
{{emit_method(method)}}
{% endfor %}
// ginx::WrapperInfo
{% if has_static_member or constructor.dispatch != 'none' %}
v8::Local<v8::FunctionTemplate>
{{class_name}}::CreateConstructorTemplate(v8::Isolate* isolate) {
{###############################
 #
 # Constructor
 #}
{% if constructor.dispatch != 'none' %}
  auto templ = v8::FunctionTemplate::New(isolate,
      &{{class_name}}::Construct{{interface_name}});
{% else %}
  auto templ = ginx::WrapperInfo::CreateConstructorTemplate(isolate);
{% endif %}
{% if has_static_member %}
  auto builder = ginx::FunctionTemplateBuilder(isolate, templ);
{###############################
 #
 # Static attributes
 #}
{% for attribute in attributes if attribute.is_static %}
{% if loop.first %}
  // static attributes
{% endif %}
{%   if attribute.is_read_only %}
  templ->SetAccessorProperty(gin::StringToSymbol(isolate, "{{attribute.name}}"),
      v8::FunctionTemplate::New(isolate, &{{class_name}}::Get_{{attribute.cc_name}}),
      v8::Local<v8::FunctionTemplate>(),
      static_cast<v8::PropertyAttribute>(v8::DontDelete | v8::DontEnum | v8::ReadOnly));
{%    else %}
  templ->SetAccessorProperty(gin::StringToSymbol(isolate, "{{attribute.name}}"),
      v8::FunctionTemplate::New(isolate, &{{class_name}}::Get_{{attribute.cc_name}}),
      v8::FunctionTemplate::New(isolate, &{{class_name}}::Set_{{attribute.cc_name}}),
      static_cast<v8::PropertyAttribute>(v8::DontDelete | v8::DontEnum));
{%    endif %}
{%  endfor %}
{###############################
 #
 # Static methods
 #}
{% for method in methods if method.is_static %}
{% if loop.first %}
  // static methods
{% endif %}
  templ->Set(gin::StringToSymbol(isolate, "{{method.name}}"),
      v8::FunctionTemplate::New(isolate, &{{class_name}}::{{method.cc_name}}),
      static_cast<v8::PropertyAttribute>(v8::DontDelete | v8::DontEnum | v8::ReadOnly));
{% endfor %}
  return builder.Build();
{% else %}
  return templ;
{% endif %}
}

{% endif %}
{% if need_instance_template %}
{##############################################################
 #
 # Interface template
 #}
v8::Local<v8::ObjectTemplate> {{class_name}}:: SetupInstanceTemplate(
      v8::Isolate* isolate,
      v8::Local<v8::ObjectTemplate> templ) {
{% for attribute in attributes if not attribute.is_static %}
{% if loop.first %}
  // attributes
{% endif %}
{%   if attribute.is_read_only %}
  templ->SetAccessorProperty(gin::StringToSymbol(isolate, "{{attribute.name}}"),
      v8::FunctionTemplate::New(isolate, &{{class_name}}::Get_{{attribute.cc_name}}),
      v8::Local<v8::FunctionTemplate>(),
      static_cast<v8::PropertyAttribute>(v8::DontDelete | v8::DontEnum | v8::ReadOnly));
{%   else %}
  templ->SetAccessorProperty(gin::StringToSymbol(isolate, "{{attribute.name}}"),
      v8::FunctionTemplate::New(isolate, &{{class_name}}::Get_{{attribute.cc_name}}),
      v8::FunctionTemplate::New(isolate, &{{class_name}}::Set_{{attribute.cc_name}}),
      static_cast<v8::PropertyAttribute>(v8::DontDelete | v8::DontEnum));
{%   endif %}
{%  endfor %}
{% for method in methods if not method.is_static %}
{% if loop.first %}
  // methods
{% endif %}
  templ->Set(gin::StringToSymbol(isolate, "{{method.name}}"),
      v8::FunctionTemplate::New(isolate, &{{class_name}}::{{method.cc_name}}),
      static_cast<v8::PropertyAttribute>(v8::DontDelete | v8::DontEnum | v8::ReadOnly));
{% endfor %}
  return templ;
}

{% endif %}
}  // namespace bindings

using namespace bindings;
DEFINE_SCRIPTABLE_OBJECT({{interface_name}}, {{class_name}});

}  // namespace dom

{% for dictionary in dictionaries %}
#include "./{{dictionary.name}}.cc"
{% endfor %}
