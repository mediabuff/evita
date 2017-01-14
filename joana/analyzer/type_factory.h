// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_ANALYZER_TYPE_FACTORY_H_
#define JOANA_ANALYZER_TYPE_FACTORY_H_

#include <memory>
#include <vector>

#include "base/macros.h"

namespace joana {

namespace ast {
class Node;
enum class TokenKind;
}

class Zone;

namespace analyzer {

class Class;
enum class FunctionTypeKind;
class Type;
class TypeParameter;
class Value;
class Variable;

//
// TypeFactory
//
class TypeFactory final {
 public:
  explicit TypeFactory(Zone* zone);
  ~TypeFactory();

  const Type& GetAnyType() { return any_type_; }
  const Type& GetOrNewClassType(Class* class_value);
  const Type& NewFunctionType(
      FunctionTypeKind kind,
      const std::vector<const TypeParameter*>& parameters,
      const std::vector<const Type*>& parameter_types,
      const Type& return_type,
      const Type& this_type);
  const Type& GetInvalidType() const { return invalid_type_; }
  const Type& GetPrimitiveType(const ast::TokenKind id);
  const Type& NewTypeName(const ast::Node& name);
  const Type& NewTypeParameter(const ast::Node& name);
  const Type& GetUnknownType() const { return unknown_type_; }
  const Type& GetVoidType() const { return void_type_; }

 private:
  class Cache;

  void InstallPrimitiveTypes();
  int NextTypeId();
  const Type& NewClassType(Class* class_value);

  std::unique_ptr<Cache> cache_;
  int current_type_id_ = 0;
  Zone& zone_;

  const Type& any_type_;
  const Type& invalid_type_;
  const Type& unknown_type_;
  const Type& void_type_;

  DISALLOW_COPY_AND_ASSIGN(TypeFactory);
};

}  // namespace analyzer
}  // namespace joana

#endif  // JOANA_ANALYZER_TYPE_FACTORY_H_