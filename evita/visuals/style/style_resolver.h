// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_STYLE_STYLE_RESOLVER_H_
#define EVITA_VISUALS_STYLE_STYLE_RESOLVER_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "base/macros.h"
#include "base/observer_list.h"
#include "evita/visuals/css/media_observer.h"
#include "evita/visuals/css/style_sheet_observer.h"
#include "evita/visuals/dom/document_observer.h"

namespace visuals {

namespace css {
class Media;
class Style;
class StyleSheet;
}

class CompiledStyleSheet;
class Document;
class Element;
class Node;
class StyleChangeObserver;

//////////////////////////////////////////////////////////////////////
//
// StyleResolver
//
class StyleResolver final : public css::MediaObserver,
                            public css::StyleSheetObserver,
                            public DocumentObserver {
 public:
  explicit StyleResolver(const Document& document,
                         const css::Media& media,
                         const std::vector<css::StyleSheet*>& style_sheets);
  ~StyleResolver() final;

  const css::Style& default_style() const { return *default_style_; }
  const Document& document() const { return document_; }
  const css::Media& media() const { return media_; }

  void AddObserver(StyleChangeObserver* observer) const;
  const css::Style& ResolveFor(const Node& node);
  void RemoveObserver(StyleChangeObserver* observer) const;

 private:
  void Clear();
  const css::Style& InlineStyleOf(const Element& element) const;
  std::unique_ptr<css::Style> ComputeStyleFor(const Element& element);

  // css::MediaObserver
  void DidChangeViewportSize() final;
  void DidChangeSystemMetrics() final;

  // css::StyleSheetObserver
  void DidAddRule(const css::Rule& rule) final;
  void DidRemoveRule(const css::Rule& rule) final;

  // DocumentObserver
  void DidChangeInlineStyle(const Element& element,
                            const css::Style* old_style) final;
  void WillRemoveChild(const ContainerNode& parent, const Node& child) final;

  std::vector<std::unique_ptr<CompiledStyleSheet>> compiled_style_sheets_;
  std::unique_ptr<css::Style> default_style_;
  const Document& document_;
  const css::Media& media_;
  mutable base::ObserverList<StyleChangeObserver> observers_;
  // TODO(eval1749): We should share |css::Style| objects for elements which
  // have same style.
  std::unordered_map<const Element*, std::unique_ptr<css::Style>> style_map_;
  const std::vector<css::StyleSheet*> style_sheets_;

  DISALLOW_COPY_AND_ASSIGN(StyleResolver);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_STYLE_STYLE_RESOLVER_H_