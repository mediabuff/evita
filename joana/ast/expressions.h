// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_EXPRESSIONS_H_
#define JOANA_AST_EXPRESSIONS_H_

#include <vector>

#include "joana/ast/container_node.h"
#include "joana/ast/node_forward.h"
#include "joana/ast/tokens.h"
#include "joana/base/iterator_utils.h"

namespace joana {
namespace ast {

//
// Expression is a base class of expression nodes.
//
class JOANA_AST_EXPORT Expression : public ContainerNode {
  DECLARE_ABSTRACT_AST_NODE(Expression, ContainerNode);

 public:
  ~Expression() override;

 protected:
  explicit Expression(const SourceCodeRange& range);

 private:
  DISALLOW_COPY_AND_ASSIGN(Expression);
};

//
// ExpressionList
//
class JOANA_AST_EXPORT ExpressionList final : public ZoneAllocated {
 public:
  ~ExpressionList();

  auto begin() const { return ReferenceRangeOf(expressions_).begin(); }
  bool empty() const { return expressions_.empty(); }
  auto end() const { return ReferenceRangeOf(expressions_).end(); }
  size_t size() const { return expressions_.size(); }

 private:
  friend class NodeFactory;

  ExpressionList(Zone* zone, const std::vector<Expression*>& expressions);

  ZoneVector<Expression*> expressions_;

  DISALLOW_COPY_AND_ASSIGN(ExpressionList);
};

//
// ArrayLiteralExpression
//
class JOANA_AST_EXPORT ArrayLiteralExpression final
    : public NodeTemplate<Expression, const ExpressionList*> {
  DECLARE_CONCRETE_AST_NODE(ArrayLiteralExpression, Expression);

 public:
  ~ArrayLiteralExpression() final;

  const ExpressionList& elements() const { return *member_at<0>(); }

 private:
  ArrayLiteralExpression(const SourceCodeRange& range,
                         const ExpressionList& elements);

  DISALLOW_COPY_AND_ASSIGN(ArrayLiteralExpression);
};

//
// AssignmentExpression
//
class JOANA_AST_EXPORT AssignmentExpression final
    : public NodeTemplate<Expression,
                          const Punctuator*,
                          const Expression*,
                          const Expression*> {
  DECLARE_CONCRETE_AST_NODE(AssignmentExpression, Expression);

 public:
  ~AssignmentExpression() final;

  const Expression& lhs() const { return *member_at<1>(); }
  const Punctuator& op() const { return *member_at<0>(); }
  const Expression& rhs() const { return *member_at<2>(); }

 private:
  AssignmentExpression(const SourceCodeRange& range,
                       const Punctuator& op,
                       const Expression& lhs,
                       const Expression& rhs);

  DISALLOW_COPY_AND_ASSIGN(AssignmentExpression);
};

//
// BinaryExpression
//
class JOANA_AST_EXPORT BinaryExpression final
    : public NodeTemplate<Expression, Token*, Expression*, Expression*> {
  DECLARE_CONCRETE_AST_NODE(BinaryExpression, Expression);

 public:
  ~BinaryExpression() final;

  const Expression& lhs() const { return *member_at<1>(); }
  const Token& op() const { return *member_at<0>(); }
  const Expression& rhs() const { return *member_at<2>(); }

 private:
  BinaryExpression(const SourceCodeRange& range,
                   Token* op,
                   Expression* lhs,
                   Expression* rhs);

  DISALLOW_COPY_AND_ASSIGN(BinaryExpression);
};

//
// CallExpression
//
class JOANA_AST_EXPORT CallExpression final
    : public NodeTemplate<Expression, Expression*, ExpressionList*> {
  DECLARE_CONCRETE_AST_NODE(CallExpression, Expression);

 public:
  ~CallExpression() final;

  const ExpressionList& arguments() const { return *member_at<1>(); }
  const Expression& callee() const { return *member_at<0>(); }

 private:
  CallExpression(const SourceCodeRange& range,
                 Expression* callee,
                 ExpressionList* arguments);

  DISALLOW_COPY_AND_ASSIGN(CallExpression);
};

//
// CommaExpression
//
class JOANA_AST_EXPORT CommaExpression final
    : public NodeTemplate<Expression, ExpressionList*> {
  DECLARE_CONCRETE_AST_NODE(CommaExpression, Expression);

 public:
  ~CommaExpression() final;

  const ExpressionList& expressions() const { return *member_at<0>(); }

 private:
  CommaExpression(const SourceCodeRange& range, ExpressionList* expressions);

  DISALLOW_COPY_AND_ASSIGN(CommaExpression);
};

//
// ComputedMemberExpression represents:
//  Expression '[' Expression ']'
//
class JOANA_AST_EXPORT ComputedMemberExpression
    : public NodeTemplate<Expression, Expression*, Expression*> {
  DECLARE_CONCRETE_AST_NODE(ComputedMemberExpression, Expression);

 public:
  ~ComputedMemberExpression() final;

  const Expression& expression() const { return *member_at<0>(); }
  const Expression& name_expression() const { return *member_at<1>(); }

 private:
  ComputedMemberExpression(const SourceCodeRange& range,
                           Expression* expression,
                           Expression* name_Expression);

  DISALLOW_COPY_AND_ASSIGN(ComputedMemberExpression);
};

//
// ConditionalExpression
//
class JOANA_AST_EXPORT ConditionalExpression final
    : public NodeTemplate<Expression, Expression*, Expression*, Expression*> {
  DECLARE_CONCRETE_AST_NODE(ConditionalExpression, Expression);

 public:
  ~ConditionalExpression() final;

  const Expression& condition() const { return *member_at<0>(); }
  const Expression& false_expression() const { return *member_at<2>(); }
  const Expression& true_expression() const { return *member_at<1>(); }

 private:
  ConditionalExpression(const SourceCodeRange& range,
                        Expression* condition,
                        Expression* true_expression,
                        Expression* false_expression);

  DISALLOW_COPY_AND_ASSIGN(ConditionalExpression);
};

//
// DeclarationExpression
//
class JOANA_AST_EXPORT DeclarationExpression final
    : public NodeTemplate<Expression, Declaration*> {
  DECLARE_CONCRETE_AST_NODE(DeclarationExpression, Expression);

 public:
  ~DeclarationExpression() final;

  const Declaration& declaration() const { return *member_at<0>(); }

 private:
  explicit DeclarationExpression(Declaration* declaration);

  DISALLOW_COPY_AND_ASSIGN(DeclarationExpression);
};

//
// ElisionExpression represents '[' elision ',' ']' syntax for array literal.
//
class JOANA_AST_EXPORT ElisionExpression final : public Expression {
  DECLARE_CONCRETE_AST_NODE(ElisionExpression, Expression);

