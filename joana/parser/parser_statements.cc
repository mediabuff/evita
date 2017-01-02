// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <vector>

#include "joana/parser/parser.h"

#include "joana/ast/declarations.h"
#include "joana/ast/node.h"
#include "joana/ast/node_factory.h"
#include "joana/ast/tokens.h"
#include "joana/base/source_code.h"
#include "joana/parser/lexer/lexer.h"
#include "joana/parser/parser_error_codes.h"

namespace joana {
namespace parser {

namespace {

bool CanHaveJsDoc(const ast::Node& statement) {
  if (statement == ast::SyntaxCode::Class)
    return true;
  if (statement == ast::SyntaxCode::ConstStatement)
    return true;
  if (statement == ast::SyntaxCode::Function)
    return true;
  if (statement == ast::SyntaxCode::LetStatement)
    return true;
  if (statement == ast::SyntaxCode::VarStatement)
    return true;
  if (statement != ast::SyntaxCode::ExpressionStatement)
    return false;
  auto& expression = statement.child_at(0);
  if (expression == ast::SyntaxCode::AssignmentExpression)
    return true;
  if (expression == ast::SyntaxCode::ReferenceExpression)
    return true;
  if (expression == ast::SyntaxCode::MemberExpression)
    return true;
  return false;
}

bool IsDeclarationKeyword(const ast::Node& name) {
  if (name != ast::SyntaxCode::Name)
    return false;
  return name == ast::NameId::Const || name == ast::NameId::Let ||
         name == ast::NameId::Var;
}

}  // namespace

//
// Functions for parsing statements
//

// Called after before consuming ':'
const ast::Node& Parser::HandleLabeledStatement(const ast::Node& label) {
  auto& colon = ConsumeToken();
  DCHECK_EQ(colon, ast::PunctuatorKind::Colon);
  if (!options_.disable_automatic_semicolon()) {
    if (!CanPeekToken() || PeekToken() == ast::PunctuatorKind::RightBrace) {
      auto& statement =
          NewEmptyStatement(SourceCodeRange::CollapseToEnd(colon.range()));
      return node_factory().NewLabeledStatement(GetSourceCodeRange(), label,
                                                statement);
    }
  }
  auto& statement = ParseStatement();
  return node_factory().NewLabeledStatement(GetSourceCodeRange(), label,
                                            statement);
}

const ast::Node& Parser::NewEmptyStatement(const SourceCodeRange& range) {
  DCHECK(range.IsCollapsed()) << range;
  return node_factory().NewEmptyStatement(range);
}

const ast::Node& Parser::NewInvalidStatement(ErrorCode error_code) {
  AddError(GetSourceCodeRange(), error_code);
  return node_factory().NewInvalidStatement(GetSourceCodeRange(),
                                            static_cast<int>(error_code));
}

const ast::Node& Parser::ParseJsDocAsStatement() {
  NodeRangeScope scope(this);
  const auto& jsdoc = ConsumeToken();
  const auto& statement = ParseStatement();
  if (statement == ast::SyntaxCode::Annotation) {
    // We don't allow statement/expression has more than one annotation.
    AddError(jsdoc, ErrorCode::ERROR_STATEMENT_UNEXPECT_ANNOTATION);
  }
  if (!CanHaveJsDoc(statement))
    AddError(jsdoc, ErrorCode::ERROR_STATEMENT_UNEXPECT_ANNOTATION);
  return node_factory().NewAnnotation(GetSourceCodeRange(), jsdoc, statement);
}

const ast::Node& Parser::ParseBlockStatement() {
  NodeRangeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::PunctuatorKind::LeftBrace);
  ConsumeToken();
  std::vector<const ast::Node*> statements;
  while (CanPeekToken()) {
    if (ConsumeTokenIf(ast::PunctuatorKind::RightBrace))
      break;
    if (ConsumeTokenIf(ast::SyntaxCode::Comment))
      continue;
    statements.push_back(&ParseStatement());
  }
  return node_factory().NewBlockStatement(GetSourceCodeRange(), statements);
}

const ast::Node& Parser::ParseBreakStatement() {
  ConsumeToken();
  if (is_separated_by_newline_) {
    if (options_.disable_automatic_semicolon())
      AddError(ErrorCode::ERROR_STATEMENT_UNEXPECT_NEWLINE);
    return node_factory().NewBreakStatement(GetSourceCodeRange(),
                                            NewEmptyName());
  }
  auto& label = CanPeekToken() && PeekToken() == ast::SyntaxCode::Name
                    ? ConsumeToken()
                    : NewEmptyName();
  ExpectSemicolon();
  return node_factory().NewBreakStatement(GetSourceCodeRange(), label);
}

const ast::Node& Parser::ParseCaseClause() {
  NodeRangeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::NameId::Case);
  ConsumeToken();
  auto& expression = ParseExpression();
  if (!CanPeekToken() || PeekToken() != ast::PunctuatorKind::Colon) {
    AddError(ErrorCode::ERROR_STATEMENT_EXPECT_COLON);
  } else {
    auto& colon = ConsumeToken();
    if (CanPeekToken() && PeekToken() == ast::PunctuatorKind::RightBrace) {
      if (options_.disable_automatic_semicolon())
        AddError(ErrorCode::ERROR_STATEMENT_EXPECT_SEMICOLON);
      return node_factory().NewCaseClause(
          GetSourceCodeRange(), expression,
          NewEmptyStatement(SourceCodeRange::CollapseToEnd(colon.range())));
    }
  }
  auto& statement = ParseStatement();
  return node_factory().NewCaseClause(GetSourceCodeRange(), expression,
                                      statement);
}

