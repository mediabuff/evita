// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/style/models/style_resolver.h"

#include "base/logging.h"
#include "evita/text/style/models/style.h"
#include "evita/text/style/models/style_rule.h"
#include "evita/text/style/models/style_selector.h"
#include "evita/text/style/models/style_sheet.h"

#define USE_SELETION_OVERLAY 1

namespace xcss {

namespace {
void InstallSyntaxColor(StyleSheet* style_sheet,
                        base::AtomicString selector,
                        const Color& color) {
  Style style;
  style.set_color(color);
  style_sheet->AddRule(selector, style);
}

StyleSheet* GetDefaultStyleSheet() {
  CR_DEFINE_STATIC_LOCAL(StyleSheet, default_style_sheet, ());
  if (default_style_sheet.Find(StyleSelector::defaults()))
    return &default_style_sheet;

  Style default_style(Color(0, 0, 0), Color(255, 255, 255));
  default_style.set_font_family(L"Consolas, Meiryo");
  default_style.set_font_size(10);
  default_style.set_font_style(xcss::FontStyle::Normal);
  default_style.set_font_weight(xcss::FontWeight::Normal);
  default_style_sheet.AddRule(StyleSelector::defaults(), default_style);

  Style marker_style;
  marker_style.set_color(Color(0x00, 0x99, 0x00));
  default_style_sheet.AddRule(StyleSelector::end_of_file_marker(),
                              marker_style);
  default_style_sheet.AddRule(StyleSelector::end_of_line_marker(),
                              marker_style);
  default_style_sheet.AddRule(StyleSelector::line_wrap_marker(), marker_style);
  default_style_sheet.AddRule(StyleSelector::tab_marker(), marker_style);

  // EvEdit 1.0's highlight color
  // selection_->SetColor(Color(0, 0, 0));
  // selection_->SetBackground(Color(0xCC, 0xCC, 0xFF));

  // We should not use GetSysColor. If we want to use here
  // default background must be obtained from GetSysColor.
  // selection_->SetColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
  // selection_->SetBackground(::GetSysColor(COLOR_HIGHLIGHT));

  // We use Vista's highlight color.
  Style active_selection;
#if USE_SELETION_OVERLAY
  active_selection.set_bgcolor(Color(51, 153, 255, 0.3f));
#else
  active_selection.set_color(Color(255, 255, 255));
  active_selection.set_bgcolor(Color(51, 153, 255));
#endif
  default_style_sheet.AddRule(StyleSelector::active_selection(),
                              active_selection);
  Style inactive_selection;
#if USE_SELETION_OVERLAY
  inactive_selection.set_bgcolor(Color(191, 205, 219, 0.3f));
#else
  inactive_selection.set_color(Color(67, 78, 84));
  inactive_selection.set_bgcolor(Color(191, 205, 219));
#endif
  default_style_sheet.AddRule(StyleSelector::inactive_selection(),
                              inactive_selection);

  // Spelling
  Style red_wave;
  red_wave.set_text_decoration_color(Color(255, 0, 0));
  red_wave.set_text_decoration_line(xcss::TextDecorationLine::Underline);
  red_wave.set_text_decoration_style(xcss::TextDecorationStyle::Wavy);
  default_style_sheet.AddRule(StyleSelector::misspelled(), red_wave);

  Style green_wave;
  green_wave.set_text_decoration_color(Color(0, 255, 0));
  green_wave.set_text_decoration_line(xcss::TextDecorationLine::Underline);
  green_wave.set_text_decoration_style(xcss::TextDecorationStyle::Wavy);
  default_style_sheet.AddRule(StyleSelector::bad_grammar(), green_wave);

  // Syntax
  InstallSyntaxColor(&default_style_sheet, StyleSelector::comment(),
                     Color(0, 128, 0));
  InstallSyntaxColor(&default_style_sheet, StyleSelector::html_attribute_name(),
                     Color(255, 0, 0));
  InstallSyntaxColor(&default_style_sheet,
                     StyleSelector::html_attribute_value(), Color(0, 0, 255));
  InstallSyntaxColor(&default_style_sheet, StyleSelector::html_comment(),
                     Color(0, 128, 0));
  InstallSyntaxColor(&default_style_sheet, StyleSelector::html_element_name(),
                     Color(128, 0, 0));
  InstallSyntaxColor(&default_style_sheet, StyleSelector::html_entity(),
                     Color(255, 0, 0));
  InstallSyntaxColor(&default_style_sheet, StyleSelector::identifier(),
                     Color(0, 0, 0));
  InstallSyntaxColor(&default_style_sheet, StyleSelector::keyword(),
                     Color(0, 0, 255));
  InstallSyntaxColor(&default_style_sheet, StyleSelector::keyword2(),
                     Color(0, 0, 192));
  InstallSyntaxColor(&default_style_sheet, StyleSelector::label(),
                     Color(128, 0, 128));
  InstallSyntaxColor(&default_style_sheet, StyleSelector::literal(),
                     Color(0, 0, 128));
  InstallSyntaxColor(&default_style_sheet, StyleSelector::operators(),
                     Color(0, 0, 128));
  InstallSyntaxColor(&default_style_sheet, StyleSelector::string_literal(),
                     Color(163, 21, 21));

  // IME
  {
    Style style;
    style.set_text_decoration_line(TextDecorationLine::Underline);
    style.set_text_decoration_style(TextDecorationStyle::Solid);
    default_style_sheet.AddRule(StyleSelector::ime_inactive1(), style);
  }
  {
    Style style;
    style.set_text_decoration_line(TextDecorationLine::Underline);
    style.set_text_decoration_style(TextDecorationStyle::Solid);
    default_style_sheet.AddRule(StyleSelector::ime_inactive2(), style);
  }
  {
    Style style;
    style.set_text_decoration_line(TextDecorationLine::Underline);
    style.set_text_decoration_style(TextDecorationStyle::Wavy);
    default_style_sheet.AddRule(StyleSelector::ime_input(), style);
  }
  {
    Style style;
    style.set_text_decoration_line(TextDecorationLine::Underline);
    style.set_text_decoration_style(TextDecorationStyle::Double);
    default_style_sheet.AddRule(StyleSelector::ime_active1(), style);
  }
  {
    Style style;
    style.set_bgcolor(Color(51, 153, 255));
    style.set_color(Color(255, 255, 255));
    default_style_sheet.AddRule(StyleSelector::ime_active2(), style);
  }

  // Bracket matching
  {
    Style style;
    style.set_bgcolor(Color(238, 255, 65));
    default_style_sheet.AddRule(base::AtomicString(L"bracket"), style);
  }

  return &default_style_sheet;
}
}  // namespace

StyleResolver::StyleResolver() {
  AddStyleSheet(GetDefaultStyleSheet());
}

StyleResolver::~StyleResolver() {
  for (auto style_sheet : style_sheets_) {
    style_sheet->RemoveObserver(this);
  }
}

void StyleResolver::AddStyleSheet(const StyleSheet* style_sheet) {
  style_sheets_.push_back(style_sheet);
  style_sheet->AddObserver(this);
  ClearCache();
}

void StyleResolver::ClearCache() {
  partial_style_cache_.clear();
  style_cache_.clear();
}

void StyleResolver::InvalidateCache(const StyleRule* rule) {
  if (rule->selector() == StyleSelector::defaults()) {
    ClearCache();
  } else {
    const auto& it = style_cache_.find(rule->selector());
    if (it != style_cache_.end())
      style_cache_.erase(it);

    auto it2 = partial_style_cache_.find(rule->selector());
    if (it2 != partial_style_cache_.end())
      partial_style_cache_.erase(it);
  }
}

void StyleResolver::RemoveStyleSheet(const StyleSheet* style_sheet) {
  const auto& it =
      std::find(style_sheets_.begin(), style_sheets_.end(), style_sheet);
  if (it != style_sheets_.end()) {
    style_sheets_.erase(it);
    style_sheet->RemoveObserver(this);
  }
  ClearCache();
}

const Style& StyleResolver::Resolve(const base::string16& selector) const {
  return Resolve(base::AtomicString(selector));
}

const Style& StyleResolver::Resolve(base::AtomicString selector) const {
  const auto cache = style_cache_.find(selector);
  if (cache != style_cache_.end())
    return *cache->second;

  auto style = ResolveWithoutDefaults(selector);
  if (selector != StyleSelector::defaults())
    style.Merge(Resolve(StyleSelector::defaults()));
  auto new_style = std::make_unique<Style>(style);
  const auto new_style_ptr = new_style.get();
  style_cache_[selector] = std::move(new_style);
  return *new_style_ptr;
}

const Style& StyleResolver::ResolveWithoutDefaults(
    const base::string16& selector) const {
  return ResolveWithoutDefaults(base::AtomicString(selector));
}

const Style& StyleResolver::ResolveWithoutDefaults(
    base::AtomicString selector) const {
  const auto cache = partial_style_cache_.find(selector);
  if (cache != partial_style_cache_.end())
    return *cache->second;

  auto new_style = std::make_unique<Style>();
  for (auto style_sheet : style_sheets_) {
    if (const auto style = style_sheet->Find(selector))
      new_style->OverrideBy(*style);
  }
  const auto new_style_ptr = new_style.get();
  style_cache_[selector] = std::move(new_style);
  return *new_style_ptr;
}

// xcss::StyleSheetObserver
void StyleResolver::DidAddRule(const StyleRule* rule) {
  InvalidateCache(rule);
}

void StyleResolver::DidRemoveRule(const StyleRule* rule) {
  InvalidateCache(rule);
}

}  // namespace xcss