 public:
  ~ElisionExpression() final;

 private:
  explicit ElisionExpression(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(ElisionExpression);
};

//
// EmptyExpression represents '(' ')' for function parameter list.
//
class JOANA_AST_EXPORT EmptyExpression final : public Expression {
  DECLARE_CONCRETE_AST_NODE(EmptyExpression, Expression);

 public:
  ~EmptyExpression() final;

 private:
  explicit EmptyExpression(const SourceCodeRange& range);

  DISALLOW_COPY_AND_ASSIGN(EmptyExpression);
};

//
// GroupExpression represents '(' expression ')' syntax.
//
class JOANA_AST_EXPORT GroupExpression final
    : public NodeTemplate<Expression, Expression*> {
  DECLARE_CONCRETE_AST_NODE(GroupExpression, Expression);

 public:
  ~GroupExpression() final;

  const Expression& expression() const { return *member_at<0>(); }

 private:
  GroupExpression(const SourceCodeRange& range, Expression* expression);

  DISALLOW_COPY_AND_ASSIGN(GroupExpression);
};

//
// InvalidExpression
//
class JOANA_AST_EXPORT InvalidExpression : public Expression {
  DECLARE_CONCRETE_AST_NODE(InvalidExpression, Expression);

 public:
  ~InvalidExpression() final;

  int error_code() const { return error_code_; }

 private:
  InvalidExpression(const Node& node, int error_code);

  const int error_code_;