const ast::Node& Parser::ParseConstStatement() {
  DCHECK_EQ(PeekToken(), ast::NameId::Const);
  ConsumeToken();
  const auto& elements = ParseBindingElements();
  ExpectSemicolon();
  return node_factory().NewConstStatement(GetSourceCodeRange(), elements);
}

const ast::Node& Parser::ParseContinueStatement() {
  ConsumeToken();
  if (is_separated_by_newline_) {
    if (options_.disable_automatic_semicolon())
      AddError(ErrorCode::ERROR_STATEMENT_UNEXPECT_NEWLINE);
    return node_factory().NewContinueStatement(GetSourceCodeRange(),
                                               NewEmptyName());
  }
  auto& label = CanPeekToken() && PeekToken() == ast::SyntaxCode::Name
                    ? ConsumeToken()
                    : NewEmptyName();
  ExpectSemicolon();
  return node_factory().NewContinueStatement(GetSourceCodeRange(), label);
}

const ast::Node& Parser::ParseDefaultLabel() {
  NodeRangeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::NameId::Default);
  auto& label = ConsumeToken();
  if (!CanPeekToken() || PeekToken() != ast::PunctuatorKind::Colon)
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_EXPECT_COLON);
  return HandleLabeledStatement(label);
}

const ast::Node& Parser::ParseDoStatement() {
  DCHECK_EQ(PeekToken(), ast::NameId::Do);
  ConsumeToken();
  auto& statement = ParseStatement();
  if (!ConsumeTokenIf(ast::NameId::While))
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_EXPECT_WHILE);
  if (options_.disable_automatic_semicolon()) {
    auto& condition = ParseParenthesisExpression();
    ExpectSemicolon();
    return node_factory().NewDoStatement(GetSourceCodeRange(), statement,
                                         condition);
  }
  auto& condition = ParseParenthesisExpression();
  ConsumeTokenIf(ast::PunctuatorKind::Semicolon);
  return node_factory().NewDoStatement(GetSourceCodeRange(), statement,
                                       condition);
}

const ast::Node& Parser::ParseExpressionStatement() {
  auto& expression = ParseExpression();
  if (expression == ast::SyntaxCode::Class ||
      expression == ast::SyntaxCode::Function) {
    return expression;
  }
  ExpectSemicolon();
  return node_factory().NewExpressionStatement(GetSourceCodeRange(),
                                               expression);
}

