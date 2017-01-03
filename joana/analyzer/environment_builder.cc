// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>

#include "joana/analyzer/environment_builder.h"

#include "joana/analyzer/context.h"
#include "joana/analyzer/environment.h"
#include "joana/analyzer/error_codes.h"
#include "joana/analyzer/factory.h"
#include "joana/analyzer/value.h"
#include "joana/analyzer/value_editor.h"
#include "joana/analyzer/value_forward.h"
#include "joana/analyzer/values.h"
#include "joana/ast/bindings.h"
#include "joana/ast/compilation_units.h"
#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/statements.h"
#include "joana/ast/syntax_forward.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace analyzer {

namespace {

const ast::Node& NameOf(const ast::Node& node) {
  if (node == ast::SyntaxCode::Name)
    return node;
  if (node == ast::SyntaxCode::BindingNameElement)
    return node.child_at(0);
  NOTREACHED() << node << " is not name node.";
  return node;
}

}  // namespace

//
// EnvironmentBuilder::LocalEnvironment
//
class EnvironmentBuilder::LocalEnvironment final {
 public:
  LocalEnvironment(EnvironmentBuilder* builder, const ast::Node& owner);
  ~LocalEnvironment();

  const LocalEnvironment* outer() const { return outer_; }
  LocalEnvironment* outer() { return outer_; }

  void Bind(const ast::Node& name, Value* value);
  Value* Find(const ast::Node& name) const;

 private:
  EnvironmentBuilder& builder_;
  LocalEnvironment* const outer_;
  const ast::Node& owner_;
  std::unordered_map<int, Value*> value_map_;

  DISALLOW_COPY_AND_ASSIGN(LocalEnvironment);
};

EnvironmentBuilder::LocalEnvironment::LocalEnvironment(
    EnvironmentBuilder* builder,
    const ast::Node& owner)
    : builder_(*builder), outer_(builder_.environment_), owner_(owner) {
  builder_.environment_ = this;
}

EnvironmentBuilder::LocalEnvironment::~LocalEnvironment() {
  builder_.environment_ = outer_;
}

void EnvironmentBuilder::LocalEnvironment::Bind(const ast::Node& name,
                                                Value* value) {
  const auto& result = value_map_.emplace(name.name_id(), value);
  DCHECK(result.second);
}

Value* EnvironmentBuilder::LocalEnvironment::Find(const ast::Node& name) const {
  const auto& it = value_map_.find(name.name_id());
  return it == value_map_.end() ? nullptr : it->second;
}

//
// EnvironmentBuilder
//
EnvironmentBuilder::EnvironmentBuilder(Context* context) : Pass(context) {}

EnvironmentBuilder::~EnvironmentBuilder() = default;

// The entry point. |node| is one of |ast::Externs|, |ast::Module| or
// |ast::Script|.
void EnvironmentBuilder::Load(const ast::Node& node) {
  auto& global_environment = factory().global_environment();
  toplevel_environment_ =
      node == ast::SyntaxCode::Module
          ? &factory().NewEnvironment(&global_environment, node)
          : &global_environment;
  Visit(node);
}

// Binding helpers
void EnvironmentBuilder::BindToFunction(const ast::Node& name,
                                        const ast::Node& declaration) {
  if (environment_) {
    if (auto* present = environment_->Find(name)) {
      AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS, present->node());
      return;
    }
    environment_->Bind(name, &factory().NewFunction(declaration));
    return;
  }

  if (auto* present = toplevel_environment_->BindingOf(name)) {
    AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS, present->node());
    return;
  }
  // TODO(eval1749): We should bind to |Constructor| if |declaration| has
  // "@constructor" annotation.
  toplevel_environment_->Bind(name, &factory().NewFunction(declaration));
}

