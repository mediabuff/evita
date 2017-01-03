// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/ast/node.h"

#include "base/files/file_path.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/syntax.h"
#include "joana/ast/tokens.h"
#include "joana/base/escaped_string_piece.h"
#include "joana/base/source_code.h"

namespace joana {
namespace ast {

//
// Node
//
Node::Node(const SourceCodeRange& range, const Syntax& syntax, size_t arity)
    : arity_(arity), range_(range), syntax_(syntax) {
  if (syntax_.is_variadic()) {
    DCHECK_GE(arity_, syntax.arity());
    return;
  }
  DCHECK_EQ(arity_, syntax.arity());
}

Node::~Node() = default;

bool Node::operator==(const Node& other) const {
  return this == &other;
}

bool Node::operator==(const Node* other) const {
  return this == other;
}

bool Node::operator!=(const Node& other) const {
  return this != &other;
}

bool Node::operator!=(const Node* other) const {
  return this != other;
}

bool Node::operator==(TokenKind kind) const {
  const auto* name = syntax_.TryAs<Token>();
  return name && name->number() == static_cast<int>(kind);
}

bool Node::operator!=(TokenKind kind) const {
  return !operator==(kind);
}

bool Node::operator==(SyntaxCode syntax_code) const {
  return syntax_ == syntax_code;
}

bool Node::operator!=(SyntaxCode syntax_code) const {
  return !operator==(syntax_code);
}

const Node& Node::child_at(size_t index) const {
  DCHECK_LT(index, arity()) << *this;
  return *nodes_[index];
}

bool Node::is_literal() const {
  return syntax_.is_literal();
}

int Node::name_id() const {
  return syntax_.As<Name>().number();
}

std::ostream& operator<<(std::ostream& ostream, const Node& node) {
  const auto& range = node.range();
  return ostream << node.syntax() << '(' << range << ", "
                 << EscapedStringPiece16(range.GetString(), '|', 20) << ')';
}

std::ostream& operator<<(std::ostream& ostream, const Node* node) {
  if (!node)
    return ostream << "(null)";
  return ostream << *node;
}
}  // namespace ast
}  // namespace joana