const ast::Node& Parser::ParseForStatement() {
  DCHECK_EQ(PeekToken(), ast::NameId::For);
  ConsumeToken();
  ExpectPunctuator(ast::PunctuatorKind::LeftParenthesis,
                   ErrorCode::ERROR_STATEMENT_EXPECT_LPAREN);

  auto* const jsdoc =
      CanPeekToken() && PeekToken() == ast::SyntaxCode::JsDocDocument
          ? &ConsumeToken()
          : nullptr;

  auto& keyword = CanPeekToken() && IsDeclarationKeyword(PeekToken())
                      ? ConsumeToken()
                      : NewEmptyName();

  auto& expression =
      CanPeekToken() && PeekToken() == ast::PunctuatorKind::Semicolon
          ? NewElisionExpression()
          : ParseExpression();

  if (jsdoc) {
    if (keyword == ast::SyntaxCode::Empty)
      AddError(*jsdoc, ErrorCode::ERROR_STATEMENT_UNEXPECT_ANNOTATION);
  }

  if (ConsumeTokenIf(ast::PunctuatorKind::Semicolon)) {
    // 'for' '(' binding ';' condition ';' step ')' statement
    auto& condition =
        CanPeekToken() && PeekToken() == ast::PunctuatorKind::Semicolon
            ? NewElisionExpression()
            : ParseExpression();
    ExpectPunctuator(ast::PunctuatorKind::Semicolon,
                     ErrorCode::ERROR_STATEMENT_EXPECT_SEMICOLON);
    auto& step =
        CanPeekToken() && PeekToken() == ast::PunctuatorKind::RightParenthesis
            ? NewElisionExpression()
            : ParseExpression();
    ExpectPunctuator(ast::PunctuatorKind::RightParenthesis,
                     ErrorCode::ERROR_STATEMENT_EXPECT_RPAREN);
    auto& body = ParseStatement();
    return node_factory().NewForStatement(GetSourceCodeRange(), keyword,
                                          expression, condition, step, body);
  }

  if (ConsumeTokenIf(ast::NameId::Of)) {
    // 'for' '(' binding 'of' expression ')' statement
    auto& expression2 = ParseAssignmentExpression();
    ExpectPunctuator(ast::PunctuatorKind::RightParenthesis,
                     ErrorCode::ERROR_STATEMENT_EXPECT_RPAREN);
    auto& body = ParseStatement();
    return node_factory().NewForOfStatement(GetSourceCodeRange(), keyword,
                                            expression, expression2, body);
  }

  if (ConsumeTokenIf(ast::PunctuatorKind::RightParenthesis)) {
    // 'for' '(' binding 'in' expression ')' statement
    auto& body = ParseStatement();
    return node_factory().NewForInStatement(GetSourceCodeRange(), keyword,
                                            expression, body);
  }

  return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_INVALID);
}

const ast::Node& Parser::ParseIfStatement() {
  auto& keyword = ConsumeToken();
  DCHECK_EQ(keyword, ast::NameId::If);
  auto& condition = ParseParenthesisExpression();
  auto& then_clause = ParseStatement();
  if (!ConsumeTokenIf(ast::NameId::Else)) {
    return node_factory().NewIfStatement(GetSourceCodeRange(), condition,
                                         then_clause);
  }
  auto& else_clause = ParseStatement();
  return node_factory().NewIfElseStatement(GetSourceCodeRange(), condition,
                                           then_clause, else_clause);
}

