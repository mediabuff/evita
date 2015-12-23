// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "gmock/gmock.h"
#include "evita/dom/testing/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/script_host.h"

namespace dom {

using ::testing::Eq;
using ::testing::_;

class TextWindowTest : public AbstractDomTest {
 public:
  ~TextWindowTest() override = default;

 protected:
  TextWindowTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(TextWindowTest);
};

TEST_F(TextWindowTest, _ctor) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_, _));
  EXPECT_SCRIPT_VALID(
      "var doc = Document.new('foo');"
      "var range = new Range(doc);"
      "var sample = new TextWindow(range);");
  EXPECT_SCRIPT_TRUE("sample instanceof TextWindow");
  EXPECT_SCRIPT_TRUE("sample instanceof Window");
  EXPECT_SCRIPT_EQ("1", "sample.id");
  EXPECT_SCRIPT_TRUE("sample.document == doc");
  EXPECT_SCRIPT_TRUE("sample.selection instanceof Selection");
}

TEST_F(TextWindowTest, clone) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_, _)).Times(2);
  EXPECT_SCRIPT_VALID(
      "var original = new TextWindow(new Range(Document.new('foo')));"
      "original.selection.range.text = 'foo';"
      "original.selection.range.end = 3;"
      "var sample = original.clone();");
  EXPECT_SCRIPT_EQ("3", "sample.selection.range.end");
}

TEST_F(TextWindowTest, makeSelectionVisible) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_, _));
  EXPECT_CALL(*mock_view_impl(), MakeSelectionVisible(Eq(1)));
  EXPECT_SCRIPT_VALID(
      "var sample = new TextWindow(new Range(Document.new('foo')));"
      "sample.makeSelectionVisible();");
}

TEST_F(TextWindowTest, realize) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_, _));
  EXPECT_SCRIPT_VALID(
      "var doc = Document.new('foo');"
      "var sample = new TextWindow(new Range(doc));"
      "var event;"
      "function event_handler(x) { event = x; }"
      "doc.addEventListener(Event.Names.ATTACH, event_handler);"
      "doc.addEventListener(Event.Names.DETACH, event_handler);");

  // The document receives "attach" event when |TextWindow| is realized.
  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  EXPECT_SCRIPT_VALID("sample.realize()");
  view_event_handler()->DidRealizeWidget(1);
  RunMessageLoopUntilIdle();

  EXPECT_SCRIPT_TRUE("event instanceof DocumentEvent");
  EXPECT_SCRIPT_EQ("attach", "event.type");
  EXPECT_SCRIPT_TRUE("event.view === sample");

  // The document receives "detach" event when |TextWindow| is destroyed.
  EXPECT_CALL(*mock_view_impl(), DestroyWindow(Eq(1)));
  EXPECT_SCRIPT_VALID("sample.destroy()");
  view_event_handler()->DidDestroyWidget(1);
  RunMessageLoopUntilIdle();

  EXPECT_SCRIPT_TRUE("event instanceof DocumentEvent");
  EXPECT_SCRIPT_EQ("detach", "event.type");
  EXPECT_SCRIPT_TRUE("event.view === sample");
}

TEST_F(TextWindowTest, zoom) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_, _));
  EXPECT_CALL(*mock_view_impl(), SetTextWindowZoom(Eq(1), Eq(1.5f)));
  EXPECT_SCRIPT_VALID(
      "var sample = new TextWindow(new Range(Document.new('foo')));"
      "sample.zoom = 1.5;");
  EXPECT_SCRIPT_EQ("1.5", "sample.zoom");
  EXPECT_SCRIPT_EQ("RangeError: TextWindow zoom must be greater than zero.",
                   "sample.zoom = 0;");
  EXPECT_SCRIPT_EQ("RangeError: TextWindow zoom must be greater than zero.",
                   "sample.zoom = -1;");
}

}  // namespace dom
