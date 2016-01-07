// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_BOX_TREE_H_
#define EVITA_VISUALS_LAYOUT_BOX_TREE_H_

#include <memory>

#include "base/macros.h"

namespace visuals {

class Box;
class Document;
class Node;
class RootBox;
class StyleTree;

//////////////////////////////////////////////////////////////////////
//
// BoxTree represents a CSS Box tree for document(node tree) with style tree.
//
class BoxTree final {
 public:
  BoxTree(const Document& document, const StyleTree& style_tree);
  ~BoxTree();

  RootBox* root_box() const;
  int version() const;

  Box* BoxFor(const Node& node) const;
  void UpdateIfNeeded();

 private:
  class Impl;

  std::unique_ptr<Impl> impl_;

  DISALLOW_COPY_AND_ASSIGN(BoxTree);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_BOX_TREE_H_