const ast::Node& Parser::ParseKeywordStatement() {
  NodeRangeScope scope(this);
  const auto& keyword = PeekToken();
  DCHECK(ast::NameSyntax::IsKeyword(keyword)) << keyword;
  switch (static_cast<ast::NameId>(keyword.name_id())) {
    case ast::NameId::Async:
      ConsumeToken();
      if (CanPeekToken() && PeekToken() == ast::NameId::Function)
        return ParseFunction(ast::FunctionKind::Async);
      PushBackToken(keyword);
      break;
    case ast::NameId::Break:
      return ParseBreakStatement();
    case ast::NameId::Case:
      return ParseCaseClause();
    case ast::NameId::Class:
      return ParseClass();
    case ast::NameId::Continue:
      return ParseContinueStatement();
    case ast::NameId::Const:
      return ParseConstStatement();
    case ast::NameId::Debugger: {
      auto& expression = node_factory().NewReferenceExpression(ConsumeToken());
      ExpectSemicolon();
      return node_factory().NewExpressionStatement(GetSourceCodeRange(),
                                                   expression);
    }
    case ast::NameId::Default:
      return ParseDefaultLabel();
    case ast::NameId::Do:
      return ParseDoStatement();
    case ast::NameId::For:
      return ParseForStatement();
    case ast::NameId::Function:
      ConsumeToken();
      if (ConsumeTokenIf(ast::PunctuatorKind::Times))
        return ParseFunction(ast::FunctionKind::Generator);
      return ParseFunction(ast::FunctionKind::Normal);
    case ast::NameId::If:
      return ParseIfStatement();
    case ast::NameId::Let:
      return ParseLetStatement();
    case ast::NameId::Return:
      return ParseReturnStatement();
    case ast::NameId::Switch:
      return ParseSwitchStatement();
    case ast::NameId::Throw:
      return ParseThrowStatement();
    case ast::NameId::Try:
      return ParseTryStatement();
    case ast::NameId::Var:
      return ParseVarStatement();
    case ast::NameId::While:
      return ParseWhileStatement();

    case ast::NameId::With:
      return ParseWithStatement();

    // Reserved keywords
    case ast::NameId::Catch:
    case ast::NameId::Else:
    case ast::NameId::Enum:
    case ast::NameId::Finally:
    case ast::NameId::Interface:
    case ast::NameId::Package:
    case ast::NameId::Private:
    case ast::NameId::Protected:
    case ast::NameId::Public:
    case ast::NameId::Static:
      ConsumeToken();
      return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_RESERVED_WORD);
  }
  return ParseExpressionStatement();
}

const ast::Node& Parser::ParseLetStatement() {
  DCHECK_EQ(PeekToken(), ast::NameId::Let);
  ConsumeToken();
  const auto& elements = ParseBindingElements();
  ExpectSemicolon();
  return node_factory().NewLetStatement(GetSourceCodeRange(), elements);
}

const ast::Node& Parser::ParseNameAsStatement() {
  auto& name = PeekToken();
  if (ast::NameSyntax::IsKeyword(name))
    return ParseKeywordStatement();
  ConsumeToken();
  if (!CanPeekToken()) {
    PushBackToken(name);
    return ParseExpressionStatement();
  }
  if (is_separated_by_newline_) {
    if (options_.disable_automatic_semicolon())
      AddError(ErrorCode::ERROR_STATEMENT_UNEXPECT_NEWLINE);
  } else {
    if (CanPeekToken() && PeekToken() == ast::PunctuatorKind::Colon)
      return HandleLabeledStatement(name);
  }
  PushBackToken(name);
  return ParseExpressionStatement();
}

// Yet another entry point called by statement parser.
const ast::Node& Parser::ParseParenthesisExpression() {
  if (!ConsumeTokenIf(ast::PunctuatorKind::LeftParenthesis)) {
    // Some people think it is redundant that C++ statement requires
    // parenthesis for an expression after keyword.
    AddError(ErrorCode::ERROR_STATEMENT_EXPECT_LPAREN);
    return ParseExpression();
  }
  auto& expression = ParseExpression();
  ExpectPunctuator(ast::PunctuatorKind::RightParenthesis,
                   ErrorCode::ERROR_STATEMENT_EXPECT_RPAREN);
  return expression;
}

const ast::Node& Parser::ParseReturnStatement() {
  ConsumeToken();
  if (is_separated_by_newline_) {
    if (options_.disable_automatic_semicolon())
      AddError(ErrorCode::ERROR_STATEMENT_UNEXPECT_NEWLINE);
    return node_factory().NewReturnStatement(GetSourceCodeRange(),
                                             NewElisionExpression());
  } else {
    if (CanPeekToken() && PeekToken() == ast::PunctuatorKind::RightBrace) {
      return node_factory().NewReturnStatement(GetSourceCodeRange(),
                                               NewElisionExpression());
    }
  }
  if (CanPeekToken() && ConsumeTokenIf(ast::PunctuatorKind::Semicolon)) {
    return node_factory().NewReturnStatement(GetSourceCodeRange(),
                                             NewElisionExpression());
  }
  auto& expression = ParseExpression();
  ExpectSemicolon();
  return node_factory().NewReturnStatement(GetSourceCodeRange(), expression);
}

