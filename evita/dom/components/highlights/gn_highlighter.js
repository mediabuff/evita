// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights.base');

goog.scope(function() {

const GnTokenStateMachine = highlights.GnTokenStateMachine;
const Highlighter = highlights.base.Highlighter;
const KeywordPainter = highlights.base.KeywordPainter;
const Token = highlights.base.Token;
const Tokenizer = highlights.base.Tokenizer;

/** @const @type {!Set<string>} */
const staticGnKeywords = highlights.staticGnKeywords;

class GnPainter extends KeywordPainter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) { super(document, staticGnKeywords); }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!KeywordPainter}
   */
  static create(document) { return new GnPainter(document); }
}

class GnHighlighter extends Highlighter {
  /**
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(document, GnPainter.create, new GnTokenStateMachine());
  }
}

highlights.GnHighlighter = GnHighlighter;
// Export |GnPainter| for testing.
highlights.GnPainter = GnPainter;
});

// Override |GnLexer| by |GnHighlighter|.
// TODO(eval1749): Once we get rid of |GnLexer|, we should get rid of this
// override.
global['GnLexer'] = highlights.GnHighlighter;
