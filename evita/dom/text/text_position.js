// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

goog.scope(function() {
/** @const @type {!Bracket.Detail} */
const NOT_BRACKET = new Bracket.Detail(Bracket.Type.NONE, 0);

/** @const @type {number} */
const MAX_BRACKET_NESTING_LEVEL = 10;

class BracketMatchData {
  /**
   * @param {!Bracket.Detail} data
   * @param {!TextOffset} offset
   */
  constructor(data, offset) {
    this.data = data;
    this.offset = offset;
  }
}

/**
 * @param {!TextPosition} position
 * @return {!Bracket.Detail}
 */
function bracketDataOf(position) {
  const charCode = position.charCode();
  // TODO(eval1749): We should get character syntax from mime type
  // information.
  return Bracket.DATA[charCode] || NOT_BRACKET;
}

/**
 * @param {string} syntax1
 * @param {string} syntax2
 * @return {boolean}
 */
function syntaxEquals(syntax1, syntax2) {
  if (syntax1 === syntax2)
    return true;
  return syntax1 === 'normal' || syntax2 === 'normal';
}

/**
 * @param {!TextPosition} position
 * Note: We use syntax coloring information for preventing matching
 * parenthesis among statement, string and comment.
 * See also |moveForwardBracket()|.
 */
function moveBackwardBracket(position) {
  /** @type {!Array<!BracketMatchData>} */ const bracketStack = [];
  // reset when we reached at bracket.
  /**  @type {string} */ let bracketCharSyntax = '?';
  /** @const @type {number} */ const startOffset = position.offset;

  while (position.offset) {
    position.move(Unit.CHARACTER, -1);
    // TODO(eval1749): We should get character syntax from mime type
    // information.
    /**  @const @type {!Bracket.Detail} */
    const bracket = bracketDataOf(position);
    if (bracket.type === Bracket.Type.NONE)
      continue;
    if (bracketCharSyntax === '?')
      bracketCharSyntax = position.charSyntax();
    else if (!syntaxEquals(position.charSyntax(), bracketCharSyntax))
      continue;

    /** @const @type {number} */
    const currentOffset = position.offset;
    position.moveWhile(function() {
      return syntaxEquals(this.charSyntax(), bracketCharSyntax) &&
          bracketDataOf(this).type === Bracket.Type.ESCAPE;
    }, Count.BACKWARD);
    if ((currentOffset - position.offset) & 1)
      continue;

    /** @const @type {number} */
    const nextOffset = position.offset;
    position.offset = currentOffset;

    if (bracket.type === Bracket.Type.LEFT) {
      if (!bracketStack.length) {
        // We reach at left bracket.
        return;
      }
      /** @const @type {!BracketMatchData} */
      const lastBracket = bracketStack.pop();
      if (lastBracket.data.pair !== position.charCode()) {
        // We reach at mismatched left bracket.
        break;
      }

      if (!bracketStack.length) {
        // We reach at matched left bracket.
        return;
      }
      position.offset = nextOffset;
    } else if (bracket.type === Bracket.Type.RIGHT) {
      if (!bracketStack.length) {
        if (position.offset !== startOffset - 1) {
          // We found right bracket.
          position.move(Unit.CHARACTER);
          return;
        }
      }
      bracketStack.push(new BracketMatchData(bracket, position.offset));
      position.offset = nextOffset;
    }
  }
  position.offset = startOffset;
}

/**
 * @param {!TextPosition} position
 * Note: We use syntax coloring information for preventing matching
 * parenthesis among statement, string and comment.
 * See also |moveBackwardBracket()|.
 */
function moveForwardBracket(position) {
  /** @type {Array<!BracketMatchData>} */ const bracketStack = [];
  // reset when we reached at racket.
  /** @type {string} */ let bracketCharSyntax = '?';
  /** @type {number} */ const startOffset = position.offset;

  for (; position.offset < position.document.length;
       position.move(Unit.CHARACTER)) {
    /**  @const @type {!Bracket.Detail} */
    const bracket = bracketDataOf(position);
    if (bracket.type === Bracket.Type.NONE)
      continue;
    if (bracketCharSyntax === '?')
      bracketCharSyntax = position.charSyntax();
    else if (!syntaxEquals(position.charSyntax(), bracketCharSyntax))
      continue;
    switch (bracket.type) {
      case Bracket.Type.ESCAPE: {
        /** @const @type {number} */
        const currentOffset = position.offset;
        position.moveWhile(function() {
          return bracketDataOf(this).type === Bracket.Type.ESCAPE &&
              syntaxEquals(this.charSyntax(), bracketCharSyntax);
        }, Count.FORWARD);
        if (!((position.offset - currentOffset) & 1))
          position.move(Unit.CHARACTER, -1);
        break;
      }
      case Bracket.Type.LEFT:
        if (!bracketStack.length) {
          if (position.offset !== startOffset) {
            // We reach left bracket.
            return;
          }
          bracketStack.push(new BracketMatchData(bracket, position.offset));
          break;
        }

        if (bracketStack.length === MAX_BRACKET_NESTING_LEVEL) {
          // We found too many left bracket.
          return;
        }
        bracketStack.push(new BracketMatchData(bracket, position.offset));
        break;
      case Bracket.Type.RIGHT:
        if (bracketStack.length === 0) {
          // We reach right bracket before seeing left bracket:
          //    ^...}
          // Move to next of right bracket:
          //    ^...}|
          position.move(Unit.CHARACTER);
          return;
        }
        /** @const @type {!BracketMatchData} */
        const lastBracket = bracketStack.pop();
        if (lastBracket.data.pair === position.charCode()) {
          if (bracketStack.length > 0)
            continue;
          // We reach matched right bracket:
          //    ^(...|)
          // Move to next of right bracket:
          //    ^(...)|
          position.move(Unit.CHARACTER);
          return;
        }
        // We reach nested mismatched right bracket:
        //    ^{ if (foo ... |} or { if ^(foo ... |}
        // Back to left bracket to know mismatched bracket:
        //    ^{ if |(foo ... } or { if ^(foo ... |}
        position.offset = lastBracket.offset;
        return;
    }
  }
  if (bracketStack.length)
    position.offset = bracketStack.pop().offset;
  else
    position.offset = startOffset;
}

class TextPosition {
  /**
   * @param {!TextDocument} document
   * @param {!number} offset
   */
  constructor(document, offset) {
    if (offset < 0 || offset > document.length)
      throw new RangeError('Invalid offset ' + offset + ' for ' + document);
    this.document = document;
    this.offset = offset;
  }

  /** * @return {number} */
  charCode() { return this.document.charCodeAt(this.offset); }

  /** * @return {string} */
  charSyntax() { return this.document.syntaxAt(this.offset); }

  /**
   * @this {!TextPosition}
   * @param {!Unit} unit
   * @param {number=} count, default is one.
   */
  move(unit, count = 1) {
    if (unit !== Unit.BRACKET)
      this.offset = this.document.computeMotion_(unit, count, this.offset);
    else if (count > 0)
      moveForwardBracket(this);
    else if (count < 0)
      moveBackwardBracket(this);
    return this;
  }

  /**
   * @this {!TextPosition}
   * @param {function() : boolean} callback
   * @param {number=} count, default is one
   * @return {!TextPosition}
   */
  moveWhile(callback, count = 1) {
    if (count < 0) {
      while (count && this.offset) {
        --this.offset;
        if (!callback.call(this)) {
          ++this.offset;
          break;
        }
        ++count;
      }
    } else if (count > 0) {
      /** @const @type {number} */
      const end = this.document.length;
      while (count && this.offset < end) {
        if (!callback.call(this))
          break;
        ++this.offset;
        --count;
      }
    }
    return this;
  }
}

/** @constructor */
text.TextPosition = TextPosition;
});

/** @constructor */
var TextPosition = text.TextPosition;
