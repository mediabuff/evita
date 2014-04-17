// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_range_h)
#define INCLUDE_evita_text_range_h

#include "base/strings/string16.h"

namespace text {

class RangeSet;

class Range {
  friend class RangeSet;

  public: struct Information {
      bool m_fLineNum;
      bool m_fColumn;
      Count m_lLineNum;
      Count m_lColumn;
  }; // Information

  private: Buffer* buffer_;
  private: Posn end_;
  private: Posn start_;

  public: Range(Buffer* buffer, Posn start, Posn end);
  public: ~Range();

  public: Buffer* buffer() const { return buffer_; }

  // [E]
  private: Posn ensurePosn(Posn) const;

  // [G]
  public: Posn GetEnd() const { return end_; }
  public: Posn GetStart() const { return start_; }
  public: base::string16 GetText() const;

  // [S]
  public: Posn SetEnd(Posn);
  public: void SetRange(Posn start, Posn end);
  public: Posn SetStart(Posn);
  public: void SetText(const base::string16& text);

  DISALLOW_COPY_AND_ASSIGN(Range);
};

}  // namespace text

#endif //!defined(INCLUDE_evita_text_range_h)
