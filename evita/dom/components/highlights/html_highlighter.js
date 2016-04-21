// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.require('highlights.base');
goog.require('highlights.xml');

goog.scope(function() {

const HtmlTokenStateMachine = highlights.HtmlTokenStateMachine;
const Highlighter = highlights.base.Highlighter;
const Painter = highlights.base.Painter;
const Token = highlights.base.Token;
const Tokenizer = highlights.base.Tokenizer;
const XmlPainter = highlights.xml.XmlPainter;

/** @const @type {!Set<string>} */
const staticHtmlKeywords = new Set();

/**
 * @param {number} charCode
 * @return {boolean}
 */
function isWhitespace(charCode) {
  return charCode <= Unicode.SPACE;
}

class HtmlPainter extends XmlPainter {
  /**
   * @private
   * @param {!TextDocument} document
   */
  constructor(document) { super(document); }

  /**
   * @override
   * @param {!Token} token
   */
  paint(token) {
    switch (token.syntax) {
      case 'script':
        return this.paintStyle(token);
      case 'style':
        return this.paintStyle(token);
    }
    super.paint(token);
  }

  /**
   * @private
   * @param {!Token} token
   */
  paintScript(token) { this.paintToken2(token, 'html_attribute_name'); }

  /**
   * @private
   * @param {!Token} token
   */
  paintStyle(token) { this.paintToken2(token, 'html_attribute_name'); }

  /**
   * @public
   * @param {!TextDocument} document
   * @return {!Painter}
   */
  static create(document) { return new HtmlPainter(document); }
}

class HtmlHighlighter extends Highlighter {
  /**
   * @param {!TextDocument} document
   */
  constructor(document) {
    super(document, HtmlPainter.create, new HtmlTokenStateMachine());
  }

  /**
   * @public
   * @return {!Set<string>}
   * For debugging
   */
  static get keywords() { return staticHtmlKeywords; }

  /**
   * @public
   * @param {string} word
   * Adds a keyword at runtime.
   */
  static addKeyword(word) { staticHtmlKeywords.add(word); }
}

Object.freeze(HtmlHighlighter);
Object.seal(HtmlHighlighter);

highlights.HtmlHighlighter = HtmlHighlighter;
});

// Override |HtmlLexer| by |HtmlHighlighter|.
// TODO(eval1749): Once we get rid of |HtmlLexer|, we should get rid of this
// override.
global['HtmlLexer'] = highlights.HtmlHighlighter;