void EnvironmentBuilder::BindToVariable(const ast::Node& origin,
                                        const ast::Node& name_node) {
  const auto& name = NameOf(name_node);
  if (environment_) {
    if (auto* present = environment_->Find(name)) {
      if (present->node() == ast::SyntaxCode::VarStatement &&
          origin == ast::SyntaxCode::VarStatement) {
        // TODO(eval1749): We should report error if |present| has type
        // annotation.
        Value::Editor().AddAsignment(&present->As<LexicalBinding>(), name_node);
        return;
      }
      AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS, present->node());
      return;
    }
    environment_->Bind(name, &factory().NewVariable(origin, name_node));
    return;
  }
  if (auto* present = toplevel_environment_->BindingOf(name)) {
    if (present->node() == ast::SyntaxCode::VarStatement &&
        origin == ast::SyntaxCode::VarStatement) {
      // TODO(eval1749): We should report error if |present| has type
      // annotation.
      Value::Editor().AddAsignment(&present->As<LexicalBinding>(), name_node);
      return;
    }
    AddError(name, ErrorCode::ENVIRONMENT_MULTIPLE_BINDINGS, present->node());
    return;
  }
  // TODO(eval1749): Expose global "var" binding to global object.
  toplevel_environment_->Bind(name, &factory().NewVariable(origin, name_node));
}

// AST node handlers
void EnvironmentBuilder::ProcessVariables(const ast::Node& statement) {
  variable_origin_ = &statement;
  for (const auto& element : ast::NodeTraversal::ChildNodesOf(statement))
    Visit(element);
}

void EnvironmentBuilder::Visit(const ast::Node& node) {
#if DCHECK_IS_ON()
  const auto result = visited_nodes_.emplace(&node);
  DCHECK(result.second) << "Visited twice " << node;
#endif
  switch (node.syntax().opcode()) {
    // Declarations
    case ast::SyntaxCode::Class:
      return VisitClass(node);
    case ast::SyntaxCode::Function:
      return VisitFunction(node);
    // Expressions
    case ast::SyntaxCode::ReferenceExpression:
      return VisitReferenceExpression(node);
    // Statements
    case ast::SyntaxCode::BlockStatement:
      return VisitBlockStatement(node);
    case ast::SyntaxCode::ConstStatement:
    case ast::SyntaxCode::LetStatement:
    case ast::SyntaxCode::VarStatement:
      return ProcessVariables(node);
  }
  VisitChildNodes(node);
}

void EnvironmentBuilder::VisitChildNodes(const ast::Node& node) {
  for (const auto& child : ast::NodeTraversal::ChildNodesOf(node))
    Visit(child);
}

//
// ast::NodeVisitor members
//

// Binding elements
void EnvironmentBuilder::VisitBindingNameElement(const ast::Node& node) {
  BindToVariable(*variable_origin_, node);
}

// Declarations
void EnvironmentBuilder::VisitClass(const ast::Node& node) {
  // TODO(eval1749): Report warning for toplevel anonymous class
  const auto& name = ast::ClassSyntax::NameOf(node);
  if (name == ast::SyntaxCode::Name)
    BindToFunction(name, node);
  VisitChildNodes(node);
}

void EnvironmentBuilder::VisitFunction(const ast::Node& node) {
  // TODO(eval1749): Report warning for toplevel anonymous class
  const auto& name = ast::FunctionSyntax::NameOf(node);
  if (name == ast::SyntaxCode::Name)
    BindToFunction(name, node);
  LocalEnvironment environment(this, node);
  variable_origin_ = &node;
  VisitChildNodes(node);
}

// Expressions
void EnvironmentBuilder::VisitReferenceExpression(const ast::Node& node) {
  const auto& name = ast::ReferenceExpressionSyntax::NameOf(node);
  for (auto* runner = environment_; runner; runner = runner->outer()) {
    if (auto* present = environment_->Find(name)) {
      factory().RegisterValue(node, present);
      return;
    }
  }
  auto* present = toplevel_environment_->BindingOf(name);
  if (!present)
    return;
  factory().RegisterValue(node, present);
}

// Statements
void EnvironmentBuilder::VisitBlockStatement(const ast::Node& node) {
  LocalEnvironment environment(this, node);
  VisitChildNodes(node);
}

}  // namespace analyzer
}  // namespace joana
