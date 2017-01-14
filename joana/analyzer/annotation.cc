// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <unordered_map>
#include <utility>

#include "joana/analyzer/annotation.h"

#include "base/logging.h"
#include "joana/analyzer/context.h"
#include "joana/analyzer/error_codes.h"
#include "joana/analyzer/type_factory.h"
#include "joana/analyzer/type_name_resolver.h"
#include "joana/analyzer/types.h"
#include "joana/ast/bindings.h"
#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/jsdoc_syntaxes.h"
#include "joana/ast/node.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/statements.h"
#include "joana/ast/tokens.h"
#include "joana/ast/types.h"

namespace joana {
namespace analyzer {

namespace {

const ast::Node& AnnotatedNodeOf(const ast::Node& node) {
  if (node.Is<ast::AssignmentExpression>())
    return AnnotatedNodeOf(ast::AssignmentExpression::RightHandSideOf(node));
  if (node.Is<ast::ComputedMemberExpression>())
    return node;
  if (node.Is<ast::Function>())
    return node;
  if (node.Is<ast::MemberExpression>())
    return node;
  if (node.Is<ast::Method>())
    return node;
  if (node.Is<ast::ReferenceExpression>())
    return node;
  return node;
}

bool IsPrototypeProperty(const ast::Node& node) {
  if (!node.Is(ast::SyntaxCode::MemberExpression))
    return false;
  return ast::MemberExpression::NameOf(node) == ast::TokenKind::Prototype;
}

bool IsMethod(const ast::Node& node) {
  if (node.Is(ast::SyntaxCode::AssignmentExpression)) {
    if (!ast::AssignmentExpression::RightHandSideOf(node).Is(
            ast::SyntaxCode::Function))
      return false;
    return IsMethod(ast::AssignmentExpression::LeftHandSideOf(node));
  }
  if (node.Is(ast::SyntaxCode::ComputedMemberExpression)) {
    return IsPrototypeProperty(
        ast::ComputedMemberExpression::MemberExpressionOf(node));
  }
  if (node.Is(ast::SyntaxCode::MemberExpression))
    return IsPrototypeProperty(ast::MemberExpression::ExpressionOf(node));
  if (node.Is(ast::SyntaxCode::ExpressionStatement))
    return IsMethod(ast::ExpressionStatement::ExpressionOf(node));
  if (node.Is(ast::SyntaxCode::Method))
    return true;
  return false;
}

// Returns member part of computed member expression or member expression.
const ast::Node* MemberExpressionOf(const ast::Node& node) {
  if (node.Is(ast::SyntaxCode::ComputedMemberExpression))
    return &ast::ComputedMemberExpression::MemberExpressionOf(node);
  if (node.Is(ast::SyntaxCode::MemberExpression))
    return &ast::MemberExpression::ExpressionOf(node);
  return nullptr;
}

}  // namespace

//
// Annotation
//
Annotation::Annotation(Context* context,
                       TypeNameResolver* resolver,
                       const Type* this_type,
                       const ast::Node& node)
    : ContextUser(context),
      node_(node),
      resolver_(*resolver),
      this_type_(this_type) {
  DCHECK_EQ(node_, ast::SyntaxCode::Annotation);
  DCHECK(resolver);
}

Annotation::~Annotation() = default;

const ast::Node& Annotation::kind() const {
  DCHECK(kind_tag_);
  return ast::JsDocTag::NameOf(*kind_tag_);
}

const Type& Annotation::type() const {
  DCHECK(has_type());
  return *type_;
}

const ast::Node* Annotation::Classify() {
  for (const auto& node :
       ast::NodeTraversal::ChildNodesOf(ast::Annotation::DocumentOf(node_))) {
    if (node != ast::SyntaxCode::JsDocTag)
      continue;
    const auto& tag_name = ast::JsDocTag::NameOf(node);
    switch (ast::Name::KindOf(tag_name)) {
      case ast::TokenKind::AtConst:
        RememberTag(&const_tag_, node);
        if (node.arity() == 0)
          continue;
        RememberTag(&kind_tag_, node);
        if (type_node_)
          type_node_ = &node.child_at(1);
        continue;
      case ast::TokenKind::AtConstructor:
      case ast::TokenKind::AtDict:
      case ast::TokenKind::AtEnum:
      case ast::TokenKind::AtInterface:
      case ast::TokenKind::AtRecord:
        RememberTag(&kind_tag_, node);
        continue;
      case ast::TokenKind::AtDefine:
      case ast::TokenKind::AtType:
      case ast::TokenKind::AtTypeDef:
        RememberTag(&kind_tag_, node);
        if (!type_node_)
          type_node_ = &node.child_at(1);
        continue;
      case ast::TokenKind::AtExtends:
        RememberTag(&extends_tag_, node);
        continue;
      case ast::TokenKind::AtFinal:
        RememberTag(&final_tag_, node);
        continue;
      case ast::TokenKind::AtImplements:
        RememberTag(&implements_tag_, node);
        continue;
      case ast::TokenKind::AtOverride:
        RememberTag(&override_tag_, node);
        continue;
      case ast::TokenKind::AtPrivate:
      case ast::TokenKind::AtProtected:
      case ast::TokenKind::AtPublic:
        RememberTag(&access_tag_, node);
        continue;
      case ast::TokenKind::AtParam:
        parameter_tags_.push_back(&node);
        continue;
      case ast::TokenKind::AtReturn:
        RememberTag(&return_tag_, node);
        continue;
      case ast::TokenKind::AtThis:
        RememberTag(&this_tag_, node);
        continue;
      case ast::TokenKind::AtTemplate:
        if (!template_tag_) {
          for (const auto& name : ast::JsDocTag::OperandsOf(node)) {
            if (!name.Is<ast::Name>()) {
              AddError(name, ErrorCode::JSDOC_EXPECT_NAME);
              continue;
            }
            const auto name_id = ast::Name::KindOf(name);
            const auto& result = type_parameter_map_.emplace(
                name_id, &type_factory().NewTypeParameter(name));
            if (result.second)
              continue;
            for (const auto& present : ast::JsDocTag::OperandsOf(node)) {
              if (present != name_id)
                continue;
              AddError(name, ErrorCode::JSDOC_MULTIPLE_NAME, present);
              break;
            }
          }
        }
        RememberTag(&template_tag_, node);
        continue;
    }
  }
  return kind_tag_ ? &kind() : nullptr;
}

void Annotation::Compile() {
  const auto* kind = Classify();
  if (kind == nullptr) {
    if (!parameter_tags_.empty() || return_tag_) {
      type_ = &ToGenericTypeIfNeeded(TransformAsFunctionType());
      return;
    }
    return MarkNotTypeAnnotation();
  }
  if (*kind == ast::TokenKind::AtConstructor) {
    type_ = &ToGenericTypeIfNeeded(TransformAsFunctionType());
    return;
  }
  if (*kind == ast::TokenKind::AtDict) {
    MarkNotTypeAnnotation();
    return;
  }
  if (*kind == ast::TokenKind::AtEnum) {
    // TODO(eval1749): NYI: enum type.
    MarkNotTypeAnnotation();
    return;
  }
  if (*kind == ast::TokenKind::AtInterface ||
      *kind == ast::TokenKind::AtRecord) {
    type_ = &ToGenericTypeIfNeeded(TransformAsInterface());
    return;
  }
  if (type_node_) {
    type_ = &ToGenericTypeIfNeeded(TransformType(*type_node_));
    return;
  }
  MarkNotTypeAnnotation();
}

const Type& Annotation::ComputeReturnType() {
  if (!return_tag_)
    return type_factory().GetVoidType();
  return TransformType(return_tag_->child_at(1));
}

// Type of "this" is determined by:
// - Class passed by caller during processing class declaration.
// - Class.prototype.Name;
// - Class.prototype[Expression];
const Type& Annotation::ComputeThisType() {
  const auto& annotated = ast::Annotation::AnnotatedOf(node_);
  if (this_type_)
    return *this_type_;

  if (!annotated.Is<ast::ExpressionStatement>())
    return type_factory().GetVoidType();

  const auto& expression = ast::ExpressionStatement::ExpressionOf(annotated);
  const auto& node = expression.Is<ast::AssignmentExpression>()
                         ? ast::AssignmentExpression::LeftHandSideOf(expression)
                         : expression;
  if (node.Is<ast::MemberExpression>())
    return ComputeThisTypeFromMember(ast::MemberExpression::ExpressionOf(node));
  if (node.Is<ast::ComputedMemberExpression>()) {
    return ComputeThisTypeFromMember(
        ast::ComputedMemberExpression::MemberExpressionOf(node));
  }
  return type_factory().GetVoidType();
}

// Returns type of |Expression| where |Expression| '.' 'prototype'.
const Type& Annotation::ComputeThisTypeFromMember(const ast::Node& node) {
  if (!IsPrototypeProperty(node))
    return type_factory().GetVoidType();
  if (const auto* type = context().TryTypeOf(node))
    return *type;
  AddError(node, ErrorCode::JSDOC_EXPECT_TYPE);
  return type_factory().GetVoidType();
}

bool Annotation::IsAnnotated(const ast::Node& node) const {
  return ast::Annotation::AnnotatedOf(node_) == node;
}

void Annotation::MarkNotTypeAnnotation() {
  if (access_tag_)
    AddError(*access_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
  if (extends_tag_)
    AddError(*extends_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
  if (final_tag_)
    AddError(*final_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
  if (implements_tag_)
    AddError(*implements_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
  if (override_tag_ && IsMethod(ast::Annotation::AnnotatedOf(node_)))
    AddError(*override_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
  if (!parameter_tags_.empty()) {
    for (const auto& parameter_tag : parameter_tags_)
      AddError(*parameter_tag, ErrorCode::JSDOC_UNEXPECT_TAG);
  }
  if (return_tag_)
    AddError(*return_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
  if (template_tag_)
    AddError(*template_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
  if (this_tag_)
    AddError(*this_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
}

void Annotation::ProcessParameter(
    std::vector<const ast::Node*>* parameter_names,
    std::vector<const Type*>* parameter_types,
    const ast::Node& parameter_node) {
  if (parameter_node.Is<ast::BindingNameElement>()) {
    const auto& name = ast::BindingNameElement::NameOf(parameter_node);
    const auto name_id = ast::Name::KindOf(name);

    // Check multiple occurrence of parameter name
    for (const auto& present : *parameter_names) {
      if (name_id == ast::Name::KindOf(*present)) {
        AddError(name, ErrorCode::JSDOC_MULTIPLE_PARAMETER, *present);
        return;
      }
    }

    // Find associated @param tag.
    for (const auto& parameter_tag : parameter_tags_) {
      if (parameter_tag->child_at(2) == name_id) {
        parameter_names->push_back(&name);
        parameter_types->push_back(&TransformType(parameter_tag->child_at(0)));
        return;
      }
    }

    AddError(name, ErrorCode::JSDOC_EXPECT_TYPE);
    parameter_names->push_back(&name);
    parameter_types->push_back(&type_factory().GetUnknownType());
    return;
  }
  NOTREACHED() << "NYI ProcessParameter " << parameter_node;
}

std::vector<const Type*> Annotation::ProcessParameterList(
    const ast::Node& parameter_list) {
  DCHECK_EQ(parameter_list, ast::SyntaxCode::ParameterList);
  std::vector<const ast::Node*> parameter_names;
  std::vector<const Type*> parameter_types;

  // Compute parameter name list and parameter type list
  for (const auto& parameter_node :
       ast::NodeTraversal::ChildNodesOf(parameter_list)) {
    ProcessParameter(&parameter_names, &parameter_types, parameter_node);
  }

  // Check not associated @param
  for (const auto& parameter_tag : parameter_tags_) {
    const auto& it = std::find_if(
        parameter_names.begin(), parameter_names.end(),
        [&](const ast::Node* present) {
          return *present == ast::Name::KindOf(parameter_tag->child_at(2));
        });
    if (it != parameter_names.end())
      continue;
    AddError(*parameter_tag, ErrorCode::JSDOC_UNEXPECT_PARAMETER);
  }
  return std::move(parameter_types);
}

void Annotation::RememberTag(const ast::Node** pointer, const ast::Node& node) {
  if (*pointer) {
    AddError(node, ErrorCode::JSDOC_MULTIPLE_TAG, **pointer);
    return;
  }
  *pointer = &node;
}

const Type& Annotation::ResolveTypeName(const ast::Node& name) {
  DCHECK_EQ(name, ast::SyntaxCode::Name);
  const auto& it = type_parameter_map_.find(ast::Name::KindOf(name));
  if (it != type_parameter_map_.end())
    return *it->second;
  return resolver_.ResolveTypeName(name);
}

// Transform @param, @return style function type to Type object.
const Type& Annotation::TransformAsFunctionType() {
  DCHECK(!parameter_tags_.empty() || !return_tag_);

  const auto& annotated = AnnotatedNodeOf(ast::Annotation::AnnotatedOf(node_));

  if (annotated.Is<ast::Method>()) {
    const auto& this_type = ComputeThisType();
    DCHECK(this_type.Is<ClassType>()) << this_type;
    if (kind_tag_)
      AddError(*kind_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
    const auto& parameter_types =
        ProcessParameterList(ast::Method::ParametersOf(annotated));
    const auto method_kind = ast::Method::MethodKindOf(annotated);
    switch (method_kind) {
      case ast::MethodKind::Constructor:
        if (return_tag_)
          AddError(*return_tag_, ErrorCode::JSDOC_UNEXPECT_RETURN);
        return type_factory().NewFunctionType(FunctionTypeKind::Constructor,
                                              parameter_types, this_type,
                                              this_type);
      case ast::MethodKind::NonStatic:
        return type_factory().NewFunctionType(FunctionTypeKind::Normal,
                                              parameter_types,
                                              ComputeReturnType(), this_type);
      case ast::MethodKind::Static:
        return type_factory().NewFunctionType(
            FunctionTypeKind::Normal, parameter_types, ComputeReturnType(),
            type_factory().GetVoidType());
    }
    NOTREACHED() << "Unknown MethodKind " << static_cast<int>(method_kind);
    return type_factory().GetVoidType();
  }

  // Short hand of single parameter or no parameter function, e.g.
  // /**
  //  * @param {number} x
  //  * var foo;
  //  */
  // is equivalent to:
  // /**
  //  * @param {number} x
  //  * var foo = function(x) {};
  //  */
  const auto& parameter_types =
      annotated.Is<ast::Function>()
          ? ProcessParameterList(ast::Function::ParametersOf(annotated))
          : parameter_tags_.empty()
                ? std::vector<const Type*>()
                : std::vector<const Type*>{
                      &TransformType(parameter_tags_.front()->child_at(1))};

  if (!annotated.Is<ast::Function>()) {
    auto is_first = true;
    for (const auto& parameter_tag : parameter_tags_) {
      if (is_first) {
        is_first = false;
        continue;
      }
      AddError(*parameter_tag, ErrorCode::JSDOC_UNEXPECT_PARAMETER);
    }
  }

  if (!kind_tag_) {
    return type_factory().NewFunctionType(FunctionTypeKind::Normal,
                                          parameter_types, ComputeReturnType(),
                                          ComputeThisType());
  }

  if (kind() == ast::TokenKind::Constructor) {
    if (return_tag_)
      AddError(*return_tag_, ErrorCode::JSDOC_UNEXPECT_RETURN);
    const auto& class_type = ComputeThisType();
    return type_factory().NewFunctionType(
        FunctionTypeKind::Constructor, parameter_types, class_type, class_type);
  }

  AddError(*kind_tag_, ErrorCode::JSDOC_UNEXPECT_TAG);
  return type_factory().GetVoidType();
}

const Type& Annotation::TransformType(const ast::Node& node) {
  if (node.Is<ast::TypeName>())
    return ResolveTypeName(ast::TypeName::NameOf(node));
  return type_factory().GetUnknownType();
}

}  // namespace analyzer
}  // namespace joana