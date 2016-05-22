// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_STYLE_MODELS_STYLE_SHEET_OBSERVER_H_
#define EVITA_TEXT_STYLE_MODELS_STYLE_SHEET_OBSERVER_H_

#include "base/macros.h"

namespace xcss {

class StyleRule;

class StyleSheetObserver {
 public:
  virtual void DidAddRule(const StyleRule* rule) = 0;
  virtual void DidRemoveRule(const StyleRule* rule) = 0;

 protected:
  StyleSheetObserver();
  virtual ~StyleSheetObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(StyleSheetObserver);
};

}  // namespace xcss

#endif  // EVITA_TEXT_STYLE_MODELS_STYLE_SHEET_OBSERVER_H_
