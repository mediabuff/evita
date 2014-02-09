// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "evita/text/range.h"

#include <algorithm>

#include "evita/text/buffer.h"
#include "evita/ed_undo.h"

// Smart handle for HGLOBAL
template<class T>
class Global
{
    private: HGLOBAL m_h;
    private: T*      m_p;

    public: Global() : m_h(NULL), m_p(NULL) {}

    public: ~Global()
    {
        if (m_h != NULL)
        {
            if (m_p != NULL) ::GlobalUnlock(m_h);
            ::GlobalFree(m_h);
        }
    } // ~Global

    public: operator HANDLE()
        { return reinterpret_cast<HANDLE>(m_h); }

    public: bool Alloc(size_t cb)
    {
        ASSERT(m_h == NULL);
        m_h = ::GlobalAlloc(GMEM_MOVEABLE, cb);
        return m_h != NULL;
    } // Alloc

    public: void Detach()
    {
        ASSERT(m_p == NULL);
        ASSERT(m_h != NULL);
        m_h = NULL;
    } // Detach

    public: T* Lock()
    {
        if (m_h == NULL) return NULL;
        return m_p = reinterpret_cast<T*>(::GlobalLock(m_h));
    } // Lock

    public: void Unlock()
    {
        if (m_h == NULL) return;
        if (m_p == NULL) return;
        m_p = NULL;
        ::GlobalUnlock(m_h);
    } // Unlock
}; // Global

/// <remark>
///  Smart handle for Windows clipboard
/// </remark>
class Clipboard
{
    private: BOOL m_fSucceeded;
    private: mutable HANDLE m_hGlobal;

    public: Clipboard() : m_hGlobal(NULL)
        { m_fSucceeded = ::OpenClipboard(NULL); }

    public: ~Clipboard()
    {
        if (NULL != m_hGlobal) ::GlobalUnlock(m_hGlobal);
        if (m_fSucceeded) ::CloseClipboard();
    } // ~Clipboard

    public: BOOL Empty()
        { return ::EmptyClipboard(); }

    public: char16* GetText() const
    {
        m_hGlobal = ::GetClipboardData(CF_UNICODETEXT);
        if (NULL == m_hGlobal) return NULL;

        return reinterpret_cast<char16*>(::GlobalLock(m_hGlobal));
    } // GetText

    public: BOOL HasFormat(uint uFormat) const
        { return ::IsClipboardFormatAvailable(uFormat); }

    public: BOOL IsOpen() const { return m_fSucceeded; }

    public: bool Set(HANDLE h)
        { return ::SetClipboardData(CF_UNICODETEXT, h) != NULL; }
}; // Clipboard

namespace text
{

/// <summary>
///  Parenthesis stack
/// </summary>
class ParenStack
{
    private: struct Entry
    {
        char16  m_wch;
    }; // Entry

    private: Entry* m_pCurr;
    private: Entry  m_rgoEntry[100];

    public: ParenStack() :
        m_pCurr(m_rgoEntry + lengthof(m_rgoEntry)) {}

    public: bool IsEmpty() const
    {
        return m_pCurr >= m_rgoEntry + lengthof(m_rgoEntry);
    } // IsEmtpy

    public: bool IsFull() const
    {
        return m_pCurr <= m_rgoEntry;
    } // IsFull

    public: char16 GetChar() const
    {
        ASSERT(! IsEmpty());
        return m_pCurr->m_wch;
    } // GetChar

    public: void Pop()
    {
        ASSERT(! IsEmpty());
        m_pCurr++;
    } // Pop

