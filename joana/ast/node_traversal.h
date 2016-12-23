// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_NODE_TRAVERSAL_H_
#define JOANA_AST_NODE_TRAVERSAL_H_

#include <iterator>

#include "joana/ast/ast_export.h"

namespace joana {
namespace ast {

class ContainerNode;
class Node;
class NodeTraversal;

// |NodeAncestors| is a return value of |NodeTraversal::AncestorsOf()| and
// |NodeTraversal::InclusiveAncestorsOf()|.
class JOANA_AST_EXPORT NodeAncestors final {
 public:
  class JOANA_AST_EXPORT Iterator final
      : public std::iterator<std::input_iterator_tag, Node> {
   public:
    Iterator(const Iterator& other);
    ~Iterator();

    reference operator*() const;
    Iterator& operator++();

    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

   private:
    friend class NodeAncestors;

    Iterator(const NodeAncestors* owner, Node* node);

    Node* node_;
    const NodeAncestors* owner_;
  };

  NodeAncestors(const NodeAncestors& other);
  ~NodeAncestors();

  Iterator begin() const { return Iterator(this, node_); }
  Iterator end() const { return Iterator(this, nullptr); }

 private:
  friend class NodeTraversal;

  explicit NodeAncestors(const Node* node);

  Node* node_;
};

// |NodeChildren| is a return value of |NodeTraversal::ChildrenOf()| and
// |NodeTraversal::InclusiveChildrenOf()|.
class JOANA_AST_EXPORT NodeChildren final {
 public:
  class JOANA_AST_EXPORT Iterator final
      : public std::iterator<std::input_iterator_tag, Node> {
   public:
    Iterator(const Iterator& other);
    ~Iterator();

    reference operator*() const;
    Iterator& operator++();

    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

   private:
    friend class NodeChildren;

    Iterator(const NodeChildren* owner, Node* node);

    Node* node_;
    const NodeChildren* owner_;
  };

  NodeChildren(const NodeChildren& other);
  ~NodeChildren();

  Iterator begin() const;
  Iterator end() const { return Iterator(this, nullptr); }

 private:
  friend class NodeTraversal;

  explicit NodeChildren(const ContainerNode& container);

  ContainerNode* container_;
};

//
// NodeTraversal
//
class JOANA_AST_EXPORT NodeTraversal final {
 public:
  NodeTraversal() = delete;
  ~NodeTraversal() = delete;

  static NodeAncestors AncestorsOf(const Node& container);
  static Node& ChildAt(const ContainerNode& container, int index);
  static NodeChildren ChildrenOf(const ContainerNode& container);
  static int CountChildren(const ContainerNode& container);
  static Node* FirstChildOf(const ContainerNode& node);
  static NodeAncestors InclusiveAncestorsOf(const Node& node);
  static Node* LastChildOf(const ContainerNode& container);
  static Node* NextSiblingOf(const Node& node);
  static ContainerNode* ParentOf(const Node& node);
  static Node* PreviousSiblingOf(const Node& node);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_NODE_TRAVERSAL_H_