  DISALLOW_COPY_AND_ASSIGN(InvalidExpression);
};

//
// LiteralExpression
//
class JOANA_AST_EXPORT LiteralExpression final
    : public NodeTemplate<Expression, Literal*> {
  DECLARE_CONCRETE_AST_NODE(LiteralExpression, Expression);

 public:
  ~LiteralExpression() final;

  const Literal& literal() const { return *member_at<0>(); }

 private:
  explicit LiteralExpression(Literal* literal);

  DISALLOW_COPY_AND_ASSIGN(LiteralExpression);
};

//
// NewExpression
//
class JOANA_AST_EXPORT NewExpression final
    : public NodeTemplate<Expression, Expression*, ExpressionList*> {
  DECLARE_CONCRETE_AST_NODE(NewExpression, Expression);

 public:
  ~NewExpression() final;

  const ExpressionList& arguments() const { return *member_at<1>(); }
  const Expression& expression() const { return *member_at<0>(); }

 private:
  NewExpression(const SourceCodeRange& range,
                Expression* expression,
                ExpressionList* arguments);

  DISALLOW_COPY_AND_ASSIGN(NewExpression);
};

//
// ObjectLiteralExpression
//
class JOANA_AST_EXPORT ObjectLiteralExpression final
    : public NodeTemplate<Expression, ExpressionList*> {
  DECLARE_CONCRETE_AST_NODE(ObjectLiteralExpression, Expression);

 public:
  ~ObjectLiteralExpression() final;

  const ExpressionList& members() const { return *member_at<0>(); }

 private:
  ObjectLiteralExpression(const SourceCodeRange& range,
                          ExpressionList* members);

  DISALLOW_COPY_AND_ASSIGN(ObjectLiteralExpression);
};

//
// MemberExpression
//
class JOANA_AST_EXPORT MemberExpression final
    : public NodeTemplate<Expression, Expression*, Name*> {
  DECLARE_CONCRETE_AST_NODE(MemberExpression, Expression);

 public:
  ~MemberExpression() final;

  const Expression& expression() const { return *member_at<0>(); }
  const Name& name() const { return *member_at<1>(); }

 private:
  MemberExpression(const SourceCodeRange& range,
                   Expression* expression,
                   Name* name);

  DISALLOW_COPY_AND_ASSIGN(MemberExpression);
};

//
// PropertyDefinitionExpression
//
class JOANA_AST_EXPORT PropertyDefinitionExpression final
    : public NodeTemplate<Expression, Expression*, Expression*> {
  DECLARE_CONCRETE_AST_NODE(PropertyDefinitionExpression, Expression);

 public:
  ~PropertyDefinitionExpression() final;

  const Expression& name() const { return *member_at<0>(); }
  const Expression& value() const { return *member_at<1>(); }

 private:
  PropertyDefinitionExpression(const SourceCodeRange& range,
                               Expression* name,
                               Expression* value);

  DISALLOW_COPY_AND_ASSIGN(PropertyDefinitionExpression);
};

//
// ReferenceExpression
//
class JOANA_AST_EXPORT ReferenceExpression final
    : public NodeTemplate<Expression, Name*> {
  DECLARE_CONCRETE_AST_NODE(ReferenceExpression, Expression);

 public:
  ~ReferenceExpression() final;

  const Name& name() const { return *member_at<0>(); }

 private:
  explicit ReferenceExpression(Name* name);

  DISALLOW_COPY_AND_ASSIGN(ReferenceExpression);
};

//
// RegExpLiteralExpression
//
class JOANA_AST_EXPORT RegExpLiteralExpression final
    : public NodeTemplate<Expression, RegExp*, Token*> {
  DECLARE_CONCRETE_AST_NODE(RegExpLiteralExpression, Expression);

 public:
  ~RegExpLiteralExpression() final;

  const Token& flags() const { return *member_at<1>(); }
  const RegExp& pattern() const { return *member_at<0>(); }

 private:
  RegExpLiteralExpression(const SourceCodeRange& range,
                          RegExp* pattern,
                          Token* flags);

  DISALLOW_COPY_AND_ASSIGN(RegExpLiteralExpression);
};

//
// UnaryExpression
//
class JOANA_AST_EXPORT UnaryExpression final
    : public NodeTemplate<Expression, Token*, Expression*> {
  DECLARE_CONCRETE_AST_NODE(UnaryExpression, Expression);

 public:
  ~UnaryExpression() final;

  const Expression& expression() const { return *member_at<1>(); }
  const Token& op() const { return *member_at<0>(); }

 private:
  UnaryExpression(const SourceCodeRange& range,
                  Token* op,
                  Expression* expression);

  DISALLOW_COPY_AND_ASSIGN(UnaryExpression);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_EXPRESSIONS_H_