// The entry point
const ast::Node& Parser::ParseStatement() {
  if (!CanPeekToken())
    return NewEmptyStatement(source_code().end());
  NodeRangeScope scope(this);
  const auto& token = PeekToken();
  if (token == ast::SyntaxCode::JsDocDocument)
    return ParseJsDocAsStatement();
  if (token == ast::SyntaxCode::Name)
    return ParseNameAsStatement();
  if (token.is_literal())
    return ParseExpressionStatement();
  if (token == ast::PunctuatorKind::LeftBrace)
    return ParseBlockStatement();
  if (token == ast::PunctuatorKind::Semicolon) {
    auto& statement = NewEmptyStatement(
        SourceCodeRange::CollapseToStart(PeekToken().range()));
    ConsumeToken();
    return statement;
  }
  return ParseExpressionStatement();
}

const ast::Node& Parser::ParseSwitchStatement() {
  DCHECK_EQ(PeekToken(), ast::NameId::Switch);
  ConsumeToken();
  auto& expression = ParseParenthesisExpression();
  std::vector<const ast::Node*> clauses;
  if (!ConsumeTokenIf(ast::PunctuatorKind::LeftBrace)) {
    AddError(ErrorCode::ERROR_STATEMENT_EXPECT_LBRACE);
    return node_factory().NewSwitchStatement(GetSourceCodeRange(), expression,
                                             clauses);
  }
  while (CanPeekToken()) {
    if (ConsumeTokenIf(ast::PunctuatorKind::RightBrace))
      break;
    clauses.push_back(&ParseStatement());
  }
  return node_factory().NewSwitchStatement(GetSourceCodeRange(), expression,
                                           clauses);
}

const ast::Node& Parser::ParseThrowStatement() {
  DCHECK_EQ(PeekToken(), ast::NameId::Throw);
  ConsumeToken();
  auto& expression = ParseExpression();
  ExpectSemicolon();
  return node_factory().NewThrowStatement(GetSourceCodeRange(), expression);
}

const ast::Node& Parser::ParseTryStatement() {
  ConsumeToken();
  auto& try_block = ParseStatement();
  if (ConsumeTokenIf(ast::NameId::Finally)) {
    auto& finally_block = ParseBlockStatement();
    return node_factory().NewTryFinallyStatement(GetSourceCodeRange(),
                                                 try_block, finally_block);
  }
  if (!ConsumeTokenIf(ast::NameId::Catch))
    return NewInvalidStatement(ErrorCode::ERROR_STATEMENT_EXPECT_CATCH);
  auto& catch_parameter = ParseExpression();
  auto& catch_block = ParseStatement();
  if (!ConsumeTokenIf(ast::NameId::Finally)) {
    return node_factory().NewTryCatchStatement(GetSourceCodeRange(), try_block,
                                               catch_parameter, catch_block);
  }
  auto& finally_block = ParseStatement();
  return node_factory().NewTryCatchFinallyStatement(GetSourceCodeRange(),
                                                    try_block, catch_parameter,
                                                    catch_block, finally_block);
}

const ast::Node& Parser::ParseVarStatement() {
  DCHECK_EQ(PeekToken(), ast::NameId::Var);
  ConsumeToken();
  const auto& elements = ParseBindingElements();
  ExpectSemicolon();
  return node_factory().NewVarStatement(GetSourceCodeRange(), elements);
}

const ast::Node& Parser::ParseWhileStatement() {
  DCHECK_EQ(PeekToken(), ast::NameId::While);
  ConsumeToken();
  auto& condition = ParseParenthesisExpression();
  auto& statement = ParseStatement();
  return node_factory().NewWhileStatement(GetSourceCodeRange(), condition,
                                          statement);
}

const ast::Node& Parser::ParseWithStatement() {
  DCHECK_EQ(PeekToken(), ast::NameId::With);
  ConsumeToken();
  auto& expression = ParseParenthesisExpression();
  auto& statement = ParseStatement();
  return node_factory().NewWithStatement(GetSourceCodeRange(), expression,
                                         statement);
}

}  // namespace parser
}  // namespace joana
