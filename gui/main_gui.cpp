#ifndef _WIN32_IE
#define _WIN32_IE 0x0500
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#define WIN32_LEAN_AND_MEAN
#define _PRSHT_H_

#include <windows.h>
// HPROPSHEETPAGE is defined in prsht.h, which we skipped.
// commctrl.h needs it.
typedef void *HPROPSHEETPAGE;

#include "gui_redirect.h"
#include <commctrl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// Include aulon headers
extern "C" {
#include "../src/defs.h"
#include "../src/menu_func.h"
}

// Global Variables
HWND hLogWindow = NULL;
HWND hMainWindow = NULL;
HFONT hFont = NULL;

// Control IDs
#define IDC_LOG_WINDOW 1001
#define IDC_BTN_CONNECT 1002
#define IDC_BTN_BBID 1003
#define IDC_BTN_FILES 1004
#define IDC_BTN_DUMP 1005
#define IDC_BTN_EXIT 1006

// Redirect implementation
// Redirect implementation
extern "C" {
// Custom message for logging
#define WM_GUI_LOG (WM_USER + 1)

void GuiLogV(const char *format, va_list args) {
  if (!hLogWindow)
    return;

  char buffer[4096];
  vsnprintf(buffer, sizeof(buffer), format, args);

  // Convert newlines (\n -> \r\n)
  char *finalBuffer = (char *)malloc(8192);
  if (!finalBuffer)
    return;

  char *s = buffer;
  char *d = finalBuffer;
  while (*s && (d - finalBuffer < 8190)) {
    if (*s == '\n') {
      *d++ = '\r';
    }
    *d++ = *s++;
  }
  *d = 0;

  // If we are not on the main thread, we must PostMessage to avoid deadlocks or
  // unsafe access
  if (GetCurrentThreadId() != GetWindowThreadProcessId(hMainWindow, NULL)) {
    // Use SendMessageTimeout or just PostMessage with a dynamic buffer?
    // For simplicity, let's use a temporary heap buffer and let the main thread
    // free it. This is a simple way to pass string data across threads.
    PostMessage(hMainWindow, WM_GUI_LOG, 0, (LPARAM)finalBuffer);
  } else {
    // Direct update if on main thread
    int len = GetWindowTextLength(hLogWindow);
    SendMessage(hLogWindow, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessage(hLogWindow, EM_REPLACESEL, FALSE, (LPARAM)finalBuffer);
    SendMessage(hLogWindow, WM_VSCROLL, SB_BOTTOM, 0);
    free(finalBuffer);
  }
}

void GuiLog(const char *format, ...) {
  va_list args;
  va_start(args, format);
  GuiLogV(format, args);
  va_end(args);
}
}

// Thread Function
DWORD WINAPI ConnectThread(LPVOID lpParam) {
  GuiLog("Thread: Initializing connection...\n");
  if (Init()) {
    GuiLog("Thread: Init Success!\n");
    MessageBox(hMainWindow, "Connection Established!", "Success", MB_OK);
  } else {
    GuiLog("Thread: Init Failed.\n");
    MessageBox(hMainWindow, "Connection Failed.", "Error", MB_ICONERROR);
  }
  // Re-enable button
  EnableWindow(GetDlgItem(hMainWindow, IDC_BTN_CONNECT), TRUE);
  return 0;
}

// Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_CREATE: {
    // CreateFont ...
    hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    // Buttons
    int y = 10;
    HWND hBtn;

    hBtn = CreateWindow("BUTTON", "Connect (Init)",
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, y, 120, 30,
                        hwnd, (HMENU)IDC_BTN_CONNECT, NULL, NULL);
    y += 35;

    hBtn = CreateWindow("BUTTON", "Get BBID",
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, y, 120, 30,
                        hwnd, (HMENU)IDC_BTN_BBID, NULL, NULL);
    y += 35;

    hBtn = CreateWindow("BUTTON", "List Files",
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, y, 120, 30,
                        hwnd, (HMENU)IDC_BTN_FILES, NULL, NULL);
    y += 35;

    hBtn = CreateWindow("BUTTON", "Dump NAND",
                        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, y, 120, 30,
                        hwnd, (HMENU)IDC_BTN_DUMP, NULL, NULL);
    y += 35;

    hBtn = CreateWindow("BUTTON", "Exit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                        10, y, 120, 30, hwnd, (HMENU)IDC_BTN_EXIT, NULL, NULL);
    y += 35;

    // Log Window
    hLogWindow = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
                                WS_CHILD | WS_VISIBLE | WS_VSCROLL |
                                    ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                                140, 10, 430, 340, hwnd, (HMENU)IDC_LOG_WINDOW,
                                NULL, NULL);
    SendMessage(hLogWindow, WM_SETFONT, (WPARAM)GetStockObject(ANSI_FIXED_FONT),
                TRUE);

    GuiLog("iQue@home initialized.\r\nReady to connect.\r\n");
    break;
  }
  case WM_COMMAND: {
    switch (LOWORD(wParam)) {
    case IDC_BTN_CONNECT:
      GuiLog("Starting connection thread...\n");
      // Disable button to prevent double clicks (TODO: Enable later)
      EnableWindow(GetDlgItem(hwnd, IDC_BTN_CONNECT), FALSE);
      CreateThread(NULL, 0, ConnectThread, NULL, 0, NULL);
      break;
    case IDC_BTN_BBID:
      GetBBID();
      break;
    case IDC_BTN_FILES:
      ListFiles();
      break;
    case IDC_BTN_DUMP:
      GuiLog("Starting NAND dump... This may take a while.\n");
      UpdateWindow(hLogWindow);
      DumpNand();
      break;
    case IDC_BTN_EXIT:
      PostQuitMessage(0);
      break;
    }
    break;
  }
  case WM_GUI_LOG: {
    char *msg = (char *)lParam;
    if (msg) {
      int len = GetWindowTextLength(hLogWindow);
      SendMessage(hLogWindow, EM_SETSEL, (WPARAM)len, (LPARAM)len);
      SendMessage(hLogWindow, EM_REPLACESEL, FALSE, (LPARAM)msg);
      SendMessage(hLogWindow, WM_VSCROLL, SB_BOTTOM, 0);
      free(msg);
    }
    break;
  }
  case WM_CLOSE:
    DestroyWindow(hwnd);
    break;
  case WM_DESTROY:
    Close(); // Ensure connection is closed
    PostQuitMessage(0);
    if (hFont)
      DeleteObject(hFont);
    break;
  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
  INITCOMMONCONTROLSEX icex;
  icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icex.dwICC = ICC_STANDARD_CLASSES;
  InitCommonControlsEx(&icex);

  WNDCLASSEX wc;
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = 0;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = "iQueHomeClass";
  wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

  if (!RegisterClassEx(&wc)) {
    MessageBox(NULL, "Window Registration Failed!", "Error",
               MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }

  hMainWindow = CreateWindowEx(
      0, "iQueHomeClass", "iQue@home",
      WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT,
      CW_USEDEFAULT, 600, 400, NULL, NULL, hInstance, NULL);

  if (hMainWindow == NULL) {
    MessageBox(NULL, "Window Creation Failed!", "Error",
               MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }

  ShowWindow(hMainWindow, nCmdShow);
  UpdateWindow(hMainWindow);

  MSG Msg;
  while (GetMessage(&Msg, NULL, 0, 0) > 0) {
    TranslateMessage(&Msg);
    DispatchMessage(&Msg);
  }
  return Msg.wParam;
}
