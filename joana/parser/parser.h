// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_PARSER_H_
#define JOANA_PARSER_PARSER_H_

#include <memory>

#include "base/macros.h"

namespace joana {
namespace ast {
class EditContext;
class Node;
}
class SourceCodeRange;

namespace internal {

class Lexer;

class Parser final {
 public:
  Parser(ast::EditContext* context, const SourceCodeRange& range);

  ~Parser();

  // Returns root node, either |Module| or |Script|, of abstract syntax tree
  // for source code specified in |range|. The result may be partial result
  // if source code does not match grammar.
  const ast::Node& Run();

 private:
  ast::EditContext* const context_;
  std::unique_ptr<Lexer> lexer_;
  ast::Node& root_;

  DISALLOW_COPY_AND_ASSIGN(Parser);
};

}  // namespace internal
}  // namespace joana

#endif  // JOANA_PARSER_PARSER_H_
