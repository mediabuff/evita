// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_DECLARATIONS_H_
#define JOANA_PUBLIC_AST_DECLARATIONS_H_

#include "joana/public/ast/node.h"
#include "joana/public/ast/node_forward.h"

namespace joana {
namespace ast {

class ExpressionList;
class Statement;

//
// FunctionKind
//
enum class FunctionKind {
  Invalid,
  Async,
  Generator,
  Normal,
};

//
// Declaration is a base class of declaration nodes.
//
class JOANA_PUBLIC_EXPORT Declaration : public Node {
  DECLARE_ABSTRACT_AST_NODE(Declaration, Node);

 public:
  ~Declaration() override;

 protected:
  explicit Declaration(const SourceCodeRange& range);

 private:
  DISALLOW_COPY_AND_ASSIGN(Declaration);
};

//
// ArrowFunction
//
// The parameter list is represented by |ast::Expression| with:
//  - () = EmptyExpression
//  - x = ReferenceExpression
//  - (a, b, ...) = GroupExpression with CommaExpression
//
class ArrowFunction final : public NodeTemplate<Declaration,
                                                FunctionKind,
                                                Expression*,
                                                ArrowFunctionBody*> {
  DECLARE_CONCRETE_AST_NODE(ArrowFunction, Declaration);

 public:
  ~ArrowFunction() final;

  const ArrowFunctionBody& body() const { return *member_at<2>(); }
  FunctionKind kind() const { return member_at<0>(); }
  const Expression& parameter_list() const { return *member_at<1>(); }

 private:
  // |statement| should be either expression statement or block statement.
  ArrowFunction(const SourceCodeRange& range,
                FunctionKind kind,
                Expression* parameter_list,
                ArrowFunctionBody* body);

  DISALLOW_COPY_AND_ASSIGN(ArrowFunction);
};

//
// Function
//
// The parameter list is represented by |ast::Expression| with:
//  - () = EmptyExpression
//  - (a, b, ...) = GroupExpression with CommaExpression
//
class Function final : public NodeTemplate<Declaration,
                                           FunctionKind,
                                           Token*,
                                           Expression*,
                                           Statement*> {
  DECLARE_CONCRETE_AST_NODE(Function, Declaration);

 public:
  ~Function() final;

  const Statement& body() const { return *member_at<3>(); }
  FunctionKind kind() const { return member_at<0>(); }
  const Token& name() const { return *member_at<1>(); }
  const Expression& parameter_list() const { return *member_at<2>(); }

 private:
  // |statement| should be either expression statement or block statement.
  Function(const SourceCodeRange& range,
           FunctionKind kind,
           Token* name,
           Expression* parameter_list,
           Statement* body);

  DISALLOW_COPY_AND_ASSIGN(Function);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_DECLARATIONS_H_
