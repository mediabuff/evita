#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - Editor - Application Main
// listener/winapp/ap_main.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ap_main.cpp#3 $
//
// Example options:
//  -dll vanilla.dll -image evcl3.image -multiple
//
#define DEBUG_BUSY 0
#define DEBUG_IDLE 0

#include "base/at_exit.h"
#include "base/bind.h"
#include "base/command_line.h"
#include "base/location.h"
#include "base/logging.h"
#pragma warning(push)
#pragma warning(disable: 4100)
#include "base/message_loop/message_loop.h"
#pragma warning(pop)
#include "base/run_loop.h"
#include "common/win/native_window.h"

#if USE_LISTENER
    #include "./ap_listener_buffer.h"
    #define SINGLE_INSTANCE_NAME    L"3C9C7EC2-0DE7-461e-8F09-14F3B830413E" 
#else // USE_LISTENER
    #include "./vi_buffer.h"
    #define SINGLE_INSTANCE_NAME    L"D47A7677-9F8E-467c-BABE-8ABDE8D58476" 
#endif // USE_LISTENER

#include "./ctrl_TabBand.h"

#include "evita/editor/application.h"
#include "./vi_IoManager.h"
#include "./vi_Style.h"
#include "./vi_TextEditWindow.h"

#if _WIN64
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='X86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

const char16* k_pwszTitle = L"Evita Common Lisp Listner";

extern HINSTANCE   g_hInstance;
extern HINSTANCE   g_hResource;
extern HWND        g_hwndActiveDialog;
extern uint        g_TabBand__TabDragMsg;

extern StyleValues g_DefaultStyle;

static void NoReturn fatalExit(const char16*);


static int CallRunningApp() {
  Handle shShared = ::OpenFileMapping(
      FILE_MAP_READ | FILE_MAP_WRITE,
      FALSE,
      k_wszFileMapping);
  if (!shShared)
      fatalExit(L"OpenFileMapping");

  auto const * p = reinterpret_cast<SharedArea*>(
      ::MapViewOfFile(shShared,
                      FILE_MAP_READ | FILE_MAP_WRITE,
                      0,      // dwFileOffsetHigh
                      0,      // dwFileOffsetLow
                      k_cbFileMapping));
  if (!p)
    fatalExit(L"MapViewOfFile");

  for (auto param: CommandLine::ForCurrentProcess()->GetArgs()) {
    char16 wsz[MAX_PATH];
    char16* pwszFile;
    auto const cwch = ::GetFullPathName(param.c_str(), lengthof(wsz), wsz,
                                        &pwszFile);
    if (!cwch || cwch > lengthof(wsz))
     continue;

    COPYDATASTRUCT oData;
    oData.dwData = 1;
    oData.cbData = sizeof(char16) * (cwch + 1);
    oData.lpData = wsz;

    ::SendMessage(p->m_hwnd, WM_COPYDATA, 0,
                  reinterpret_cast<LPARAM>(&oData));
  }
  return 0;
}

static void NoReturn fatalExit(const char16* pwsz) {
  char16 wsz[100];
  ::wsprintf(wsz, L"Evita Text Editor can't start (%s).", pwsz);
  ::FatalAppExit(0, wsz);
}

static void DoIdle() {
  static int idle_count;
  #if DEBUG_IDLE
    DVLOG(4) << "idle_count=" << idle_count << " running=" <<
        base::MessageLoop::current()->is_running();
  #endif
  if (Application::Get()->OnIdle(idle_count))
    ++idle_count;
  else
    idle_count= 0;
  if (!Application::instance().is_quit())
    base::MessageLoop::current()->PostTask(FROM_HERE, base::Bind(DoIdle));
}

