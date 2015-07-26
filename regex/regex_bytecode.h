// -*- Mode: C++ -*-
//
// Regex Application Program Interface
// regex_bytecode_h.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evedit2/mainline/regex/regex_bytecode.h#3 $
//
#ifndef REGEX_REGEX_BYTECODE_H_
#define REGEX_REGEX_BYTECODE_H_

#include "regex/regex.h"

namespace Regex {
namespace RegexPrivate {

class Scanner;

//////////////////////////////////////////////////////////////////////
//
// Op
//  Regex byte code
//
enum Op {
#define DefByteCode(mp_mnemonic, mp_operand) Op_##mp_mnemonic,
#include "regex_bytecode.inc"
  Op_Limit,
};  // Op

class StringOperand {
 public:
  class Enum {
   public:
    explicit Enum(const StringOperand* p)
        : m_pwch(p->Get()), m_pwchEnd(p->Get() + p->GetLength()) {}

    bool AtEnd() const { return m_pwch == m_pwchEnd; }
    char16 Get() const {
      ASSERT(!AtEnd());
      return *m_pwch;
    }
    void Next() {
      ASSERT(!AtEnd());
      m_pwch++;
    }

   private:
    const char16* m_pwch;
    const char16* m_pwchEnd;
  };

  const char16* Get() const {
    return reinterpret_cast<const char16*>(this + 1);
  }

  char16 Get(int nNth) const { return Get()[nNth]; }
  int GetLength() const { return m_cwch; }

  // TODO(eval1749) |m_cwch| shoul be private.
  int m_cwch;
};

//////////////////////////////////////////////////////////////////////
//
// RegexObj
//
class RegexObj {
 public:
  struct Capture {
    int m_iNth;
    char16* m_pwszName;
  };  // Capture

  void* operator new(size_t, void* pv) { return pv; }

  RegexObj(int rgfOption,
           int nMaxCapture,
           int nMinLen,
           int ofsCode,
           int ofsScanner)
      : m_nMaxCapture(nMaxCapture),
        m_nMinLen(nMinLen),
        m_ofsCode(ofsCode),
        m_ofsScanner(ofsScanner),
        m_rgfOption(rgfOption) {}

  const int* GetCodeStart() const {
    return reinterpret_cast<int*>(reinterpret_cast<Int>(this) + m_ofsCode);
  }
  int GetMaxCapture() const { return m_nMaxCapture; }
  int GetMinLen() const { return m_nMinLen; }
  const Scanner* GetScanner() const {
    return reinterpret_cast<Scanner*>(reinterpret_cast<Int>(this) +
                                      m_ofsScanner);
  }
  bool IsBackward() const {
    return (m_rgfOption & Regex::Option_Backward) != 0;
  }
  bool NextMatch(Regex::IMatchContext*) const;
  bool StartMatch(Regex::IMatchContext*) const;

#if _DEBUG
  void Describe() const;
#endif

 private:
  ~RegexObj() = default;

  int m_nMaxCapture;
  int m_nMinLen;
  int m_ofsCode;
  int m_ofsScanner;
  int m_rgfOption;
};

}  // namespace RegexPrivate
}  // namespace Regex

#endif  // REGEX_REGEX_BYTECODE_H_
