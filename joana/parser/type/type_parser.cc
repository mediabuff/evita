// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stack>
#include <utility>

#include "joana/parser/type/type_parser.h"

#include "joana/ast/declarations.h"
#include "joana/ast/node_factory.h"
#include "joana/ast/tokens.h"
#include "joana/ast/types.h"
#include "joana/base/error_sink.h"
#include "joana/base/source_code.h"
#include "joana/base/source_code_range.h"
#include "joana/parser/public/parser_context.h"
#include "joana/parser/type/type_error_codes.h"
#include "joana/parser/type/type_lexer.h"
#include "joana/parser/utils/bracket_tracker.h"

namespace joana {
namespace parser {

using Name = ast::Name;
using Token = ast::Token;
using Type = ast::Type;

namespace {

bool CanStartType(const Token& token) {
  return token.Is<Name>() || token == ast::PunctuatorKind::LeftParenthesis ||
         token == ast::PunctuatorKind::LeftBracket ||
         token == ast::PunctuatorKind::LeftBrace;
}

std::unique_ptr<BracketTracker> NewBracketTracker(
    ErrorSink* error_sink,
    const SourceCodeRange& source_code_range) {
  const auto descriptions = std::vector<BracketTracker::Description>{
      {ast::PunctuatorKind::LeftParenthesis,
       static_cast<int>(TypeErrorCode::ERROR_TYPE_EXPECT_RPAREN),
       ast::PunctuatorKind::RightParenthesis,
       static_cast<int>(TypeErrorCode::ERROR_TYPE_UNEXPECT_RPAREN)},
      {ast::PunctuatorKind::LeftBrace,
       static_cast<int>(TypeErrorCode::ERROR_TYPE_EXPECT_RBRACE),
       ast::PunctuatorKind::RightBrace,
       static_cast<int>(TypeErrorCode::ERROR_TYPE_UNEXPECT_RBRACE)},
      {ast::PunctuatorKind::LeftBracket,
       static_cast<int>(TypeErrorCode::ERROR_TYPE_EXPECT_RBRACKET),
       ast::PunctuatorKind::RightBracket,
       static_cast<int>(TypeErrorCode::ERROR_TYPE_UNEXPECT_RBRACKET)},
      {ast::PunctuatorKind::LessThan,
       static_cast<int>(TypeErrorCode::ERROR_TYPE_EXPECT_RANGLE),
       ast::PunctuatorKind::GreaterThan,
       static_cast<int>(TypeErrorCode::ERROR_TYPE_UNEXPECT_RANGLE)},
  };

  return std::make_unique<BracketTracker>(error_sink, source_code_range,
                                          descriptions);
}

}  // namespace

//
// TypeParser::TypeNodeScope
//
class TypeParser::TypeNodeScope final {
 public:
  explicit TypeNodeScope(TypeParser* parser)
      : parser_(*parser), node_start_(parser->node_start_) {
    parser_.node_start_ = parser_.PeekToken().range().start();
  }

  ~TypeNodeScope() { parser_.node_start_ = node_start_; }

 private:
  TypeParser& parser_;
  const int node_start_;

