// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "base/strings/utf_string_conversions.h"
#include "gmock/gmock.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/buffer.h"
#include "evita/dom/document.h"
#include "evita/dom/file_path.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"

namespace {

using ::testing::Eq;
using ::testing::_;

class DocumentTest : public dom::AbstractDomTest {
  protected: DocumentTest() {
  }
  public: virtual ~DocumentTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(DocumentTest);
};

TEST_F(DocumentTest, Constructor) {
  // TODO(yosi): We should remove all buffers for each test case.
  EXPECT_SCRIPT_EQ("bar", "var sample1 = new Document('bar');"
                          "sample1.name");
  EXPECT_SCRIPT_EQ("bar (2)",
                   "var sample2 = new Document('bar'); sample2.name");
  EXPECT_SCRIPT_EQ("bar (3)",
                   "var sample2 = new Document('bar'); sample2.name");

  RunScript("new Document('bar.cc')");
  EXPECT_SCRIPT_EQ("bar (2).cc",
                   "var sample2 = new Document('bar.cc'); sample2.name");
  EXPECT_SCRIPT_EQ("bar (3).cc",
                   "var sample2 = new Document('bar.cc'); sample2.name");

  RunScript("new Document('.bar')");
  EXPECT_SCRIPT_EQ(".bar (2)",
                   "var sample2 = new Document('.bar'); sample2.name");
  EXPECT_SCRIPT_EQ(".bar (3)",
                   "var sample2 = new Document('.bar'); sample2.name");
}

TEST_F(DocumentTest, Document_list) {
  RunScript("['foo', 'bar', 'baz'].forEach(function(name) {"
            "  new Document(name);"
            "});"
            "var samples = Document.list.sort(function(a, b) {"
            "  return a.name.localeCompare(b.name);"
            "});");
  EXPECT_SCRIPT_EQ("3", "samples.length");
  EXPECT_SCRIPT_EQ("bar", "samples[0].name");
  EXPECT_SCRIPT_EQ("baz", "samples[1].name");
  EXPECT_SCRIPT_EQ("foo", "samples[2].name");
}

TEST_F(DocumentTest, Document_load) {
  EXPECT_CALL(*mock_view_impl(),
              LoadFile(_, Eq(dom::FilePath::FullPath(L"foo"))));
  EXPECT_CALL(*mock_view_impl(),
              LoadFile(_, Eq(dom::FilePath::FullPath(L"bar"))));
  RunScript("var a = Document.load('foo');");
  auto const document = dom::Document::Find(L"foo");
  auto const absoulte_filename = dom::FilePath::FullPath(L"foo");
  document->buffer()->SetFile(absoulte_filename, FileTime());
  RunScript("var b = Document.load('foo');"
            "var c = Document.load('bar');");
  EXPECT_SCRIPT_EQ(base::UTF16ToUTF8(absoulte_filename), "a.filename");
  EXPECT_SCRIPT_TRUE("a === b");
  EXPECT_SCRIPT_TRUE("a !== c");
}

TEST_F(DocumentTest, DocumentFind) {
  EXPECT_SCRIPT_TRUE("var sample1 = Document.find('foo'); sample1 == null");
  RunScript("new Document('foo')");
  EXPECT_SCRIPT_EQ("foo", "var sample2 = Document.find('foo'); sample2.name");
}

TEST_F(DocumentTest, DocumentGetOrNew) {
  RunScript("var doc = new Document('foo');");
  EXPECT_SCRIPT_TRUE("doc === Document.getOrNew('foo')");
}

TEST_F(DocumentTest, charCodeAt) {
  RunScript("var doc = new Document('foo');"
            "new Range(doc).text = 'foobar';");
  EXPECT_SCRIPT_EQ("111", "doc.charCodeAt(1)");
}

TEST_F(DocumentTest, length) {
  RunScript("var doc = new Document('length');");
  EXPECT_SCRIPT_EQ("0", "doc.length");
  RunScript("new Range(doc).text = 'foobar';");
  EXPECT_SCRIPT_EQ("6", "doc.length");
}

TEST_F(DocumentTest, load_) {
  EXPECT_CALL(*mock_view_impl(), LoadFile(_, Eq(L"foo")));
  RunScript("var doc = new Document('foo'); doc.load_('foo')");
  
}

TEST_F(DocumentTest, modified) {
  RunScript("var doc = new Document('foo');"
            "var range = new Range(doc);");
  EXPECT_SCRIPT_FALSE("doc.modified");

  RunScript("range.text = 'foo';");
  EXPECT_SCRIPT_TRUE("doc.modified");

#if 0
  // Since mock ViewDelegate doesn't reset modified flag, we disable
  // this test case.
  EXPECT_CALL(*mock_view_impl(), SaveFile(_, Eq(L"foo")));
  RunScript("doc.save('foo')");
  EXPECT_SCRIPT_FALSE("doc.modified");
#endif
}

TEST_F(DocumentTest, name) {
  EXPECT_SCRIPT_EQ("baz", "var sample1 = new Document('baz'); sample1.name");
}

TEST_F(DocumentTest, renameTo) {
  RunScript("var doc = new Document('foo'); doc.renameTo('bar')");
  EXPECT_SCRIPT_EQ("bar", "doc.name");
}

TEST_F(DocumentTest, save) {
  EXPECT_CALL(*mock_view_impl(), SaveFile(_, Eq(L"foo")));
  RunScript("var doc = new Document('foo'); doc.save('foo')");
  
}

}  // namespace