    public: void Push(char16 wch)
    {
        ASSERT(! IsFull());
        --m_pCurr;
        ASSERT(m_pCurr < m_rgoEntry + lengthof(m_rgoEntry));
        m_pCurr->m_wch = wch;
    } // Push
}; // ParentStack

/// <summary>
///   Constructs Range object for Buffer with specified range.
/// </summary>
/// <param name="pBuffer">Buffer object for Range</param>
/// <param name="lStart">Start position</param>
/// <param name="lEnd">End position</param>
Range::Range(Buffer* pBuffer, Posn lStart, Posn lEnd) :
    m_eKind(Kind_Range),
    m_lEnd(lEnd),
    m_lStart(lStart),
    m_pBuffer(pBuffer)
{
    ASSERT(m_pBuffer->IsValidRange(m_lStart, m_lEnd));
    m_pBuffer->InternalAddRange(this);
} // Range::Range

/// <summary>
///   Constructs Range object as same as pRange.
/// </summary>
/// <param name="pRange">Range object</param>
Range::Range(const Range& other) :
    m_eKind(Kind_Range),
    m_lEnd(other.GetEnd()),
    m_lStart(other.GetStart()),
    m_pBuffer(other.GetBuffer())
{
    m_pBuffer->InternalAddRange(this);
} // Range::Range

/// <summary>
///  Remove this range from buffer
/// </summary>
Range::~Range()
{
    DEBUG_PRINTF("%p\n", this);
    if (NULL != m_pBuffer)
    {
        m_pBuffer->InternalRemoveRange(this);
    }
} // Range::~Range

/// <summary>
///   Delete object.
/// </summary>
void Range::destroy() {
    HANDLE const h = m_pBuffer->GetHeap();
    ASSERT(NULL != h);
    this->Range::~Range();
    ::HeapFree(h, 0, this);
} // destroy

/// <summary>
///  Returns case of this range
/// </summary>
/// <returns>StringCase of this range</returns>
StringCase Range::AnalyzeCase() const
{
    StringCase eCase = StringCase_None;

    enum State
    {
        State_Start,

        State_FirstCapSecond,
        State_FirstCapInWord,
        State_FirstCapNotWord,

        State_Lower,

        State_RestCapInWord,
        State_RestCapNotWord,
        State_RestCapSecond,

        State_Upper,
    } eState = State_Start;

    foreach (Buffer::EnumChar, oEnum, this)
    {
        char16 wch = oEnum.Get();

        switch (eState)
        {
        case State_Start:
            if (IsUpperCase(wch))
            {
                eCase  = StringCase_CapitalizedAll;
                eState = State_FirstCapSecond;
            }
            else if (IsLowerCase(wch))
            {
                eCase  = StringCase_Lower;
                eState = State_Lower;
            }
            break;

        case State_FirstCapInWord:
            if (IsUpperCase(wch))
            {
                // We see "FoB"
                return StringCase_Mixed;
            }
            else if (IsLowerCase(wch))
            {
                // We see "Foo"
            }
            else
            {
                // We see "Foo+"
                eState = State_FirstCapNotWord;
            }
            break;

        case State_FirstCapNotWord:
            if (IsUpperCase(wch))
            {
                // We see "Foo B"
                eState = State_RestCapInWord;
            }
            else if (IsLowerCase(wch))
            {
                // We see "Foo b"
                eCase  = StringCase_Capitalized;
                eState = State_Lower;
            }
            else
            {
                // We see "Foo+"
            }
            break;

        case State_FirstCapSecond:
            if (IsUpperCase(wch))
            {
                // We see "FO"
                eCase  = StringCase_Upper;
                eState = State_Upper;
            }
            else if (IsLowerCase(wch))
            {
                // We see "Fo"
                eState = State_FirstCapInWord;
            }
            else
            {
                // We see "F+"
                eState = State_FirstCapNotWord;
            }
            break;

        case State_Lower:
            if (IsUpperCase(wch))
            {
                // We see "foB"
                return StringCase_Mixed;
            }
            else if (IsLowerCase(wch))
            {
                // We see "Foo"
            }
            else
            {
                // We see "foo+"
            }
            break;

        case State_RestCapInWord:
            if (IsUpperCase(wch))
            {
                // We see "Foo Bar BaZ"
                return StringCase_Mixed;
            }
            else if (IsLowerCase(wch))
            {
                // We see "Foo Bar Ba"
            }
            else
            {
                // We see "Foo Bar+ Baz+"
                eState = State_RestCapNotWord;
            }
            break;

        case State_RestCapNotWord:
            if (IsUpperCase(wch))
            {
                // We see "Foo Bar B"
                eState = State_RestCapSecond;
            }
            else if (IsLowerCase(wch))
            {
                // We see "Foo Bar b"
                return StringCase_Mixed;
            }
            else
            {
                // We see "Foo Bar+"
            }
            break;

        case State_RestCapSecond:
            if (IsUpperCase(wch))
            {
                // We see "FO"
                eCase  = StringCase_Upper;
                eState = State_Upper;
            }
            else if (IsLowerCase(wch))
            {
                // We see "Fo"
                eState = State_RestCapInWord;
            }
            else
            {
                // We see "F+"
                eState = State_RestCapNotWord;
            }
            break;

        case State_Upper:
            if (IsUpperCase(wch))
            {
                // We see "FOO"
            }
            else if (IsLowerCase(wch))
            {
                return StringCase_Mixed;
            }
            else
            {
                // We see "FOO+"
            }
            break;

        default:
            CAN_NOT_HAPPEN();
        } // switch state
    } // for each char

    return eCase;
} // Range::AnalyzeCase

/// <summary>
///  Capitablize this range.
/// </summary>
/// <seealso cref="Range::CapitalizeAll"/>
/// <seealso cref="Range::Downcase"/>
/// <seealso cref="Range::Upcase"/>
void Range::Capitalize()
{
    UndoBlock oUndo(GetBuffer(), L"Range.Capialize");

    bool fCapitalized = false;

    foreach (Buffer::EnumChar, oEnum, this)
    {
        char16 wch = oEnum.Get();
        if (fCapitalized)
        {
            oEnum.Set(::CharDowncase(wch));
        }
        else if (IsUpperCase(wch))
        {
            fCapitalized = true;
        }
        else if (IsLowerCase(wch))
        {
            oEnum.Set(::CharUpcase(wch));
            fCapitalized = true;
        }
    } // for each char
} // Range::Capitalize

/// <summary>
///  Capitalize all words in this range.
/// </summary>
/// <seealso cref="Range::Capitalize"/>
/// <seealso cref="Range::Downcase"/>
/// <seealso cref="Range::Upcase"/>
void Range::CapitalizeAll()
{
    UndoBlock oUndo(GetBuffer(), L"Range.CapializeAll");

    bool fWord = false;

    foreach (Buffer::EnumChar, oEnum, this)
    {
        char16 wch = oEnum.Get();
        if (fWord)
        {
            if (IsUpperCase(wch))
            {
                oEnum.Set(::CharDowncase(wch));
            }
            else if (! IsLowerCase(wch))
            {
                fWord = false;
            }
        }
        else if (IsUpperCase(wch))
        {
            fWord = true;
        }
        else if (IsLowerCase(wch))
        {
            oEnum.Set(::CharUpcase(wch));
            fWord = true;
        }
    } // for each char
} // Range::CapitalizeAll

/// <summary>
///  Collapse this range to specified end.
/// </summary>
/// <param name="eCollapse">Collapse direction</param>
void Range::Collapse(CollapseWhich eCollapse)
{
    switch (eCollapse)
    {
    case Collapse_End:
        m_lStart = m_lEnd;
        break;

    case Collapse_Start:
        m_lEnd = m_lStart;
        break;
    }
} // Range::Collapse

/// <summary>
///  Copies contents of this range into Windows clipboard
/// </summary>
/// <returns>Number of characters copied to clipboard</returns>
/// <seealso cref="Range::Cut"/>
/// <seealso cref="Range::Paste"/>
Count Range::Copy()
{
    if (m_lStart == m_lEnd)
    {
        return 0;
    }

    Count cwch = 0;
    for (Posn lPosn = m_lStart; lPosn < m_lEnd; lPosn += 1)
    {
        char16 wch = m_pBuffer->GetCharAt(lPosn);
        if (wch == 0x0A)
        {
            cwch += 1;
        }

        cwch += 1;
    } // for posn

    size_t cb = sizeof(char16) * (cwch + 1);

    Global<char16> oGlobal;
    if (! oGlobal.Alloc(cb)) return 0;

    {
        char16* pwch = oGlobal.Lock();

        if (NULL == pwch) return 0;

        for (Posn lPosn = m_lStart; lPosn < m_lEnd; lPosn += 1)
        {
            char16 wch = m_pBuffer->GetCharAt(lPosn);
            if (wch == 0x0A)
            {
                *pwch++ = 0x0D;
            }
            *pwch++ = wch;
        } // for posn

        *pwch = 0;

        oGlobal.Unlock();
    }

    Clipboard oClipboard;
    if (! oClipboard.IsOpen()) return 0;
    if (! oClipboard.Empty()) return 0;
    if (! oClipboard.Set(oGlobal)) return 0;

    oGlobal.Detach();

    return cwch;
} // Range::Copy

/// <summary>
///   Downcase contents of this region.
/// </summary>
/// <seealso cref="Range::Capitalize"/>
/// <seealso cref="Range::CapitalizeAll"/>
/// <seealso cref="Range::Upcase"/>
void Range::Downcase()
{
    UndoBlock oUndo(GetBuffer(), L"Range.Downcase");

    foreach (Buffer::EnumChar, oEnum, this)
    {
        oEnum.Set(::CharDowncase(oEnum.Get()));
    } // for each char
} // Range::Downcase

/// <summary>
///   Move or extend this range to specified end.
/// </summary>
/// <param name="eUnit">Unit for end</param>
/// <param name="fExtend">True to extend this range, or false to move</param>
/// <seealso cref="Range::StartOf"/>
Count Range::EndOf(Unit eUnit, bool fExtend)
{
    Posn lPosn = m_pBuffer->ComputeEndOf(eUnit, m_lEnd);

    Count k = m_lEnd - lPosn;

    m_lEnd = lPosn;
    if (! fExtend)
    {
        m_lStart = m_lEnd;
    }

    return k;
} // Range::EndOf

/// <summary>
///   Returns valid position
/// </summary>
/// <param name="lPosn">Position to validate</param>
/// <returns>Valid position</returns>
Posn Range::ensurePosn(Posn lPosn) const
{
    if (lPosn < 0) return 0;
    if (lPosn > m_pBuffer->GetEnd()) return m_pBuffer->GetEnd();
    return lPosn;
} // ensurePosn

/// <summary>
///   Find specified character in range from start.
/// </summary>
/// <returns>Position of character or -1 if not found</returns>
Posn Range::FindFirstChar(char16 wchFind) const
{
    foreach (Buffer::EnumChar, oEnum, this)
    {
        if (oEnum.Get() == wchFind)
        {
            return oEnum.GetPosn();
        }
    } // for posn

    return -1;
} // Range::FindFirstChar

/// <summary>
///   Retrive line number and column of this range.
/// </summary>
/// <param name="n">Maximum number of scanning</param>
/// <param name="out_oInfo">Information to be return</param>
// FIXME 2007-07-18 yosi We should stop if counter reaches zero.
void Range::GetInformation(Information* out_oInfo, Count n) const
{
    Count k = n;
    out_oInfo->m_lLineNum = 1;
    for (Posn lPosn = 0; lPosn < m_lStart; lPosn++)
    {
        if (m_pBuffer->GetCharAt(lPosn) == 0x0A)
        {
            out_oInfo->m_lLineNum += 1;
        }
    } // for posn

    out_oInfo->m_fLineNum = k > 0;

    Posn lLineStart = m_pBuffer->ComputeStartOf(Unit_Line, m_lStart);
    out_oInfo->m_fColumn = (m_lStart - lLineStart) < n;
    out_oInfo->m_lColumn = m_lStart - lLineStart;
} // Range::GetInformation

/// <summary>
///  Get contents of this range as C-string.
/// </summary>
/// <returns>C-string</returns>
/// <seealso cref="Range::SetText(const char16*, int)"/>
base::string16 Range::GetText() const
{
    return m_pBuffer->GetText(m_lStart, m_lEnd);
} // Range::GetText

/// <summary>
///   Move start (if n &lt; 0) or end (if n > 0) of this range.
/// </summary>
/// <param name="eUnit">Unit to move</param>
/// <param name="n">Number of unit to move</param>
/// <returns>Number of moved unit</returns>
/// <seealso cref="Range::MoveEnd"/>
/// <seealso cref="Range::MoveStart"/>
Count Range::Move(Unit eUnit, Count n)
{
    if (n > 0)
    {
        m_lEnd = m_lStart;
    }
    else
    {
        m_lStart = m_lEnd;
    }

    Count k = m_pBuffer->ComputeMotion(eUnit, n, &m_lStart);

    m_lEnd = m_lStart;

    return k;
} // Range::Move

/// <summary>
///   Move end of this range.
/// </summary>
/// <param name="eUnit">Unit to move</param>
/// <param name="n">Number of unit to move</param>
/// <returns>Number of moved unit</returns>
/// <seealso cref="Range::Move"/>
/// <seealso cref="Range::MoveStart"/>
Count Range::MoveEnd(Unit eUnit, Count n)
{
    return m_pBuffer->ComputeMotion(eUnit, n, &m_lEnd);
} // Range::MoveEnd

/// <summary>
///   Move start of this range.
/// </summary>
/// <param name="eUnit">Unit to move</param>
/// <param name="n">Number of unit to move</param>
/// <returns>Number of moved unit</returns>
/// <seealso cref="Range::Move"/>
/// <seealso cref="Range::MoveEnd"/>
Count Range::MoveStart(Unit eUnit, Count n)
{
    return m_pBuffer->ComputeMotion(eUnit, n, &m_lStart);
} // Range::MoveStart

/// <summary>
///  Replace this range with Windows clipboard
/// </summary>
/// <seealso cref="Range::Copy"/>
/// <seealso cref="Range::Cut"/>
void Range::Paste()
{
    if (m_pBuffer->IsReadOnly())
    {
        return;
    }

    Clipboard oClipboard;
    if (! oClipboard.IsOpen())
    {
        return;
    }

    char16* pwsz = oClipboard.GetText();
    if (NULL == pwsz)
    {
        return;
    }

    if (0 == *pwsz)
    {
        return;
    }

    UndoBlock oUndo(this, L"Range.Paste");

    m_pBuffer->Delete(m_lStart, m_lEnd);

    Posn lPosn = m_lStart;
    char16* pwchStart = pwsz;
    enum { Start, Normal, Cr } eState = Start;
    while (0 != *pwsz)
    {
        switch (eState)
        {
        case Normal:
            if (0x0D == *pwsz)
            {
                eState = Cr;
            }
            break;

        case Cr:
            switch (*pwsz)
            {
            case 0x0A:
            {
                pwsz[-1] = 0x0A;
                Count k = static_cast<Count>(pwsz - pwchStart);
                m_pBuffer->Insert(lPosn, pwchStart, k);
                lPosn += k;
                eState = Start;
                pwchStart = pwsz + 1;
                break;
            } // 0x0A

            case 0x0D:
                break;

            default:
                eState = Normal;
                break;
            }
            break;

        case Start:
            eState = 0x0D == *pwsz ? Cr : Normal;
            break;
        } // switch eState
        pwsz++;
    } // while

    if (Start != eState)
    {
        Count k = static_cast<Count>(pwsz - pwchStart);
        m_pBuffer->Insert(lPosn, pwchStart, k);
        lPosn += k;
    }

    m_lStart = lPosn;
    m_lEnd   = lPosn;
} // Range::Paste

// Range::SetEnd
Posn Range::SetEnd(Posn lPosn)
{
    SetRange(m_lStart, lPosn);
    return m_lEnd;
} // Range::SetEnd

// Range::SetRange
void Range::SetRange(Posn lStart, Posn lEnd)
{
    lStart = ensurePosn(lStart);
    lEnd   = ensurePosn(lEnd);
    if (lStart > lEnd) swap(lStart, lEnd);
    m_lStart = lStart;
    m_lEnd   = lEnd;
} // Range::SetRange

void Range::SetRange(const Range* pRange)
{
    ASSERT(pRange->GetBuffer() == GetBuffer());
    m_lStart = pRange->GetStart();
    m_lEnd   = pRange->GetEnd();
} // Range::SetRang

// Range::SetStart
Posn Range::SetStart(Posn lPosn)
{
    SetRange(lPosn, m_lEnd);
    return m_lStart;
} // Range::SetStart

/// <summary>
///  Replace contents of this range with specified string.
/// </summary>
/// <param name="pwch">Start of string</param>
/// <param name="cwch">Number of characters in string</param>
/// <returns>Number of characters of string.</returns>
/// <seealso cref="Range::GetText(void)"/>
void Range::SetText(const base::string16& text)
{
    if (m_pBuffer->IsReadOnly())
    {
        // TODO: We should throw read only buffer exception.
        return;
    }

    if (m_lStart == m_lEnd)
    {
      UndoBlock oUndo(this, L"Range.SetText");
      m_pBuffer->Insert(m_lStart, text.data(),
                        static_cast<Count>(text.length()));
    }
    else
    {
        UndoBlock oUndo(this, L"Range.SetText");
        m_pBuffer->Delete(m_lStart, m_lEnd);
        m_pBuffer->Insert(m_lStart, text.data(),
                          static_cast<Count>(text.length()));
    } // if

    m_lEnd = ensurePosn(static_cast<Posn>(m_lStart + text.length()));
}

/// <summary>
///   Move or extend this range to specified start.
/// </summary>
/// <param name="eUnit">Unit for start</param>
/// <param name="fExtend">True to extend this range, or false to move</param>
/// <seealso cref="Range::EndOf"/>
Count Range::StartOf(Unit eUnit, bool fExtend)
{
    Posn lPosn = m_pBuffer->ComputeStartOf(eUnit, m_lStart);

    Count k = lPosn - m_lStart;

    m_lStart = lPosn;
    if (! fExtend) m_lEnd = m_lStart;

    return k;
} // Range::StartOf

/// <summary>
///  Downcase contents of this region.
/// </summary>
/// <seealso cref="Range::Capitalize"/>
/// <seealso cref="Range::CapitalizeAll"/>
/// <seealso cref="Range::Downcase"/>
void Range::Upcase()
{
    UndoBlock oUndo(GetBuffer(), L"Range.Upcase");

    foreach (Buffer::EnumChar, oEnum, this)
    {
        oEnum.Set(::CharUpcase(oEnum.Get()));
    } // for each char
} // Range::Upcase

/// <summary>
///  Constructs buffer character enumerator from range.
/// </summary>
/// <param name="pRange">Range for enumeration</param>
Buffer::EnumChar::EnumChar(const Range* pRange) :
    m_lEnd(pRange->GetEnd()),
    m_lPosn(pRange->GetStart()),
    m_pBuffer(pRange->GetBuffer()) {}

/// <summary>
///  Constructs buffer reverse character enumerator from range.
/// </summary>
/// <param name="pRange">Range for enumeration</param>
Buffer::EnumCharRev::EnumCharRev(const Range* pRange) :
    m_lStart(pRange->GetStart()),
    m_lPosn(pRange->GetEnd()),
    m_pBuffer(pRange->GetBuffer()) {}

/// <summary>
///  Construct UndoBlock with specified range and name
/// </summary>
/// <param name="p">Range</param>
/// <param name="s">Name of undo block</param>
UndoBlock::UndoBlock(Range* range, const base::string16& name)
    : UndoBlock(range->GetBuffer(), name) {
}

}  // namespace text
