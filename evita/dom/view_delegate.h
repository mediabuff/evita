// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_view_delegate_h)
#define INCLUDE_evita_dom_view_delegate_h

#include <vector>

#include "base/callback_forward.h"
#include "base/strings/string16.h"
#include "evita/dom/forms/dialog_box_id.h"
#include "evita/dom/window_id.h"
#include "evita/dom/window_id.h"
#include "evita/text/search_and_replace_model.h"

namespace base {
class WaitableEvent;
}

namespace dom {

class Buffer;
class Document;
class EditorWindow;
class Form;
class TextWindow;
class ViewEventHandler;
class Window;

typedef EventTargetId DialogBoxId;

struct TextWindowCompute {
  // Note: Value of |WindowCompute::Method| must match with JavaScript.
  enum class Method {
    EndOfWindow,
    EndOfWindowLine,
    MapPointToPosition,
    MapPositionToPoint,
    MoveScreen,
    MoveWindow,
    MoveWindowLine,
    StartOfWindow,
    StartOfWindowLine,
  };
  Method method;
  text::Posn position;
  int count;
  float x;
  float y;
};

class ViewDelegate {
  public: typedef base::Callback<void(base::string16 filename)>
      GetFilenameForLoadCallback;

  public: typedef base::Callback<void(base::string16 filename)>
      GetFilenameForSaveCallback;

  public: typedef base::Callback<void(int response_code)>
      MessageBoxCallback;

  public: ViewDelegate() = default;
  public: virtual ~ViewDelegate() = default;

  public: virtual void AddWindow(WindowId parent_id, WindowId child_id) = 0;
  public: virtual void ChangeParentWindow(WindowId window_id,
                                          WindowId new_parent_window_id) = 0;
  public: virtual void ComputeOnTextWindow(WindowId window_id,
                                         TextWindowCompute* data,
                                         base::WaitableEvent* event) = 0;
  public: virtual void CreateDialogBox(DialogBoxId dialog_box_id) = 0;
  public: virtual void CreateEditorWindow(const EditorWindow* window) = 0;
  public: virtual void CreateTableWindow(WindowId window_id,
                                         Document* document) = 0;
  public: virtual void CreateTextWindow(const TextWindow* window) = 0;
  public: virtual void DestroyWindow(WindowId window_id) = 0;
  public: virtual void DoFind(DialogBoxId dialog_box_id,
                              text::Direction direction) = 0;
  public: virtual void FocusWindow(WindowId window_id) = 0;
  public: virtual void GetFilenameForLoad(
      WindowId window_id, const base::string16& dir_path,
      GetFilenameForLoadCallback callback) = 0;
  public: virtual void GetFilenameForSave(
      WindowId window_id, const base::string16& dir_path,
      GetFilenameForSaveCallback callback) = 0;
  public: virtual void GetTableRowStates(WindowId window_id,
      const std::vector<base::string16>& keys, int* states,
      base::WaitableEvent* event) = 0;
  public: virtual void LoadFile(Document* document,
                                const base::string16& filename) = 0;

  public: virtual void MakeSelectionVisible(WindowId window_id) = 0;
  public: virtual void MessageBox(WindowId window_id,
      const base::string16& message, const base::string16& title, int flags,
      MessageBoxCallback callback) = 0;
  public: virtual void Reconvert(WindowId window_id, text::Posn start,
                                 text::Posn end) = 0;
  public: virtual void RealizeDialogBox(const Form* form) = 0;
  public: virtual void RealizeWindow(WindowId window_id) = 0;
  public: virtual void RegisterViewEventHandler(
      ViewEventHandler* event_handler) = 0;
  public: virtual void ReleaseCapture(WindowId window_id) = 0;
  public: virtual void ReloadTextBuffer(dom::Buffer* buffer) = 0;
  public: virtual void SaveFile(Document* document,
                                const base::string16& filename) = 0;
  public: virtual void SetCapture(WindowId window_id) = 0;
  public: virtual void ShowDialogBox(DialogBoxId dialog_box_id) = 0;
  public: virtual void SplitHorizontally(WindowId left_window_id,
                                         WindowId new_right_window_id) = 0;
  public: virtual void SplitVertically(WindowId above_window_id,
                                       WindowId new_below_window_id) = 0;

  DISALLOW_COPY_AND_ASSIGN(ViewDelegate);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_view_delegate_h)
