// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <ostream>
#include <type_traits>

#include "base/logging.h"
#include "joana/analyzer/types.h"
#include "joana/analyzer/value_forward.h"
#include "joana/analyzer/values.h"
#include "joana/ast/declarations.h"
#include "joana/ast/node_printer.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace analyzer {

namespace {

const ast::Node* ValueNameOf(const Value& value) {
  const auto& node = value.node();
  if (value.Is<Class>()) {
    const auto& name = node.child_at(0);
    return name.Is<ast::Name>() ? &name : nullptr;
  }
  if (value.Is<Function>()) {
    if (node.Is<ast::Function>()) {
      const auto& name = ast::Function::NameOf(node);
      return name.Is<ast::Name>() ? &name : nullptr;
    }
    if (node.Is<ast::Method>()) {
      return &ast::Method::NameOf(node);
    }
    return nullptr;
  }
  if (value.Is<Property>())
    return &node;
  if (value.Is<Variable>())
    return &node;
  return nullptr;
}

//
// Printable
//
template <typename T>
struct Printable {
  static_assert(std::is_base_of<Value, T>::value, "Should be Value");
  const T* value;
};

Printable<Value> AsPrintable(const Value& value) {
  return Printable<Value>{&value};
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<ConstructedClass>& printable) {
  const auto& value = *printable.value;
  ostream << value.kind();
  const auto* delimiter = "<";
  for (const auto& argument : value.arguments()) {
    ostream << delimiter << argument;
    delimiter = ",";
  }
  if (*delimiter == ',')
    ostream << '>';
  return ostream << '@' << value.id() << ']';
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<Function>& printable) {
  const auto& value = *printable.value;
  ostream << "Function";
  const auto* name = ValueNameOf(value);
  if (!name)
    return ostream << '@' << value.id();
  return ostream << '[' << ast::AsSourceCode(*name) << '@' << value.id() << ']';
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<GenericClass>& printable) {
  const auto& value = *printable.value;
  ostream << value.kind() << '[';
  if (const auto* name = ValueNameOf(value))
    ostream << ast::AsSourceCode(*name);
  const auto* delimiter = "<";
  for (const auto& parameter : value.parameters()) {
    ostream << delimiter << ast::AsSourceCode(parameter.name());
    delimiter = ",";
  }
  if (*delimiter == ',')
    ostream << '>';
  return ostream << '@' << value.id() << ']';
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<NormalClass>& printable) {
  const auto& value = *printable.value;
  ostream << value.kind() << '[';
  if (const auto* name = ValueNameOf(value))
    ostream << ast::AsSourceCode(*name);
  else
    ostream << "%anonymous%";
  return ostream << '@' << value.id() << ']';
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<OrdinaryObject>& printable) {
  const auto& value = *printable.value;
  return ostream << "$OrdinaryObject@" << value.id() << ' ' << value.node();
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<Property>& printable) {
  const auto& value = *printable.value;
  return ostream << value.visibility() << "Property["
                 << ast::AsSourceCode(value.key()) << '@' << value.id() << ']';
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<Undefined>& printable) {
  const auto& value = *printable.value;
  return ostream << "Undefined@" << value.id() << ' ' << value.node();
}

std::ostream& operator<<(std::ostream& ostream,
                         const Printable<Variable>& printable) {
  const auto& value = *printable.value;
  return ostream << value.kind() << "Var["
                 << ast::AsSourceCode(*ValueNameOf(value)) << '@' << value.id()
                 << ']';
}

// Dispatcher
std::ostream& operator<<(std::ostream& ostream,
                         const Printable<Value>& printable) {
  const auto& value = *printable.value;
#define V(name)                            \
  if (auto* derived = value.TryAs<name>()) \
    return ostream << Printable<name>{derived};
  FOR_EACH_ANALYZE_VALUE(V)
#undef V

  NOTREACHED() << "Need operator<< for " << value.class_name();
  return ostream;
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream, ClassKind kind) {
  switch (kind) {
    case ClassKind::Class:
      return ostream << "Class";
    case ClassKind::Interface:
      return ostream << "Interface";
    case ClassKind::Record:
      return ostream << "Record";
  }
  return ostream << "ClassKind" << static_cast<int>(kind);
}

std::ostream& operator<<(std::ostream& ostream, const Value& value) {
  return ostream << AsPrintable(value);
}

std::ostream& operator<<(std::ostream& ostream, const Value* value) {
  if (!value)
    return ostream << "(null)";
  return ostream << *value;
}

std::ostream& operator<<(std::ostream& ostream, VariableKind kind) {
  static const char* kTexts[] = {
#define V(name) #name,
      FOR_EACH_VARIABLE_KIND(V)
#undef V
  };
  const auto& it = std::begin(kTexts) + static_cast<size_t>(kind);
  if (it < std::begin(kTexts) || it >= std::end(kTexts))
    return ostream << "VariableKind" << static_cast<size_t>(kind);
  return ostream << *it;
}

std::ostream& operator<<(std::ostream& ostream, Visibility kind) {
  static const char* kTexts[] = {
#define V(name) #name,
      FOR_EACH_VISIBILITY(V)
#undef V
  };
  const auto& it = std::begin(kTexts) + static_cast<size_t>(kind);
  if (it < std::begin(kTexts) || it >= std::end(kTexts))
    return ostream << "Visibility" << static_cast<size_t>(kind);
  return ostream << *it;
}

}  // namespace analyzer
}  // namespace joana
