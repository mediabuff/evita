// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @const @type {!Object} */
var goog = {};

/** @param {string} fullName */
goog.require = function(fullName) {
  /** @const @type {!Array<string>} */
  const modulePath = [];
  /** @type {!Object} */
  let runner = global;
  for (const name of fullName.split('.')) {
    modulePath.push(name);
    if (!(name in runner)) {
      Editor.loadModule(modulePath.join('.'));
      if (!(name in runner))
        throw new Error(`No such module ${modulePath.join('.')}`);
    }
    runner = runner[name];
  }
  return runner;
};

/** @param {string} qualifiedName */
goog.provide = function(qualifiedName) {
  /** @type {!Object} */
  let runner = global;
  qualifiedName.split('.').forEach(name => {
    if (!(name in runner))
      runner[name] = {};
    runner = /** @type {!Object} */ (runner[name]);
  });
};


/** @param {!function(): ?} fn */
goog.scope = function(fn) {
  fn();
};

goog.provide("core");
goog.provide("text");
