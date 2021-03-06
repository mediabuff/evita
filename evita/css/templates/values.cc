// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <ostream>

#include "evita/css/values.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"

namespace css {

{% for type in types if not type.is_primitive %}
//////////////////////////////////////////////////////////////////////
//
// CSS Value {{type.Name}}
//
{%  for member in type.members if not member.is_keyword %}
{{type.Name}}::{{type.Name}}({{member.Parameter}} {{member.name}})
    : value_(Value({{member.name}})) {}
{% endfor %}
{# Copy constructor #}
{{type.Name}}::{{type.Name}}({{type.Parameter}} other) :
    value_(other.value_) {}

{{type.Name}}::{{type.Name}}(const Value& value) : value_(value) {}
{# Default constructor #}
{{type.Name}}::{{type.Name}}() : value_({{type.initial}}) {}
{{type.Name}}::~{{type.Name}}() {}

{{type.Name}}& {{type.Name}}::operator=({{type.Parameter}} other) {
  value_ = other.value_;
  return *this;
}

bool {{type.Name}}::operator==({{type.Parameter}} other) const {
  if (this == &other)
    return true;
  return value_ == other.value_;
}

bool {{type.Name}}::operator!=({{type.Parameter}} other) const {
  return !operator==(other);
}

{#############################################################
 #
 # Getters
 #}
{%  for member in type.members if not member.is_keyword %}
{{member.Return}} {{type.Name}}::as_{{member.name}}() const {
  DCHECK(is_{{member.name}}()) << *this;
  return value_.as_{{member.name}}();
}

{%  endfor %}
{#############################################################
 #
 # Predicates
 #}
{% for member in type.members %}
bool {{type.Name}}::is_{{member.name}}() const {
{% if member.is_keyword %}
  return value_.is_keyword() && value_.as_keyword() == Keyword::{{member.Name}};
{% else %}
  return value_.is_{{member.name}}();
{% endif %}
}

{% endfor %}
{#############################################################
 #
 # Static Constructors
 #}
{% for member in type.members if member.is_keyword %}
// static
{{type.Name}} {{type.Name}}::{{member.Name}}() {
  return {{type.Name}}(Value(Keyword::{{member.Name}}));
}

{% endfor %}
{#############################################################
 #
 # Convert To Value
 #}
Value ToValue({{type.Parameter}} {{type.name}}) {
  return {{type.name}}.value();
}

{#############################################################
 #
 # Printer
 #}
// To make |css::Style| printer output simpler, we don't print type name
// in value printer.
std::ostream& operator<<(std::ostream& ostream,
                         {{type.Parameter}} {{type.name}}) {
  return ostream << {{type.name}}.value();
}

{% endfor %}

base::string16 KeywordToString16(Keyword keyword) {
  const base::char16* const texts[] = {
{% for keyword in keywords %}
  L"{{keyword.text}}",
{% endfor %}
  };
  const auto& it = std::begin(texts) + static_cast<size_t>(keyword);
  if (it < std::begin(texts) || it >= std::end(texts))
    return base::string16(L"???");
  return base::string16(*it);
}

std::ostream& operator<<(std::ostream& ostream, Keyword keyword) {
  return ostream << base::UTF16ToUTF8(KeywordToString16(keyword));
}

}  // namespace css

