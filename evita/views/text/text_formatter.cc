// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/text_formatter.h"

#include "base/logging.h"
#include "common/memory/singleton.h"
#include "evita/css/style.h"
#include "evita/css/style_resolver.h"
#include "evita/css/style_selector.h"
#include "evita/text/buffer.h"
#include "evita/text/interval.h"
#include "evita/text/marker.h"
#include "evita/text/marker_set.h"
#include "evita/views/text/render_cell.h"
#include "evita/views/text/render_style.h"
#include "evita/views/text/render_selection.h"
#include "evita/views/text/render_text_block.h"
#include "evita/views/text/render_text_line.h"
#include "evita/text/spelling.h"

namespace views {
namespace rendering {

namespace {

const float cxLeftMargin = 10.0f;
const int k_nTabWidth = 4;

float AlignHeightToPixel(const gfx::Canvas& gfx, float height) {
  return gfx.AlignToPixel(gfx::SizeF(0.0f, height)).height;
}

float AlignWidthToPixel(const gfx::Canvas&, float width) {
  return width;
}

inline char16 toxdigit(int k) {
  if (k <= 9)
    return static_cast<char16>(k + '0');
 return static_cast<char16>(k - 10 + 'A');
}

gfx::ColorF CssColorToColorF(const css::Color& color) {
  return gfx::ColorF(
      static_cast<float>(color.red()) / 255,
      static_cast<float>(color.green()) / 255,
      static_cast<float>(color.blue()) / 255,
      color.alpha());
}

Font* GetFont(const gfx::Canvas& gfx, const css::Style& style) {
  return FontSet::Get(gfx, style)->FindFont(gfx, 'x');
}

const css::Style& GetDefaultStyle(const TextBlock* text_block) {
  return text_block->text_buffer()->GetDefaultStyle();
}

RenderStyle GetRenderStyle(const gfx::Canvas& gfx,
                           const css::Style& style) {
  return RenderStyle(style, GetFont(gfx, style));
}

TextSelection FormatSelection(const text::Buffer* buffer,
                              const Selection& selection) {
  const auto& style = buffer->style_resolver()->ResolveWithoutDefaults(
      selection.active ? css::StyleSelector::active_selection() :
                         css::StyleSelector::inactive_selection());
  return TextSelection(CssColorToColorF(style.bgcolor()),
                       selection.start, selection.end);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextScanner
//  Enumerator for characters and interval
//
class TextFormatter::TextScanner final {
  private: Posn m_lBufEnd;
  private: Posn m_lBufStart;
  private: Posn m_lPosn;
  private: const text::Buffer* m_pBuffer;
  private: text::Interval* m_pInterval;
  private: char16 m_rgwch[80];
  private: mutable const text::Marker* spelling_marker_;
  private: mutable const text::Marker* syntax_marker_;

  public: TextScanner(const text::Buffer* buffer, Posn lPosn)
      : m_pBuffer(buffer),
        m_lPosn(lPosn),
        spelling_marker_(nullptr),
        syntax_marker_(nullptr) {
    m_pInterval = m_pBuffer->GetIntervalAt(m_lPosn);
    DCHECK(m_pInterval);
    fill();
  }

  public: ~TextScanner() = default;

  public: const common::AtomicString& spelling() const;
  public: const css::StyleResolver* style_resolver() const {
    return m_pBuffer->style_resolver();
  }
  public: const common::AtomicString& syntax() const;

  public: bool AtEnd() const {
    return m_lBufStart == m_lBufEnd;
  }

  private: void fill() {
    auto const cwch = m_pBuffer->GetText(
        m_rgwch, m_lPosn, static_cast<Posn>(m_lPosn + lengthof(m_rgwch)));

    m_lBufStart = m_lPosn;
    m_lBufEnd   = m_lPosn + cwch;
  }

  public: char16 GetChar() const {
    if (AtEnd())
      return 0;
    DCHECK_GE(m_lPosn, m_lBufStart);
    DCHECK_LT(m_lPosn, m_lBufEnd);
    return m_rgwch[m_lPosn - m_lBufStart];
  }

  public: Posn GetPosn() const { return m_lPosn; }

  public: const css::Style& GetStyle() const {
    if (AtEnd())
      return m_pBuffer->GetDefaultStyle();
    DCHECK(m_pInterval);
    return m_pInterval->style();
  }

  public: RenderStyle MakeRenderStyle(const css::Style& style,
                                      Font* font) const;

  public: void Next() {
    if (AtEnd())
      return;
    m_lPosn += 1;
    if (m_lPosn >= m_lBufEnd)
      fill();

    if (!m_pInterval->Contains(m_lPosn)) {
      m_pInterval = m_pBuffer->GetIntervalAt(m_lPosn);
      DCHECK(m_pInterval);
    }
  }

  DISALLOW_COPY_AND_ASSIGN(TextScanner);

};

const common::AtomicString& TextFormatter::TextScanner::spelling() const {
  if (!spelling_marker_ || m_lPosn >= spelling_marker_->end()) {
    spelling_marker_ = m_pBuffer->spelling_markers()->
        GetLowerBoundMarker(m_lPosn);
  }
  return spelling_marker_ && spelling_marker_->Contains(m_lPosn) ?
      spelling_marker_->type() : common::AtomicString::Empty();
}

const common::AtomicString& TextFormatter::TextScanner::syntax() const {
  if (!syntax_marker_ || m_lPosn >= syntax_marker_->end()) {
    syntax_marker_ = m_pBuffer->syntax_markers()->
        GetLowerBoundMarker(m_lPosn);
  }
  return syntax_marker_ && syntax_marker_->Contains(m_lPosn) ?
      syntax_marker_->type() : common::AtomicString::Empty();
}

RenderStyle TextFormatter::TextScanner::MakeRenderStyle(
    const css::Style& style, Font* font) const {
  return RenderStyle(style, font);
}

//////////////////////////////////////////////////////////////////////
//
// TextFormatter
//
TextFormatter::TextFormatter(const gfx::Canvas& gfx, TextBlock* text_block,
                             const Selection& selection, Posn lStart,
                             float zoom)
    : default_render_style_(GetRenderStyle(gfx, GetDefaultStyle(text_block))),
      default_style_(GetDefaultStyle(text_block)),
      m_gfx(gfx),
      text_block_(text_block),
      text_scanner_(new TextScanner(text_block->text_buffer(), lStart)),
      zoom_(zoom) {
  DCHECK(!text_block_->bounds().empty());
  DCHECK_GT(zoom_, 0.0f);
  text_block->set_selection(FormatSelection(text_block->text_buffer(),
                                            selection));
}

TextFormatter::~TextFormatter() {
}

void TextFormatter::Format() {
  DCHECK(!text_block_->bounds().empty());
  for (;;) {
    auto const pLine = FormatLine();
    DCHECK_GT(pLine->bounds().height(), 0.0f);

    text_block_->Append(pLine);

    // Line must have at least one cell other than filler.
    DCHECK_GE(pLine->GetEnd(), pLine->GetStart());

    if (text_block_->GetHeight() >= text_block_->height()) {
      // TextBlock is filled up with lines.
      break;
    }

    if (auto const marker_cell = pLine->cells().back()->as<MarkerCell>()) {
      if (marker_cell->marker_name() == TextMarker::EndOfDocument) {
        // We have no more contents.
        break;
      }
    }
  }

  auto& style = text_block_->text_buffer()->GetDefaultStyle();
  text_block_->set_default_style(RenderStyle(style, nullptr));
  text_block_->Finish();
}

// Returns true if more contents is avaialble, otherwise returns false.
TextLine* TextFormatter::FormatLine() {
  DCHECK(!text_block_->bounds().empty());
  auto const pLine = new TextLine();
  pLine->set_start(text_scanner_->GetPosn());

  auto x = text_block_->left();
  auto descent = 0.0f;
  auto ascent  = 0.0f;

  Cell* pCell;

  // Left margin
  {
    auto const cyMinHeight = 1.0f;

    pCell = new FillerCell(default_render_style_, cxLeftMargin, cyMinHeight);
    pLine->AddCell(pCell);
    x += cxLeftMargin;
  }

  for (;;) {
    if (text_scanner_->AtEnd()) {
      pCell = formatMarker(TextMarker::EndOfDocument);
      break;
    }

    auto const wch = text_scanner_->GetChar();

    if (wch == 0x0A) {
      pCell = formatMarker(TextMarker::EndOfLine);
      text_scanner_->Next();
      break;
    }

    auto const cx = pCell->width();

    pCell = formatChar(pCell, x, wch);
    if (!pCell) {
      pCell = formatMarker(TextMarker::LineWrap);
      break;
    }

    text_scanner_->Next();

    if (pLine->last_cell() == pCell) {
      x -= cx;
    } else {
      pLine->AddCell(pCell);
    }

    x += pCell->width();
    descent = std::max(pCell->descent(), descent);
    ascent  = std::max(pCell->height() - pCell->descent(), ascent);
  }

  // We have at least one cell.
  //   o end of buffer: End-Of-Buffer MarkerCell
  //   o end of line:   End-Of-Line MarkerCell
  //   o wrapped line:  Warp MarkerCell
  DCHECK(pCell);
  pLine->AddCell(pCell);

  x += pCell->width();
  descent = std::max(pCell->descent(), descent);
  ascent  = std::max(pCell->height() - pCell->descent(), ascent);

  pLine->Fix(text_block_->left(), text_block_->top() + text_block_->GetHeight(),
             AlignHeightToPixel(m_gfx, ascent),
             AlignHeightToPixel(m_gfx, descent));

  return pLine;
}

//////////////////////////////////////////////////////////////////////
//
// TextFormatter::formatChar
//
Cell* TextFormatter::formatChar(Cell* pPrev, float x, char16 wch) {
  auto const lPosn = text_scanner_->GetPosn();
  auto style = text_scanner_->GetStyle();

  auto const spelling = text_scanner_->spelling();
  if (!spelling.empty()) {
    style.Merge(text_scanner_->style_resolver()->
        ResolveWithoutDefaults(spelling));
  }

  auto const syntax = text_scanner_->syntax();
  if (!syntax.empty()) {
    style.Merge(text_scanner_->style_resolver()->
        ResolveWithoutDefaults(syntax));
  }

  style.Merge(text_scanner_->style_resolver()->Resolve(
    css::StyleSelector::defaults()));
  style.set_font_size(style.font_size() * zoom_);

  if (0x09 == wch) {
    style.OverrideBy(text_scanner_->style_resolver()->ResolveWithoutDefaults(
        css::StyleSelector::end_of_file_marker()));
    auto const pFont = FontSet::Get(m_gfx, style)->FindFont(m_gfx, 'x');
    auto const cxTab = AlignWidthToPixel(m_gfx, pFont->GetCharWidth(' ')) *
                          k_nTabWidth;
    auto const x2 = (x + cxTab - cxLeftMargin) / cxTab * cxTab;
    auto const cx = (x2 + cxLeftMargin) - x;
    auto const cxM = AlignWidthToPixel(m_gfx, pFont->GetCharWidth('M'));
    if (pPrev && x2 + cxM > text_block_->right())
      return nullptr;

    auto const height = AlignHeightToPixel(m_gfx, pFont->height());
    return new MarkerCell(text_scanner_->MakeRenderStyle(style, pFont), cx,
                          height, lPosn, TextMarker::Tab);
  }

  auto const pFont = wch < 0x20 || wch == 0xFEFF ?
      nullptr :
      FontSet::Get(m_gfx, style)->FindFont(m_gfx, wch);

  if (!pFont) {
    style.OverrideBy(text_scanner_->style_resolver()->ResolveWithoutDefaults(
        css::StyleSelector::end_of_file_marker()));
    auto const font = FontSet::Get(m_gfx, style)->FindFont(m_gfx, 'u');
    base::string16 string;
    if (wch < 0x20) {
      string.push_back('^');
      string.push_back(static_cast<base::char16>(wch + 0x40));
    } else {
      string.push_back('u');
      string.push_back(toxdigit((wch >> 12) & 15));
      string.push_back(toxdigit((wch >> 8) & 15));
      string.push_back(toxdigit((wch >> 4) & 15));
      string.push_back(toxdigit((wch >> 0) & 15));
    }

    auto const width = font->GetTextWidth(string) + 4;
    auto const char_width = font->GetCharWidth('M');
    if (pPrev && x + width + char_width > text_block_->right())
      return nullptr;
    auto const height = AlignHeightToPixel(m_gfx, font->height());
    return new UnicodeCell(text_scanner_->MakeRenderStyle(style, font),
                           width, height, lPosn, string);
  }

  auto render_style = text_scanner_->MakeRenderStyle(style, pFont);
  auto const cx = AlignWidthToPixel(m_gfx, pFont->GetCharWidth(wch));
  if (pPrev) {
    auto const cxM = AlignWidthToPixel(m_gfx, pFont->GetCharWidth('M'));
    if (x + cx + cxM > text_block_->right()) {
      // We doesn't have enough room for a char in the line.
      return nullptr;
    }

    if (pPrev->Merge(render_style, cx)) {
      pPrev->as<TextCell>()->AddChar(wch);
      return pPrev;
    }
  }

  auto const height = AlignHeightToPixel(m_gfx, pFont->height());
  return new TextCell(render_style, cx, height, lPosn, base::string16(1u, wch));
}

Cell* TextFormatter::formatMarker(TextMarker marker_name) {
  auto style = text_scanner_->GetStyle();
  style.Merge(text_scanner_->style_resolver()->Resolve(
    css::StyleSelector::defaults()));
  style.OverrideBy(text_scanner_->style_resolver()->ResolveWithoutDefaults(
      css::StyleSelector::end_of_line_marker()));
  style.set_font_size(style.font_size() * zoom_);

  auto const pFont = FontSet::Get(m_gfx, style)->FindFont(m_gfx, 'x');
  auto const width = AlignWidthToPixel(m_gfx, pFont->GetCharWidth('x'));
  auto const height = AlignHeightToPixel(m_gfx, pFont->height());
  return new MarkerCell(text_scanner_->MakeRenderStyle(style, pFont),
                        width, height, text_scanner_->GetPosn(),
                        marker_name);
}

} // namespace rendering
} // namespace views
