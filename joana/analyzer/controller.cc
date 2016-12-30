// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "joana/analyzer/controller.h"

#include "joana/analyzer/context.h"
#include "joana/analyzer/environment.h"
#include "joana/analyzer/environment_builder.h"
#include "joana/analyzer/value.h"
#include "joana/ast/node.h"

namespace joana {
namespace analyzer {

namespace {

//
// Indent
//
struct Indent {
  int depth;
};

std::ostream& operator<<(std::ostream& ostream, const Indent& indent) {
  if (indent.depth == 0)
    return ostream;
  for (auto counter = 0; counter < indent.depth - 1; ++counter)
    ostream << "|  ";
  return ostream << "+--";
}

//
// Dump
//
struct Dump {
  int depth;
  const Environment* environment;
};

std::ostream& operator<<(std::ostream& ostream, const Dump& dump) {
  const auto& environment = *dump.environment;
  const auto depth = dump.depth;
  ostream << Indent{depth} << "Environment " << environment.owner()
          << std::endl;
  for (const auto& name : environment.names()) {
    ostream << Indent{depth + 1} << *environment.BindingOf(*name) << std::endl;
  }
  return ostream;
}

}  // namespace

//
// Controller
//
Controller::Controller(const AnalyzerSettings& settings)
    : context_(new Context(settings)) {}

Controller::~Controller() = default;

void Controller::Analyze() {
  std::cout << Dump{0, &context_->global_environment()};
}

void Controller::Load(const ast::Node& node) {
  EnvironmentBuilder builder(context_.get());
  builder.Load(node);
}

}  // namespace analyzer
}  // namespace joana