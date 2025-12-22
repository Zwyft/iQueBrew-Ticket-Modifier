
// Thread-Safe Logging Implementation for inclusion in multiple GUI files
// Avoids code duplication between main_gui.cpp and modern_gui.cpp

extern "C" {
// Custom message for logging
#ifndef WM_GUI_LOG
#define WM_GUI_LOG (WM_USER + 1)
#endif

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

  // If we are not on the main thread, we must PostMessage
  if (GetCurrentThreadId() != GetWindowThreadProcessId(hMainWindow, NULL)) {
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
