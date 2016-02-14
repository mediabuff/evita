// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

namespace dom {

class ComponentsTest : public AbstractDomTest {
 public:
  ~ComponentsTest() override = default;

 protected:
  ComponentsTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(ComponentsTest);
};

TEST_F(ComponentsTest, Imaging) {
  RunFile({"components", "imaging", "imaging_test.js"});
}

}  // namespace dom