static int MainLoop() {
  // Initialize Default Style
  // This initialize must be before creating edit buffers.
  {
      g_DefaultStyle.m_rgfMask =
          StyleValues::Mask_Background |
          StyleValues::Mask_Color |
          StyleValues::Mask_Decoration |
          StyleValues::Mask_FontFamily |
          StyleValues::Mask_FontStyle |
          StyleValues::Mask_FontWeight |
          StyleValues::Mask_Marker |
          StyleValues::Mask_Syntax;

        #if 0
          //g_DefaultStyle->SetBackground(Color(0xF0, 0xF0, 0xF0));
          g_DefaultStyle->SetBackground(Color(247, 247, 239));
          g_DefaultStyle->SetColor(Color(0x00, 0x00, 0x00));
          g_DefaultStyle->SetMarker(Color(0x00, 0x66, 0x00));
      #else
          g_DefaultStyle.m_crBackground = Color(255, 255, 255);
          g_DefaultStyle.m_crColor      = Color(0x00, 0x00, 0x00);
          g_DefaultStyle.m_crMarker     = Color(0x00, 0x99, 0x00);
      #endif

        //#define BaseFont L"Lucida Console"
      //#define BaseFont L"Courier New"
      #define BaseFont L"Consolas, MS Gothic"

        {
          //FontSet* pFontSet = new FontSet;
          //pFontSet->Add(Font::Create(BaseFont, nFontSize, ANSI_CHARSET));
          //pFontSet->Add(Font::Create(L"Courier New", nFontSize, ANSI_CHARSET));
          //pFontSet->Add(Font::Create(L"MS Gothic", nFontSize, SHIFTJIS_CHARSET));
          g_DefaultStyle.m_pwszFontFamily = BaseFont;
          g_DefaultStyle.m_nFontSize      = 10;
      }
  }

  auto& frame = *Application::Get()->CreateFrame();
  for (auto const filename: CommandLine::ForCurrentProcess()->GetArgs()) {
    auto const buffer = Application::Get()->Load(filename.c_str());
    frame.AddWindow(new TextEditWindow(buffer));
  }

  // When there is no filename argument, we start lisp.
  if (!frame.GetFirstPane()) {
    #if USE_LISTENER && _NDEBUG
      auto const buffer = new ListenerBuffer();
      buffer->Start();
    #else // USE_LISTENER
      auto const buffer = new Buffer(L"*scratch*");
    #endif // USE_LISTENER

    Application::Get()->InternalAddBuffer(buffer);
    frame.AddWindow(new TextEditWindow(buffer));
  }
  frame.Realize();

  base::MessageLoop message_loop(base::MessageLoop::TYPE_UI);
  DoIdle();
  base::RunLoop run_loop;
  run_loop.Run();
  return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
  base::AtExitManager at_exit;
  CommandLine::Init(0, nullptr);
  {
    logging::LoggingSettings settings;
    settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
    logging::InitLogging(settings);
  }

  common::win::NativeWindow::Init(hInstance);
  g_hInstance = hInstance;
  g_hResource = hInstance;

  auto const command_line = CommandLine::ForCurrentProcess();
  g_fMultiple = command_line->HasSwitch("multiple");

  if (!g_fMultiple) {
    g_hEvent = ::CreateEventW(nullptr,   // lpEventAttrs
                              TRUE,   // fManualReset
                              FALSE,  // fInitialState
                              L"Local\\" SINGLE_INSTANCE_NAME);

      if (!g_hEvent)
      fatalExit(L"CreateEvent");

      if (::GetLastError() == ERROR_ALREADY_EXISTS) {
      auto const nWait = ::WaitForSingleObject(g_hEvent, 30 * 100);
      switch (nWait) {
        case WAIT_OBJECT_0:
          return CallRunningApp();

          default:
          ::MessageBox(nullptr, L"WaitForSignleObject", k_pwszTitle,
                       MB_APPLMODAL | MB_ICONERROR);
          break;
      }
    }
  }

  // Common Control
  {
      INITCOMMONCONTROLSEX oInit;
      oInit.dwSize = sizeof(oInit);
      oInit.dwICC  = ICC_BAR_CLASSES;
      if (!::InitCommonControlsEx(&oInit)) {
          ::MessageBox(
              nullptr,
              L"InitCommonControlsEx",
              k_pwszTitle,
              MB_APPLMODAL | MB_ICONERROR);
          return 1;
      }
  }

    ::TabBand__Init(g_hInstance);

    g_TabBand__TabDragMsg = ::RegisterWindowMessage(TabBand__TabDragMsgStr);

  return MainLoop();
}
