#include "gui_redirect.h"
#include <commctrl.h>
#include <stdio.h>
#include <windows.h>


// Define colors
#define COLOR_BG_DARK RGB(13, 13, 26)   // #0d0d1a
#define COLOR_BG_PANEL RGB(26, 26, 48)  // #1a1a30
#define COLOR_ACCENT RGB(124, 108, 240) // #7c6cf0
#define COLOR_TEXT WHITE_BRUSH

// Globals
HFONT hFontTitle, hFontNormal;
HBRUSH hBrushBg, hBrushPanel;

// ... Window Procedure with WM_CTLCOLORSTATIC handling for dark mode ...
