// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/converter.h"

#include "base/time/time.h"
#include "evita/text/buffer.h"
#include "evita/v8_glue/converter.h"

namespace gin {

// base::Time
v8::Handle<v8::Value> Converter<base::Time>::ToV8(v8::Isolate* isolate,
                                                  base::Time time) {
  return v8::Date::New(isolate, time.ToJsTime());
}

bool Converter<base::Time>::FromV8(v8::Isolate*,
                                   v8::Handle<v8::Value> val,
                                   base::Time* out) {
  if (!val->IsDate())
    return false;
  auto const date = v8::Handle<v8::Date>::Cast(val);
  *out = base::Time::FromJsTime(date->ValueOf());
  return true;
}

v8::Handle<v8::Value> Converter<text::LineAndColumn>::ToV8(
    v8::Isolate* isolate,
    const text::LineAndColumn& line_and_column) {
  auto const result = v8::Object::New(isolate);
  result->Set(gin::StringToV8(isolate, "column"),
              v8::Integer::New(isolate, line_and_column.column));
  result->Set(gin::StringToV8(isolate, "lineNumber"),
              v8::Integer::New(isolate, line_and_column.line_number));
  return result;
}

}  // namespace gin
