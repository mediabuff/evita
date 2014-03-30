// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

Object.defineProperty(TextFieldSelection.prototype, 'end', {
  /**
   * @this {!TextFieldSelection}
   * @return {number}
   */
  get: function() {
    return Math.max(this.anchorOffset, this.focusOffset);
  }
});

Object.defineProperty(TextFieldSelection.prototype, 'start', {
  /**
   * @this {!TextFieldSelection}
   * @return {number}
   */
  get: function() {
    return Math.min(this.anchorOffset, this.focusOffset);
  }
});

Object.defineProperty(TextFieldSelection.prototype, 'text', {
  /**
   * @this {!TextFieldSelection}
   * @return {string}
   */
  get: function() {
    return this.collapsed ? '' :
        this.control.value.substring(this.start, this.end);
  },
  /**
   * @this {!TextFieldSelection}
   * @param {string} new_text
   */
  set: function(new_text) {
    var value = this.control.value;
    if (this.collapsed) {
      var offset = this.focusOffset;
      this.control.value = value.substr(0, offset) + new_text +
                           value.substr(offset);
      this.focusOffset = offset + new_text.length;
      return;
    }
    var start = this.start;
    var end = this.end;
    this.control.value = value.substr(0, start) + new_text + value.substr(end);
    // Relocate anchorOffset and focusOffset
    var diff = new_text.length - (end - start);
    if (!diff)
      return;
    if (this.anchorOffset > start)
      this.anchorOffset = Math.max(this.anchorOffset + diff, start);
    if (this.focusOffset > start)
      this.focusOffset = Math.max(this.focusOffset + diff, start);
  },
});

/** @param {number} offset */
global.TextFieldSelection.prototype.collapseTo = function(offset) {
  this.anchorOffset = this.focusOffset = offset;
};

/** @param {number} offset */
global.TextFieldSelection.prototype.extendTo = function(offset) {
  this.focusOffset = offset;
};