  DISALLOW_COPY_AND_ASSIGN(TypeNodeScope);
};

//
// TypeParser
//
TypeParser::TypeParser(ParserContext* context,
                       const SourceCodeRange& range,
                       const ParserOptions& options,
                       TypeLexerMode mode)
    : bracket_tracker_(NewBracketTracker(&context->error_sink(), range)),
      context_(*context),
      lexer_(new TypeLexer(context, range, options, mode)),
      node_start_(range.start()),
      options_(options) {}

TypeParser::TypeParser(ParserContext* context,
                       const SourceCodeRange& range,
                       const ParserOptions& options)
    : TypeParser(context, range, options, TypeLexerMode::Normal) {}

TypeParser::~TypeParser() = default;

ast::NodeFactory& TypeParser::node_factory() {
  return context_.node_factory();
}

const SourceCode& TypeParser::source_code() const {
  return lexer_->source_code();
}

void TypeParser::AddError(int start, int end, TypeErrorCode error_code) {
  AddError(source_code().Slice(start, end), error_code);
}

void TypeParser::AddError(const SourceCodeRange& range,
                          TypeErrorCode error_code) {
  context_.error_sink().AddError(range, static_cast<int>(error_code));
}

void TypeParser::AddError(const Token& token, TypeErrorCode error_code) {
  AddError(token.range(), error_code);
}

void TypeParser::AddError(TypeErrorCode error_code) {
  if (CanPeekToken())
    return AddError(PeekToken().range(), error_code);
  AddError(lexer_->location(), error_code);
}

bool TypeParser::CanPeekToken() const {
  return lexer_->CanPeekToken();
}

const Token& TypeParser::ConsumeToken() {
  auto& token = lexer_->ConsumeToken();
  bracket_tracker_->Feed(token);
  return token;
}

const Token& TypeParser::PeekToken() const {
  return lexer_->PeekToken();
}

void TypeParser::SkipTokensTo(ast::PunctuatorKind kind) {
  while (CanPeekToken() && !ConsumeTokenIf(kind))
    ConsumeToken();
}

// Factory members
SourceCodeRange TypeParser::ComputeNodeRange() const {
  if (!CanPeekToken())
    return source_code().Slice(node_start_, lexer_->range().end());
  return source_code().Slice(node_start_, PeekToken().range().end());
}

const Type& TypeParser::NewAnyType() {
  return node_factory().NewAnyType(ComputeNodeRange());
}

const Type& TypeParser::NewFunctionType(
    ast::FunctionTypeKind kind,
    const std::vector<const Type*>& parameters,
    const Type& return_type) {
  return node_factory().NewFunctionType(ComputeNodeRange(), kind, parameters,
                                        return_type);
}

const Type& TypeParser::NewInvalidType() {
  return node_factory().NewInvalidType(ComputeNodeRange());
}

const Type& TypeParser::NewNullableType(const Type& type) {
  return node_factory().NewNullableType(ComputeNodeRange(), type);
}

const Type& TypeParser::NewNonNullableType(const Type& type) {
  return node_factory().NewNonNullableType(ComputeNodeRange(), type);
}

const Type& TypeParser::NewOptionalType(const Type& type) {
  return node_factory().NewOptionalType(ComputeNodeRange(), type);
}

const Type& TypeParser::NewRecordType(
    const std::vector<RecordMember>& members) {
  return node_factory().NewRecordType(ComputeNodeRange(), members);
}

const Type& TypeParser::NewRestType(const Type& type) {
  return node_factory().NewRestType(ComputeNodeRange(), type);
}

const Type& TypeParser::NewTupleType(const std::vector<const Type*>& members) {
  return node_factory().NewTupleType(ComputeNodeRange(), members);
}

const Type& TypeParser::NewTypeApplication(
    const Name& name,
    const std::vector<const Type*>& types) {
  return node_factory().NewTypeApplication(ComputeNodeRange(), name, types);
}

const Type& TypeParser::NewTypeGroup(const Type& type) {
  return node_factory().NewTypeGroup(ComputeNodeRange(), type);
}

const Type& TypeParser::NewTypeName(const Name& name) {
  return node_factory().NewTypeName(name.range(), name);
}

const Type& TypeParser::NewUnionType(const std::vector<const Type*>& members) {
  DCHECK(!members.empty());
  if (members.size() == 1)
    return *members.front();
  return node_factory().NewUnionType(ComputeNodeRange(), members);
}

const Type& TypeParser::NewUnknownType() {
  return node_factory().NewUnknownType(ComputeNodeRange());
}

const Type& TypeParser::NewVoidType(const SourceCodeRange& range) {
  return node_factory().NewVoidType(range);
}

// Type ::=
//      | '?'
//      | '*'
//      | NullableType
//      | NonNullableType
//      | UnionType
//      | TypeName
//      | FunctionType
//      | TypeApplication
//      | TypeGroup
//      | RecordType
//      | TupleType
// NullableType ::= '?' Type
// NonNullableType ::= '!' Type
// RecordType ::= '{' (Name ':' Type ','?)* '}'
// TupleType ::= '[' (Type ',')* ']'
// UnionTYpe ::= Type ('|' Type*)
const Type& TypeParser::Parse() {
  const auto& type = ParseUnionType();
  bracket_tracker_->Finish();
  return type;
}

const Type& TypeParser::ParseFunctionType(const Name& name) {
  if (!ConsumeTokenIf(ast::PunctuatorKind::LeftParenthesis))
    return NewTypeName(name);
  std::vector<const Type*> parameters;
  auto kind = ast::FunctionTypeKind::Normal;
  auto expect_type = false;
  if (ConsumeTokenIf(ast::NameId::New)) {
    kind = ast::FunctionTypeKind::New;
    if (!ConsumeTokenIf(ast::PunctuatorKind::Colon))
      AddError(TypeErrorCode::ERROR_TYPE_EXPECT_COLON);
    expect_type = true;
  } else if (ConsumeTokenIf(ast::NameId::This)) {
    kind = ast::FunctionTypeKind::This;
    if (!ConsumeTokenIf(ast::PunctuatorKind::Colon))
      AddError(TypeErrorCode::ERROR_TYPE_EXPECT_COLON);
    expect_type = true;
  }
  while (CanPeekToken() &&
         PeekToken() != ast::PunctuatorKind::RightParenthesis) {
    parameters.push_back(&ParseType());
    expect_type = false;
    if (!CanPeekToken())
      break;
    if (PeekToken() == ast::PunctuatorKind::RightParenthesis)
      break;
    expect_type = true;
    if (ConsumeTokenIf(ast::PunctuatorKind::Comma))
      continue;
    AddError(TypeErrorCode::ERROR_TYPE_EXPECT_COMMA);
  }
  if (expect_type)
    AddError(TypeErrorCode::ERROR_TYPE_EXPECT_TYPE);
  SkipTokensTo(ast::PunctuatorKind::RightParenthesis);
  if (ConsumeTokenIf(ast::PunctuatorKind::Colon))
    return NewFunctionType(kind, parameters, ParseType());
  auto& return_type = NewVoidType(lexer_->location());
  return NewFunctionType(kind, parameters, return_type);
}

const Type& TypeParser::ParseNameAsType(const Name& name) {
  if (ConsumeTokenIf(ast::PunctuatorKind::LessThan))
    return ParseTypeApplication(name);
  return NewTypeName(name);
}

// RecordType ::= '{' (Name ':' Type ','?)* '}'
const Type& TypeParser::ParseRecordType() {
  std::vector<RecordMember> members;
  while (CanPeekToken() && PeekToken() != ast::PunctuatorKind::RightBrace) {
    if (!PeekToken().Is<Name>())
      break;
    auto& name = ConsumeToken().As<Name>();
    if (!ConsumeTokenIf(ast::PunctuatorKind::Colon))
      AddError(TypeErrorCode::ERROR_TYPE_EXPECT_COLON);
    if (!CanPeekToken())
      break;
    auto& type = ParseType();
    members.push_back(std::make_pair(&name, &type));
    if (!ConsumeTokenIf(ast::PunctuatorKind::Comma))
      break;
  }
  SkipTokensTo(ast::PunctuatorKind::RightBrace);
  return NewRecordType(members);
}

// TupleType ::= '[' (Name ','?)* ']'
const Type& TypeParser::ParseTupleType() {
  std::vector<const Type*> members;
  auto after_comma = false;
  while (CanPeekToken() && PeekToken() != ast::PunctuatorKind::RightBracket) {
    members.push_back(&ParseType());
    after_comma = ConsumeTokenIf(ast::PunctuatorKind::Comma);
    if (!after_comma)
      break;
  }
  if (after_comma)
    AddError(TypeErrorCode::ERROR_TYPE_EXPECT_TYPE);
  SkipTokensTo(ast::PunctuatorKind::RightBracket);
  return NewTupleType(members);
}

const Type& TypeParser::ParseType() {
  if (!CanPeekToken()) {
    AddError(TypeErrorCode::ERROR_TYPE_EXPECT_TYPE);
    return NewInvalidType();
  }
  TypeNodeScope scope(this);
  auto& type = ParseTypeBeforeEqual();
  if (!ConsumeTokenIf(ast::PunctuatorKind::Equal))
    return type;
  return NewOptionalType(type);
}

// TypeApplication ::= TypeName '<' (Type',')* '>'
const Type& TypeParser::ParseTypeApplication(const Name& name) {
  std::vector<const Type*> parameters;
  while (CanPeekToken() && PeekToken() != ast::PunctuatorKind::GreaterThan) {
    parameters.push_back(&ParseType());
    if (!ConsumeTokenIf(ast::PunctuatorKind::Comma))
      break;
  }
  SkipTokensTo(ast::PunctuatorKind::GreaterThan);
  return NewTypeApplication(name, parameters);
}

const Type& TypeParser::ParseTypeBeforeEqual() {
  TypeNodeScope scope(this);
  if (ConsumeTokenIf(ast::PunctuatorKind::Times))
    return NewAnyType();
  if (ConsumeTokenIf(ast::PunctuatorKind::Question)) {
    if (CanPeekToken() && CanStartType(PeekToken()))
      return NewNullableType(ParseType());
    return NewUnknownType();
  }
  if (ConsumeTokenIf(ast::PunctuatorKind::LogicalNot))
    return NewNonNullableType(ParseType());
  if (PeekToken() == ast::NameId::Function)
    return ParseFunctionType(ConsumeToken().As<Name>());
  if (PeekToken().Is<Name>())
    return ParseNameAsType(ConsumeToken().As<Name>());
  if (ConsumeTokenIf(ast::PunctuatorKind::LeftBrace))
    return ParseRecordType();
  if (ConsumeTokenIf(ast::PunctuatorKind::LeftBracket))
    return ParseTupleType();
  if (ConsumeTokenIf(ast::PunctuatorKind::LeftParenthesis))
    return ParseTypeGroup();
  if (ConsumeTokenIf(ast::PunctuatorKind::DotDotDot))
    return NewRestType(ParseType());
  ConsumeToken();
  return NewInvalidType();
}

const Type& TypeParser::ParseTypeGroup() {
  auto& type = ParseUnionType();
  ConsumeTokenIf(ast::PunctuatorKind::RightParenthesis);
  return NewTypeGroup(type);
}

const Type& TypeParser::ParseUnionType() {
  if (!CanPeekToken()) {
    AddError(TypeErrorCode::ERROR_TYPE_EXPECT_TYPE);
    return NewInvalidType();
  }
  TypeNodeScope scope(this);
  std::vector<const Type*> types;
  types.push_back(&ParseType());
  while (CanPeekToken() && ConsumeTokenIf(ast::PunctuatorKind::BitOr))
    types.push_back(&ParseType());
  return NewUnionType(types);
}

}  // namespace parser
}  // namespace joana
