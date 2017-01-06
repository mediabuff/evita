// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/analyzer/environment.h"

#include "base/logging.h"
#include "joana/ast/syntax_forward.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace analyzer {

//
// Environment
//
Environment::Environment(Zone* zone, Environment* outer, const ast::Node& owner)
    : names_(zone),
      name_map_(zone),
      outer_(outer),
      owner_(owner),
      value_map_(zone) {}

Environment::Environment(Zone* zone, const ast::Node& owner)
    : Environment(zone, nullptr, owner) {}

Environment::~Environment() = default;

void Environment::Bind(const ast::Node& name, Variable* value) {
  const auto key = ast::Name::IdOf(name);
  const auto& result = value_map_.emplace(key, value);
  DCHECK(result.second) << name << " is already bound.";
  const auto& name_result = name_map_.emplace(key, &name);
  DCHECK(name_result.second);
  names_.push_back(&name);
}

Variable* Environment::TryValueOf(const ast::Node& name) const {
  const auto& it = value_map_.find(ast::Name::IdOf(name));
  return it == value_map_.end() ? nullptr : it->second;
}

}  // namespace analyzer
}  // namespace joana
