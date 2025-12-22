#ifndef _WIN32_IE
#define _WIN32_IE 0x0500
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

// Standard includes
#include <commctrl.h>
#include <commdlg.h>
#include <objidl.h>
#include <shlobj.h>

// C++ Headers for GDI+
#include <algorithm>
namespace Gdiplus {
using std::max;
using std::min;
} // namespace Gdiplus

#include <cstdint>
#include <gdiplus.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#include "gui_redirect.h"

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "comdlg32.lib")

using namespace Gdiplus;

extern "C" {
#include "../src/defs.h"
#include "../src/fs.h"
#include "../src/menu_func.h"
}

// ================= MODERN GUI (HIGH FIDELITY PORT) =================
// Replicates the layout of ique_tools.py (CustomTkinter)

// ================= COLORS (Refined - Less Bright) =================
#define COL_BG 0x1a0d0d    // #0d0d1a
#define COL_PANEL 0x301a1a // #1a1a30
#define COL_INPUT 0x502525 // #252550
#define COL_TEXT 0xFFFFFF
// Darkened versions of original colors
#define COL_ACCENT 0xc45866   // Darker Purple
#define COL_GREEN 0x7ab500    // Darker Green
#define COL_RED 0x4737d0      // Darker Red
#define COL_BLUE 0x968413     // Darker Cyan
#define COL_ORANGE 0x008fd6   // Darker Orange
#define COL_LAVENDER 0x8e87d0 // Darker Lavender

// Ticket Handling
#define TICKET_SIZE 0x2B4C
#define TICKET_HEADER_SIZE 4
#define TICKET_OFFSET_TRIAL_TYPE 0x29B6
#define TICKET_OFFSET_TRIAL_LIMIT 0x29B8

struct ParsedTicket {
  int index;
  std::string name;
  std::string chinese_name;
  bool is_trial;
};

std::vector<ParsedTicket> loaded_tickets;

// GB2312 to UTF-8/ANSI Helper
std::string GB2312ToUTF8(const char *gb2312) {
  int len = MultiByteToWideChar(936, 0, gb2312, -1, NULL, 0); // 936 = GB2312
  if (len == 0)
    return "";

  std::vector<wchar_t> wstr(len);
  MultiByteToWideChar(936, 0, gb2312, -1, &wstr[0], len);

  int utf8_len = WideCharToMultiByte(CP_ACP, 0, &wstr[0], -1, NULL, 0, NULL,
                                     NULL); // CP_ACP for Windows ANSI (Simple)
  if (utf8_len == 0)
    return "";

  std::vector<char> str(utf8_len);
  WideCharToMultiByte(CP_ACP, 0, &wstr[0], -1, &str[0], utf8_len, NULL, NULL);

  return std::string(&str[0]);
}

uint16_t Swap16(uint16_t val) { return (val << 8) | (val >> 8); }
uint32_t Swap32(uint32_t val) {
  return (val << 24) | ((val << 8) & 0x00FF0000) | ((val >> 8) & 0x0000FF00) |
         (val >> 24);
}

void ParseTicketSys(const char *filepath) {
  loaded_tickets.clear();
  FILE *f = fopen(filepath, "rb");
  if (!f)
    return;

  // Read header (num tickets)
  uint32_t num_tickets_be;
  fread(&num_tickets_be, 4, 1, f);
  uint32_t num_tickets = Swap32(num_tickets_be);

  for (uint32_t i = 0; i < num_tickets; i++) {
    std::vector<uint8_t> buffer(TICKET_SIZE);
    fread(buffer.data(), TICKET_SIZE, 1, f);

    // Offsets for name
    uint16_t thumb_len = Swap16(*(uint16_t *)&buffer[0x44]);
    uint16_t title_len = Swap16(*(uint16_t *)&buffer[0x46]);

    int name_offset = 0x48 + thumb_len + title_len;
    if (name_offset >= TICKET_SIZE)
      continue;

    // Read Name (Null terminated or up to limit)
    std::string raw_name = "";
    for (int k = name_offset; k < 0x2800; k++) {
      if (buffer[k] == 0)
        break;
      raw_name += (char)buffer[k];
    }

    std::string english_name = "Unknown Game";

    // Match on RAW GB2312 bytes (not decoded) since decoding fails on
    // non-Chinese Windows These are the actual GB2312 byte sequences for the
    // Chinese game names Format: \xHH where HH is the hex byte value

    // 马力欧医生 (Dr. Mario 64) - GB2312 bytes
    if (raw_name.find("\xC2\xED\xC1\xA6\xC5\xB7\xD2\xBD\xC9\xFA") !=
        std::string::npos)
      english_name = "Dr. Mario 64";
    // 塞尔达的传说 (Zelda) - GB2312 bytes
    else if (raw_name.find("\xC8\xFB\xB6\xFB\xB4\xEF") != std::string::npos)
      english_name = "Zelda: Ocarina of Time";
    // 神游马力欧 (Super Mario) - GB2312 bytes
    else if (raw_name.find("\xC9\xF1\xD3\xCE\xC2\xED\xC1\xA6\xC5\xB7") !=
             std::string::npos)
      english_name = "Super Mario 64";
    // 水上摩托 (Wave Race) - GB2312 bytes
    else if (raw_name.find("\xCB\xAE\xC9\xCF\xC4\xA6\xCD\xD0") !=
             std::string::npos)
      english_name = "Wave Race 64";
    // 星际火狐 (Star Fox) - GB2312 bytes
    else if (raw_name.find("\xD0\xC7\xBC\xCA\xBB\xF0\xBA\xFC") !=
             std::string::npos)
      english_name = "Star Fox 64";
    // 耀西故事 (Yoshi's Story) - GB2312 bytes
    else if (raw_name.find("\xD2\xAB\xCE\xF7\xB9\xCA\xCA\xC2") !=
             std::string::npos)
      english_name = "Yoshi's Story";
    // 任天堂明星大乱斗 (Smash Bros) - GB2312 bytes
    else if (raw_name.find("\xC8\xCE\xCC\xEC\xCC\xC3") != std::string::npos)
      english_name = "Super Smash Bros.";
    // 纸片马力欧 (Paper Mario) - GB2312 bytes
    else if (raw_name.find("\xD6\xBD\xC6\xAC\xC2\xED\xC1\xA6\xC5\xB7") !=
             std::string::npos)
      english_name = "Paper Mario";
    // 动物森林 (Animal Crossing) - GB2312 bytes
    else if (raw_name.find("\xB6\xAF\xCE\xEF\xC9\xAD\xC1\xD6") !=
             std::string::npos)
      english_name = "Animal Crossing";
    // 组合机器人 (Custom Robo) - GB2312 bytes
    else if (raw_name.find("\xD7\xE9\xBA\xCF\xBB\xFA\xC6\xF7\xC8\xCB") !=
             std::string::npos)
      english_name = "Custom Robo";
    // 罪与罚 (Sin & Punishment) - GB2312 bytes
    else if (raw_name.find("\xD7\xEF\xD3\xEB\xB7\xA3") != std::string::npos)
      english_name = "Sin & Punishment";
    // 越野摩托 (Excitebike) - GB2312 bytes
    else if (raw_name.find("\xD4\xBD\xD2\xB0\xC4\xA6\xCD\xD0") !=
             std::string::npos)
      english_name = "Excitebike 64";
    // 马力欧卡丁车 (Mario Kart) - GB2312 bytes
    else if (raw_name.find(
                 "\xC2\xED\xC1\xA6\xC5\xB7\xBF\xA8\xB6\xA1\xB3\xB5") !=
             std::string::npos)
      english_name = "Mario Kart 64";
    // F-Zero X is already in ASCII
    else if (raw_name.find("F-Zero X") != std::string::npos)
      english_name = "F-Zero X";

    // Check Trial
    uint16_t trial_type =
        Swap16(*(uint16_t *)&buffer[TICKET_OFFSET_TRIAL_TYPE]);
    bool is_trial = (trial_type != 0);

    loaded_tickets.push_back({(int)i, english_name, raw_name, is_trial});
  }
  fclose(f);
}

void UnlockTicket(const char *filepath, int index) {
  FILE *f = fopen(filepath, "rb+");
  if (!f)
    return;

  // Calculate offset
  long offset = 4 + (index * TICKET_SIZE) + TICKET_OFFSET_TRIAL_TYPE;
  fseek(f, offset, SEEK_SET);

  // Zero out Trial Type (2 bytes)
  uint16_t zero16 = 0;
  fwrite(&zero16, 2, 1, f);

  // Zero out Trial Limit (4 bytes)
  uint32_t zero32 = 0;
  fwrite(&zero32, 4, 1, f);

  fclose(f);
}

// ================= GAME DATABASE =================
struct GameInfo {
  const char *title;
  const char *cid;
  const char *rom;
  const char *release;
};

const GameInfo GAMES[] = {
    {"Super Mario 64", "1101104", "Super Mario 64.z64", "2003-11-18"},
    {"Yoshi's Story", "1102101", "Yoshi's Story.z64", "2004-03-25"},
    {"Super Smash Bros.", "1201105", "Super Smash Bros.z64", "2005-02-24"},
    {"Zelda: Ocarina of Time", "2101104", "Zelda Ocarina of Time.z64",
     "2003-11-18"},
    {"Paper Mario", "2102104", "Paper Mario.z64", "2004-06-08"},
    {"Animal Crossing", "2104108", "Animal Crossing.z64", "2006-01-27"},
    {"Custom Robo", "2105103", "Custom Robo.z64", "2004-12-25"},
    {"Star Fox 64", "4101104", "Star Fox 64.z64", "2003-11-18"},
    {"Sin & Punishment", "4102103", "Sin and Punishment.z64", "2004-09-25"},
    {"Wave Race 64", "5101104", "Wave Race 64.z64", "2003-11-18"},
    {"Excitebike 64", "5102108", "Excitebike 64.z64", "2004-06-25"},
    {"Mario Kart 64", "5201104", "Mario Kart 64.z64", "2003-12-25"},
    {"F-Zero X", "5202103", "F-Zero X.z64", "2004-02-25"},
    {"Dr. Mario 64", "6101104", "Dr. Mario 64.z64", "2003-11-18"},
};
const int NUM_GAMES = 14;

// ================= GLOBALS =================
HWND hMainWindow = NULL;
HWND hComboGames = NULL;
HWND hListRoms = NULL;
HWND hLogWindow = NULL;

// Labels for Info Panel
HWND hLblTitle, hLblRelease, hLblCID, hLblROM, hLblSize;

// Brushes
HBRUSH hBrushBg, hBrushPanel, hBrushInput;

// Fonts
HFONT hFontHeader, hFontBold, hFontNormal, hFontMono;

// Storage Stats (updated on connect)
bool g_deviceConnected = false;
uint32_t g_freeBlocks = 0;
uint32_t g_usedBlocks = 0;
uint32_t g_badBlocks = 0;
const uint32_t BLOCK_SIZE_KB = 16;  // Each block is 16KB
const uint32_t TOTAL_BLOCKS = 4096; // 0x1000 blocks

// IDs
// Custom Macros
#define IDC_LOG_WINDOW 1001
#define IDC_COMBO_GAMES 2001
#define IDC_LIST_ROM 2002
#define IDC_LIST_ROMS 2002 // Alias for compatibility
#define IDC_BTN_OPEN_ROMS 2003
#define IDC_BTN_ADD_ROM 2004
#define IDC_BTN_CONNECT 3001
#define IDC_BTN_DISCONNECT 3002
#define IDC_BTN_DUMP_TICKET 3003
#define IDC_BTN_RESTORE_TICKET 3004
#define IDC_BTN_CONVERT 3004 // Alias
#define IDC_BTN_UNLOCK 3005
#define IDC_BTN_EXTRACT 3006
#define IDC_BTN_FLASH 3007
#define IDC_BTN_LOAD_TICKET 3008 // Replaced Export

// Global Ticket Path (Default)
char currentTicketPath[MAX_PATH] = "ticket.sys";
int currentTab = 0; // 0=Database, 1=Ticket.sys

void UpdateGameInfo(int index); // Forward decl

void PopulateGameList() {
  SendMessage(hComboGames, CB_RESETCONTENT, 0, 0);

  if (currentTab == 0) {
    // Database Mode
    for (int i = 0; i < NUM_GAMES; i++) {
      SendMessage(hComboGames, CB_ADDSTRING, 0, (LPARAM)GAMES[i].title);
    }
  } else {
    // Ticket Mode
    if (loaded_tickets.empty()) {
      SendMessage(hComboGames, CB_ADDSTRING, 0, (LPARAM) "(No tickets loaded)");
    } else {
      for (const auto &t : loaded_tickets) {
        char buf[256];
        // Show "[TRIAL] Name" or "[FULL] Name"
        sprintf(buf, "[%s] %s", t.is_trial ? "TRIAL" : "FULL", t.name.c_str());
        SendMessage(hComboGames, CB_ADDSTRING, 0, (LPARAM)buf);
      }
    }
  }
  SendMessage(hComboGames, CB_SETCURSEL, 0, 0);
  UpdateGameInfo(0);
}

void SelectTicketFile(HWND hwnd) {
  GuiLog("Debug: SelectTicketFile called.\n"); // Debug Log

  OPENFILENAME ofn;
  char szFile[MAX_PATH] = {0};

  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  ofn.lpstrFilter = "System Files (*.sys)\0*.sys\0All Files (*.*)\0*.*\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL;
  ofn.Flags =
      OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_HIDEREADONLY;

  GuiLog("Debug: Opening File Dialog...\n");

  if (GetOpenFileName(&ofn) == TRUE) {
    GuiLog("Debug: File selected.\n");
    strcpy(currentTicketPath, szFile);
    char buf[512];
    sprintf(buf, "Selected Ticket: %s\n", currentTicketPath);
    GuiLog(buf);

    // Parse the newly selected file
    ParseTicketSys(currentTicketPath);

    sprintf(buf, "Parsed %d tickets.\n", (int)loaded_tickets.size());
    GuiLog(buf);

    MessageBox(hwnd, "Ticket file loaded and parsed.", "Success", MB_OK);

    // Auto-switch to Ticket Tab
    currentTab = 1;
    PopulateGameList();
    InvalidateRect(hwnd, NULL, TRUE); // Redraw tabs
  } else {
    DWORD err = CommDlgExtendedError();
    if (err != 0) { // 0 means user cancelled
      char buf[128];
      sprintf(buf, "Debug: GetOpenFileName failed. Error: 0x%08X\n", err);
      GuiLog(buf);
    } else {
      GuiLog("Debug: Dialog cancelled by user.\n");
    }
  }
}

void UnlockAllTickets() {
  GuiLog("Scanning for trial tickets to unlock...\n");

  // Ensure we have latest data from current path
  ParseTicketSys(currentTicketPath);

  if (loaded_tickets.empty()) {
    GuiLog("No tickets loaded to unlock.\n");
    // Try reloading logic?
    return;
  }

  int unlockCount = 0;
  for (const auto &t : loaded_tickets) {
    if (t.is_trial) {
      char msg[256];
      sprintf(msg, "Unlocking: %s...\n", t.name.c_str());
      GuiLog(msg);
      UnlockTicket(currentTicketPath, t.index);
      unlockCount++;
    }
  }

  if (unlockCount == 0) {
    GuiLog("No trial tickets found. All games are full versions!\n");
    MessageBox(hMainWindow,
               "No trial tickets found.\nAll games appear to be full versions.",
               "Info", MB_OK);
  } else {
    char buf[128];
    sprintf(buf, "Successfully unlocked %d games!\n", unlockCount);
    GuiLog(buf);
    MessageBox(hMainWindow, buf, "Success", MB_OK);
    // Reload to reflect changes
    ParseTicketSys(currentTicketPath);
  }
}

// Include shared logging implementation (AFTER globals/IDs are defined)
#include "gui_common.cpp"
DWORD WINAPI ConnectThread(LPVOID lpParam) {
  GuiLog("Thread: Initializing connection...\n");
  if (Init()) {
    GuiLog("Thread: Init Success!\n");
    MessageBox(hMainWindow, "Connection Established!", "Success", MB_OK);
    EnableWindow(GetDlgItem(hMainWindow, IDC_BTN_DISCONNECT), TRUE);
    EnableWindow(GetDlgItem(hMainWindow, IDC_BTN_CONNECT), FALSE);

    // Fetch storage stats
    g_deviceConnected = true;
    if (get_storage_stats(&g_freeBlocks, &g_usedBlocks, &g_badBlocks)) {
      char buf[128];
      sprintf(buf, "Storage: %lu KB free / %lu KB used\n",
              (unsigned long)(g_freeBlocks * BLOCK_SIZE_KB),
              (unsigned long)(g_usedBlocks * BLOCK_SIZE_KB));
      GuiLog(buf);
    }
    InvalidateRect(hMainWindow, NULL, FALSE); // Redraw status bar
  } else {
    GuiLog("Thread: Init Failed.\n");
    MessageBox(hMainWindow, "Connection Failed.", "Error", MB_ICONERROR);
    EnableWindow(GetDlgItem(hMainWindow, IDC_BTN_CONNECT), TRUE);
    g_deviceConnected = false;
  }
  return 0;
}

// Draw Utils
void DrawPanel(HDC hdc, int x, int y, int w, int h, const char *title) {
  RECT r = {x, y, x + w, y + h};
  FillRect(hdc, &r, hBrushPanel);

  // Draw Title
  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, RGB(255, 255, 255));
  SelectObject(hdc, hFontBold);
  TextOut(hdc, x + 20, y + 15, title, strlen(title));
}

void DrawButton(LPDRAWITEMSTRUCT lp, const char *text, COLORREF color) {
  if (lp->CtlType != ODT_BUTTON)
    return;
  HDC hdc = lp->hDC;
  RECT rect = lp->rcItem;

  COLORREF drawColor = color;
  if (lp->itemState & ODS_SELECTED) {
    // Darken
    drawColor = RGB(GetRValue(color) * 0.8, GetGValue(color) * 0.8,
                    GetBValue(color) * 0.8);
  }
  if (lp->itemState & ODS_DISABLED) {
    drawColor = RGB(80, 80, 80);
  }

  HBRUSH hBrush = CreateSolidBrush(drawColor);
  FillRect(hdc, &rect, hBrush);
  DeleteObject(hBrush);

  // Rounded effect (fake with borders? Win32 Regions are hard, stick to
  // rectangle for now)

  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, RGB(255, 255, 255));
  SelectObject(hdc, hFontBold);
  DrawText(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

// GDI+ Globals
ULONG_PTR gdiplusToken;
Image *currentBoxArt = NULL;

// Helper to convert char* to wstring for GDI+
std::wstring ToWString(const char *str) {
  int len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
  std::vector<wchar_t> buf(len);
  MultiByteToWideChar(CP_ACP, 0, str, -1, &buf[0], len);
  return std::wstring(&buf[0]);
}

void LoadBoxArt(const char *cid, const char *title) {
  if (currentBoxArt) {
    delete currentBoxArt;
    currentBoxArt = NULL;
  }

  // Try CID first (e.g., "images/1001.png")
  char path[256];
  sprintf(path, "images\\%s.png", cid);
  currentBoxArt = Image::FromFile(ToWString(path).c_str());

  if (!currentBoxArt || currentBoxArt->GetLastStatus() != Ok) {
    if (currentBoxArt) {
      delete currentBoxArt;
      currentBoxArt = NULL;
    }

    // Try Title (e.g., "images/Super Mario 64.png")
    // Strip trailing period from title to avoid "Title..png" issue
    char cleanTitle[256];
    strncpy(cleanTitle, title, sizeof(cleanTitle) - 1);
    cleanTitle[sizeof(cleanTitle) - 1] = '\0';
    size_t len = strlen(cleanTitle);
    while (len > 0 && cleanTitle[len - 1] == '.') {
      cleanTitle[--len] = '\0';
    }
    sprintf(path, "images\\%s.png", cleanTitle);
    currentBoxArt = Image::FromFile(ToWString(path).c_str());
  }

  if (currentBoxArt && currentBoxArt->GetLastStatus() != Ok) {
    delete currentBoxArt;
    currentBoxArt = NULL;
  }

  // Force redraw of the box art area (covers entire image + fallback icon
  // region)
  RECT r = {45, 195, 45 + 100, 195 + 150};
  InvalidateRect(hMainWindow, &r, TRUE);
}

// Get ROM file size in bytes (returns 0 if file not found)
long GetRomFileSize(const char *romName) {
  char path[512];
  sprintf(path, "roms\\%s", romName);
  FILE *f = fopen(path, "rb");
  if (!f)
    return 0;
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fclose(f);
  return size;
}

// Format file size as human-readable string
void FormatFileSize(long bytes, char *outBuf, size_t bufLen) {
  if (bytes == 0) {
    sprintf(outBuf, "Not Found");
  } else if (bytes < 1024) {
    sprintf(outBuf, "%ld B", bytes);
  } else if (bytes < 1024 * 1024) {
    sprintf(outBuf, "%.1f KB", bytes / 1024.0);
  } else {
    sprintf(outBuf, "%.1f MB", bytes / (1024.0 * 1024.0));
  }
}

// Helper to update Info Labels based on selection
// Helper to update Info Labels based on selection
void UpdateGameInfo(int index) {
  if (index < 0)
    return;

  if (currentTab == 0) {
    if (index >= NUM_GAMES)
      return;
    SetWindowText(hLblTitle, GAMES[index].title);
    SetWindowText(hLblRelease, GAMES[index].release);
    SetWindowText(hLblCID, GAMES[index].cid);
    SetWindowText(hLblROM, GAMES[index].rom);

    // Get and display ROM size
    long romSize = GetRomFileSize(GAMES[index].rom);
    char sizeBuf[64];
    FormatFileSize(romSize, sizeBuf, sizeof(sizeBuf));
    SetWindowText(hLblSize, sizeBuf);

    LoadBoxArt(GAMES[index].cid, GAMES[index].title);
  } else {
    // Ticket Mode
    if (index >= (int)loaded_tickets.size()) {
      SetWindowText(hLblTitle, "-");
      SetWindowText(hLblRelease, "-");
      SetWindowText(hLblCID, "-");
      SetWindowText(hLblROM, "-");
      SetWindowText(hLblSize, "-");
      if (currentBoxArt) {
        delete currentBoxArt;
        currentBoxArt = NULL;
      }
      InvalidateRect(hMainWindow, NULL, FALSE);
      return;
    }

    const auto &t = loaded_tickets[index];
    SetWindowText(hLblTitle, t.name.c_str());
    SetWindowText(hLblRelease, t.is_trial ? "Trial Version" : "Full Version");

    // Try to find CID from database match
    int matchIdx = -1;
    for (int i = 0; i < NUM_GAMES; ++i) {
      if (t.name == GAMES[i].title) {
        matchIdx = i;
        break;
      }
    }

    if (matchIdx != -1) {
      char cidBuf[32];
      sprintf(cidBuf, "%s", GAMES[matchIdx].cid);
      SetWindowText(hLblCID, cidBuf);
      SetWindowText(hLblROM, GAMES[matchIdx].rom);

      // Get and display ROM size
      long romSize = GetRomFileSize(GAMES[matchIdx].rom);
      char sizeBuf[64];
      FormatFileSize(romSize, sizeBuf, sizeof(sizeBuf));
      SetWindowText(hLblSize, sizeBuf);

      LoadBoxArt(cidBuf, t.name.c_str());
    } else {
      SetWindowText(hLblCID, "Unknown");
      SetWindowText(hLblROM, "Unknown");
      SetWindowText(hLblSize, "Unknown");
      LoadBoxArt("0", t.name.c_str());
    }
  }
}

// Helper Thread for Dumping Ticket
DWORD WINAPI DumpTicketThread(LPVOID lpParam) {
  GuiLog("Thread: Dumping ticket.sys to application folder...\n");
  // AulonReadFile expects "XX filename" (skips first 2 chars)
  // We pass "  ticket.sys"
  if (AulonReadFile("  ticket.sys")) {
    GuiLog("Thread: ticket.sys Dumped Successfully!\n");
    MessageBox(hMainWindow, "ticket.sys saved to application folder.",
               "Success", MB_OK);

    // Reset path to default and parse
    strcpy(currentTicketPath, "ticket.sys");
    ParseTicketSys(currentTicketPath);

    char buf[128];
    sprintf(buf, "Parsed %d tickets from ticket.sys:\n",
            (int)loaded_tickets.size());
    GuiLog(buf);

    for (const auto &t : loaded_tickets) {
      sprintf(buf, " - %s [%s]\n", t.name.c_str(),
              t.is_trial ? "TRIAL" : "FULL");
      GuiLog(buf);
    }

  } else {
    GuiLog("Thread: Dump Failed. Is the file missing?\n");
    MessageBox(hMainWindow, "Failed to dump ticket.sys.", "Error",
               MB_ICONERROR);
  }
  return 0;
}

// Helper Thread for Restoring Ticket
DWORD WINAPI RestoreTicketThread(LPVOID lpParam) {
  char msg[MAX_PATH + 64];
  sprintf(msg, "Thread: Restoring %s to device...\n", currentTicketPath);
  GuiLog(msg);

  // Check if file exists first
  FILE *f = fopen(currentTicketPath, "rb");
  if (!f) {
    GuiLog("Error: Ticket file not found!\n");
    MessageBox(hMainWindow, "Selected ticket file not found!", "Error",
               MB_ICONERROR);
    return 0;
  }
  fclose(f);

  // If we are restoring a custom path, we should copy it to "ticket.sys" first
  // so AulonWriteFile picks it up, as it expects a local file named
  // "ticket.sys".
  if (strcmp(currentTicketPath, "ticket.sys") != 0) {
    GuiLog("Copying selected file to ticket.sys for upload...\n");
    if (!CopyFile(currentTicketPath, "ticket.sys", FALSE)) {
      GuiLog("Error copying file.\n");
      MessageBox(hMainWindow,
                 "Failed to copy selected ticket file to 'ticket.sys'.",
                 "Error", MB_ICONERROR);
      return 0;
    }
  }

  // AulonWriteFile expects "XX filename"
  if (AulonWriteFile("  ticket.sys")) {
    GuiLog("Thread: ticket.sys Restored Successfully!\n");
    MessageBox(hMainWindow, "ticket.sys flashed to device.", "Success", MB_OK);
  } else {
    GuiLog("Thread: Restore Failed. Check connection.\n");
    MessageBox(hMainWindow, "Failed to write ticket.sys.", "Error",
               MB_ICONERROR);
  }
  return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_CREATE: {
    // Create Brushes (BGR format for CreateSolidBrush)
    hBrushBg = CreateSolidBrush(RGB(13, 13, 26));
    hBrushPanel = CreateSolidBrush(RGB(26, 26, 48));
    hBrushInput = CreateSolidBrush(RGB(37, 37, 80));

    // Fonts
    hFontHeader =
        CreateFont(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
                   OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                   DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    hFontBold =
        CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET,
                   OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                   DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    hFontNormal =
        CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
                   OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                   DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    hFontMono =
        CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
                   OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                   DEFAULT_PITCH | FF_MODERN, "Consolas");

    // === CONTROLS ===

    // 1. GAME SELECTION (Top Left)
    // Shifted down to avoid overlapping the "Game Selection" header
    hComboGames = CreateWindow("COMBOBOX", "",
                               WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST |
                                   CBS_OWNERDRAWFIXED | CBS_HASSTRINGS,
                               45, 145, 410, 200, hwnd, (HMENU)IDC_COMBO_GAMES,
                               NULL, NULL);

    // Remove default borders to match "Flat" look
    LONG_PTR style = GetWindowLongPtr(hComboGames, GWL_EXSTYLE);
    SetWindowLongPtr(hComboGames, GWL_EXSTYLE, style & ~WS_EX_CLIENTEDGE);
    SetWindowPos(hComboGames, NULL, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    SendMessage(hComboGames, WM_SETFONT, (WPARAM)hFontNormal, TRUE);
    PopulateGameList();
    // for (int i = 0; i < NUM_GAMES; i++) {
    //   SendMessage(hComboGames, CB_ADDSTRING, 0, (LPARAM)GAMES[i].title);
    // }
    // SendMessage(hComboGames, CB_SETCURSEL, 0, 0);

    // Info Labels (Shifted Right to clear Box Art, and Down)
    // Box Art will be at x=45, w=100. So text starts at 160+.
    int infoX = 170, infoY = 200;

    CreateWindow("STATIC", "Title:", WS_CHILD | WS_VISIBLE, infoX, infoY, 80,
                 20, hwnd, NULL, NULL, NULL);
    hLblTitle =
        CreateWindow("STATIC", "-", WS_CHILD | WS_VISIBLE | SS_RIGHT,
                     infoX + 80, infoY, 200, 20, hwnd, NULL, NULL, NULL);

    CreateWindow("STATIC", "Release:", WS_CHILD | WS_VISIBLE, infoX, infoY + 30,
                 80, 20, hwnd, NULL, NULL, NULL);
    hLblRelease =
        CreateWindow("STATIC", "-", WS_CHILD | WS_VISIBLE | SS_RIGHT,
                     infoX + 80, infoY + 30, 200, 20, hwnd, NULL, NULL, NULL);

    CreateWindow("STATIC", "CID:", WS_CHILD | WS_VISIBLE, infoX, infoY + 60, 80,
                 20, hwnd, NULL, NULL, NULL);
    hLblCID =
        CreateWindow("STATIC", "-", WS_CHILD | WS_VISIBLE | SS_RIGHT,
                     infoX + 80, infoY + 60, 200, 20, hwnd, NULL, NULL, NULL);

    CreateWindow("STATIC", "ROM:", WS_CHILD | WS_VISIBLE, infoX, infoY + 90, 80,
                 20, hwnd, NULL, NULL, NULL);
    hLblROM =
        CreateWindow("STATIC", "-", WS_CHILD | WS_VISIBLE | SS_RIGHT,
                     infoX + 80, infoY + 90, 200, 20, hwnd, NULL, NULL, NULL);

    CreateWindow("STATIC", "Size:", WS_CHILD | WS_VISIBLE, infoX, infoY + 120,
                 80, 20, hwnd, NULL, NULL, NULL);
    hLblSize =
        CreateWindow("STATIC", "-", WS_CHILD | WS_VISIBLE | SS_RIGHT,
                     infoX + 80, infoY + 120, 200, 20, hwnd, NULL, NULL, NULL);

    UpdateGameInfo(0);

    // 2. ROM MANAGEMENT (Bottom Left)
    // Shifted down to clear the "ROM Management" header
    int romBtnY = 440;
    CreateWindow("BUTTON", "Open RoMs", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                 45, romBtnY, 150, 35, hwnd, (HMENU)IDC_BTN_OPEN_ROMS, NULL,
                 NULL);

    CreateWindow("BUTTON", "+ Add Custom", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                 210, romBtnY, 150, 35, hwnd, (HMENU)IDC_BTN_ADD_ROM, NULL,
                 NULL);

    hListRoms = CreateWindowEx(
        0, "LISTBOX", "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | WS_BORDER, 45, 490,
        410, 130, hwnd, (HMENU)IDC_LIST_ROMS, NULL, NULL);

    // Remove 3D borders for flat look
    SetWindowLongPtr(hListRoms, GWL_EXSTYLE,
                     GetWindowLongPtr(hListRoms, GWL_EXSTYLE) &
                         ~WS_EX_CLIENTEDGE);
    SetWindowPos(hListRoms, NULL, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    SendMessage(hListRoms, LB_ADDSTRING, 0,
                (LPARAM) "Super Mario 64.z64"); // Dummy data

    // 3. DEVICE CONNECTION (Top Right)
    CreateWindow("BUTTON", "Connect", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 500,
                 60, 250, 45, hwnd, (HMENU)IDC_BTN_CONNECT, NULL, NULL);

    CreateWindow("BUTTON", "Disconnect",
                 WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_DISABLED, 760, 60,
                 250, 45, hwnd, (HMENU)IDC_BTN_DISCONNECT, NULL, NULL);

    // 4. ACTIONS (Top Right / Middle Right)
    int actY = 160;
    CreateWindow("BUTTON", "Dump ticket.sys",
                 WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 500, actY, 510, 45, hwnd,
                 (HMENU)IDC_BTN_DUMP_TICKET, NULL, NULL);
    actY += 55;

    CreateWindow("BUTTON", "Unlock All Trials",
                 WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 500, actY, 510, 45, hwnd,
                 (HMENU)IDC_BTN_UNLOCK, NULL, NULL);
    actY += 55;

    CreateWindow("BUTTON", "Restore ticket.sys",
                 WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 500, actY, 510, 45, hwnd,
                 (HMENU)IDC_BTN_RESTORE_TICKET, NULL, NULL);
    actY += 55;

    CreateWindow("BUTTON", "Extract Keys", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                 500, actY, 510, 45, hwnd, (HMENU)IDC_BTN_EXTRACT, NULL, NULL);
    actY += 55;

    CreateWindow("BUTTON", "Flash Selected ROM",
                 WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 500, actY, 510, 45, hwnd,
                 (HMENU)IDC_BTN_FLASH, NULL, NULL);
    actY += 55;

    CreateWindow("BUTTON", "Load ticket.sys",
                 WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 500, actY, 510, 45, hwnd,
                 (HMENU)IDC_BTN_LOAD_TICKET, NULL, NULL);

    // 5. CONSOLE (Bottom Right - ish)
    hLogWindow = CreateWindowEx(
        0, "EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY |
            ES_AUTOVSCROLL | WS_BORDER,
        500, 490, 510, 120, hwnd, (HMENU)IDC_LOG_WINDOW, NULL, NULL);

    // Remove 3D borders
    SetWindowLongPtr(hLogWindow, GWL_EXSTYLE,
                     GetWindowLongPtr(hLogWindow, GWL_EXSTYLE) &
                         ~WS_EX_CLIENTEDGE);
    SetWindowPos(hLogWindow, NULL, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    SendMessage(hLogWindow, WM_SETFONT, (WPARAM)hFontMono, TRUE);

    GuiLog("iQue Tools v2.0 (Port) Loaded.\r\n");
    break;
  }

  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    FillRect(hdc, &ps.rcPaint, hBrushBg);

    // Header
    RECT rHeader = {0, 0, 1100, 60};
    FillRect(hdc, &rHeader, hBrushPanel);

    // Draw Colorful Logo: iQue
    SetBkMode(hdc, TRANSPARENT);
    SelectObject(hdc, hFontHeader);

    int x = 15;
    int y = 12;

    // Draw "iQue" as one compact word with VERY tight spacing (like the
    // reference)
    SetTextColor(hdc, RGB(231, 76, 60)); // Red i
    TextOut(hdc, x, y, "i", 1);
    x += 6; // Almost no gap between i and Q

    SetTextColor(hdc, RGB(46, 204, 113)); // Green Q
    TextOut(hdc, x, y, "Q", 1);
    x += 20; // Q is wider but still tight

    SetTextColor(hdc, RGB(52, 152, 219)); // Blue u
    TextOut(hdc, x, y, "u", 1);
    x += 16; // Tight

    SetTextColor(hdc, RGB(241, 196, 15)); // Yellow e
    TextOut(hdc, x, y, "e", 1);

    // Title "iQue Tools" - positioned to the right of the colored logo
    SetTextColor(hdc, RGB(255, 255, 255));
    TextOut(hdc, 85, y, "iQue Tools", 10);

    SetTextColor(hdc, RGB(180, 180, 180));
    SelectObject(hdc, hFontNormal);
    TextOut(hdc, 85, y + 30, "64-bit Hacking Toolkit", 22);

    // ===================================
    // GAME SELECTION PANEL
    // ===================================
    RECT rPanel1 = {25, 80, 475, 370}; // Increased height for Size label
    FillRect(hdc, &rPanel1, hBrushPanel);

    // Draw Tab Buttons
    // Tab 1: Database (45, 115, 120, 25)
    // Tab 2: Ticket.sys (175, 115, 120, 25)
    RECT rTab1 = {45, 115, 165, 140};
    RECT rTab2 = {170, 115, 290, 140};

    // Fill Tabs
    HBRUSH hBrushTabActive = CreateSolidBrush(COL_PANEL); // Matches panel
    HBRUSH hBrushTabInactive = CreateSolidBrush(RGB(50, 50, 80)); // Darker

    FillRect(hdc, &rTab1,
             (currentTab == 0) ? hBrushTabActive : hBrushTabInactive);
    FillRect(hdc, &rTab2,
             (currentTab == 1) ? hBrushTabActive : hBrushTabInactive);

    DeleteObject(hBrushTabActive);
    DeleteObject(hBrushTabInactive);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));
    SelectObject(hdc, hFontNormal);

    DrawText(hdc, "Database", -1, &rTab1,
             DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DrawText(hdc, "Ticket.sys", -1, &rTab2,
             DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Box Art / Icon Area
    if (currentBoxArt && currentBoxArt->GetLastStatus() == Ok) {
      Graphics graphics(hdc);

      // Get original image dimensions
      int imgW = currentBoxArt->GetWidth();
      int imgH = currentBoxArt->GetHeight();

      // Target area: max 100 wide, max 150 tall
      int maxW = 100;
      int maxH = 150;

      // Calculate scale to fit while preserving aspect ratio
      float scaleW = (float)maxW / imgW;
      float scaleH = (float)maxH / imgH;
      float scale = (scaleW < scaleH) ? scaleW : scaleH;

      int drawW = (int)(imgW * scale);
      int drawH = (int)(imgH * scale);

      // Center in the available area
      int drawX = 45 + (maxW - drawW) / 2;
      int drawY = 195 + (maxH - drawH) / 2;

      graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
      graphics.DrawImage(currentBoxArt, drawX, drawY, drawW, drawH);
    } else {
      // Fallback: Controller Icon
      HPEN hPenIcon = CreatePen(PS_SOLID, 2, RGB(200, 200, 200));
      SelectObject(hdc, hPenIcon);
      SelectObject(hdc, GetStockObject(NULL_BRUSH));

      int iconX = 65; // Center in the 100px area (45 -> 145)
      int iconY = 230;
      RoundRect(hdc, iconX, iconY, iconX + 60, iconY + 40, 10, 10);
      Ellipse(hdc, iconX + 40, iconY + 15, iconX + 45, iconY + 20); // Buttons A
      Ellipse(hdc, iconX + 48, iconY + 10, iconX + 53, iconY + 15); // Buttons B

      MoveToEx(hdc, iconX + 15, iconY + 20, NULL); // D-Pad horizontal
      LineTo(hdc, iconX + 25, iconY + 20);
      MoveToEx(hdc, iconX + 20, iconY + 15, NULL); // D-Pad vertical
      LineTo(hdc, iconX + 20, iconY + 25);

      DeleteObject(hPenIcon);
    }

    SetTextColor(hdc, RGB(255, 255, 255));
    SelectObject(hdc, hFontBold);
    TextOut(hdc, 70, 95, "Game Selection", 14);

    // Icon for Header
    // Simple Controller Icon
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    SelectObject(hdc, hPen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    RoundRect(hdc, 40, 95, 60, 110, 5, 5); // Mini Controller
    DeleteObject(hPen);

    // Tabs now replace "Select Title" label - commented out to avoid overlap
    // SelectObject(hdc, hFontNormal);
    // TextOut(hdc, 40, 120, "Select Title", 12);

    // Panels
    DrawPanel(hdc, 25, 380, 450, 260, "ROM Management");

    DrawPanel(hdc, 485, 30, 540, 90, "Device Connection");
    DrawPanel(hdc, 485, 130, 540, 500, "Actions");

    // ===================================
    // STATUS BAR (Bottom)
    // ===================================
    RECT rStatusBar = {0, 640, 1100, 670};
    FillRect(hdc, &rStatusBar, hBrushPanel);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(180, 180, 180));
    SelectObject(hdc, hFontNormal);

    if (g_deviceConnected) {
      // Calculate storage in MB
      uint32_t freeMB = (g_freeBlocks * BLOCK_SIZE_KB) / 1024;
      uint32_t usedMB = (g_usedBlocks * BLOCK_SIZE_KB) / 1024;
      uint32_t totalMB = (TOTAL_BLOCKS * BLOCK_SIZE_KB) / 1024;

      // Calculate percentage for progress bar
      int usedPercent = (g_usedBlocks * 100) / TOTAL_BLOCKS;

      // Draw text
      char statusText[128];
      sprintf(statusText,
              "Device Connected | Storage: %lu MB used / %lu MB total (%d%%)",
              (unsigned long)usedMB, (unsigned long)totalMB, usedPercent);
      TextOut(hdc, 20, 648, statusText, strlen(statusText));

      // Draw progress bar
      int barX = 700, barY = 650, barW = 350, barH = 12;
      RECT rBarBg = {barX, barY, barX + barW, barY + barH};
      HBRUSH hBrushBarBg = CreateSolidBrush(RGB(50, 50, 80));
      FillRect(hdc, &rBarBg, hBrushBarBg);
      DeleteObject(hBrushBarBg);

      // Filled portion
      int filledW = (barW * usedPercent) / 100;
      if (filledW > 0) {
        RECT rBarFill = {barX, barY, barX + filledW, barY + barH};
        COLORREF fillColor = (usedPercent > 90)   ? RGB(220, 50, 50)
                             : (usedPercent > 70) ? RGB(220, 180, 50)
                                                  : RGB(50, 180, 100);
        HBRUSH hBrushFill = CreateSolidBrush(fillColor);
        FillRect(hdc, &rBarFill, hBrushFill);
        DeleteObject(hBrushFill);
      }
    } else {
      TextOut(hdc, 20, 648, "Device Not Connected", 20);
    }

    EndPaint(hwnd, &ps);
    return 0;
  }

  case WM_CTLCOLORSTATIC: {
    HDC hdc = (HDC)wParam;
    SetBkColor(hdc, RGB(26, 26, 48)); // Matches Panel
    SetTextColor(hdc, RGB(255, 255, 255));
    return (LRESULT)hBrushPanel;
  }
  case WM_CTLCOLOREDIT: {
    HDC hdc = (HDC)wParam; // Log Window
    SetBkColor(hdc, RGB(37, 37, 80));
    SetTextColor(hdc, RGB(255, 255, 255));
    return (LRESULT)hBrushInput;
  }
  case WM_CTLCOLORLISTBOX: {
    HDC hdc = (HDC)wParam;
    SetBkColor(hdc, RGB(37, 37, 80)); // Dark List BG
    SetTextColor(hdc, RGB(255, 255, 255));
    return (LRESULT)hBrushInput;
  }

  case WM_DRAWITEM: {
    LPDRAWITEMSTRUCT lp = (LPDRAWITEMSTRUCT)lParam;

    // Handle ComboBox (Game Selection)
    if (lp->CtlID == IDC_COMBO_GAMES) {
      HDC hdc = lp->hDC;
      RECT rc = lp->rcItem;

      if (lp->itemID == -1)
        return TRUE;

      // Background Color
      COLORREF bgColor = RGB(37, 37, 80); // Input Color
      if (lp->itemState & ODS_SELECTED) {
        bgColor = RGB(60, 60, 100); // Lighter Selection
      }

      HBRUSH hBrush = CreateSolidBrush(bgColor);
      FillRect(hdc, &rc, hBrush);
      DeleteObject(hBrush);

      // Text
      char text[256];
      SendMessage(hComboGames, CB_GETLBTEXT, lp->itemID, (LPARAM)text);

      SetBkMode(hdc, TRANSPARENT);
      SetTextColor(hdc, RGB(255, 255, 255));
      SelectObject(hdc, hFontNormal);
      rc.left += 5; // Padding
      DrawText(hdc, text, -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

      return TRUE;
    }

    // Handle Buttons
    COLORREF c = RGB(100, 100, 100);
    const char *txt = "?";

    switch (lp->CtlID) {
    case IDC_BTN_OPEN_ROMS:
      txt = "Open RoMs";
      c = RGB(50, 50, 80);
      break;
    case IDC_BTN_ADD_ROM:
      txt = "+ Add Custom";
      c = COL_ACCENT;
      break;
    case IDC_BTN_CONNECT:
      txt = "Connect";
      c = COL_GREEN;
      break;
    case IDC_BTN_DISCONNECT:
      txt = "Disconnect";
      c = COL_RED;
      break;
    case IDC_BTN_DUMP_TICKET:
      txt = "Dump ticket.sys";
      c = COL_BLUE;
      break;
    case IDC_BTN_UNLOCK:
      txt = "Unlock All Trials";
      c = COL_ACCENT; // Special visual
      break;
    case IDC_BTN_RESTORE_TICKET: // Same as CONVERT
      txt = "Restore ticket.sys";
      c = COL_ORANGE;
      break;
    case IDC_BTN_EXTRACT:
      txt = "Extract Keys";
      c = COL_ACCENT;
      break;
    case IDC_BTN_FLASH:
      txt = "Flash Selected ROM";
      c = COL_GREEN;
      break;
    case IDC_BTN_LOAD_TICKET:
      txt = "Load ticket.sys";
      c = COL_ORANGE; // Distinct color, maybe make it Teal? Let's use
                      // COL_LAVENDER reused or custom?
      // COL_LAVENDER was 0x... let's stick to Orange/Blue scheme or make it
      // Green? Let's keep it Lavender for now as it replaces Export
      c = COL_LAVENDER;
      break;
    }
    DrawButton(lp, txt, c);
    return TRUE;
  }

  case WM_LBUTTONDOWN: {
    // Enable dragging for title bar area (roughly)
    int x = LOWORD(lParam);
    int y = HIWORD(lParam);

    // Check Tabs
    if (y >= 115 && y <= 140) {
      if (x >= 45 && x <= 165) {
        if (currentTab != 0) {
          currentTab = 0;
          PopulateGameList();
          InvalidateRect(hwnd, NULL, FALSE);
        }
      } else if (x >= 170 && x <= 290) {
        if (currentTab != 1) {
          currentTab = 1;
          PopulateGameList();
          InvalidateRect(hwnd, NULL, FALSE);
        }
      }
    }

    // Title bar drag
    if (y < 60) {
      SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
    }
    return 0;
  }

  case WM_COMMAND: {
    if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == IDC_COMBO_GAMES) {
      int idx = SendMessage(hComboGames, CB_GETCURSEL, 0, 0);
      UpdateGameInfo(idx);
      return 0;
    }

    switch (LOWORD(wParam)) {
    case IDC_BTN_CONNECT:
      EnableWindow(GetDlgItem(hwnd, IDC_BTN_CONNECT), FALSE);
      CreateThread(NULL, 0, ConnectThread, NULL, 0, NULL);
      break;

    case IDC_BTN_DUMP_TICKET:
      GuiLog("Action: Dump ticket.sys requested.\n");
      // Run in thread to prevent freezing
      CreateThread(NULL, 0, DumpTicketThread, NULL, 0, NULL);
      break;

    case IDC_BTN_UNLOCK:
      GuiLog("Action: Unlock All Trials requested.\n");
      UnlockAllTickets();
      break;

    case IDC_BTN_RESTORE_TICKET: // Repurposed
      GuiLog("Action: Restore ticket.sys requested.\n");
      CreateThread(NULL, 0, RestoreTicketThread, NULL, 0, NULL);
      break;

    case IDC_BTN_LOAD_TICKET:
      // GuiLog("Action: Load Selection requested.\n"); // Already logging in
      // Select function
      SelectTicketFile(hwnd);
      break;

    case IDC_BTN_FLASH: {
      GuiLog("Action: Flash Selected ROM requested.\n");

      // Get selected ROM from list
      int romIdx = SendMessage(hListRoms, LB_GETCURSEL, 0, 0);
      if (romIdx == LB_ERR) {
        MessageBox(hwnd, "Please select a ROM to flash.", "No Selection",
                   MB_OK | MB_ICONWARNING);
        break;
      }

      char romName[256];
      SendMessage(hListRoms, LB_GETTEXT, romIdx, (LPARAM)romName);

      // Get ROM file size
      long romSize = GetRomFileSize(romName);
      if (romSize == 0) {
        MessageBox(hwnd, "ROM file not found in 'roms' folder.",
                   "File Not Found", MB_OK | MB_ICONERROR);
        break;
      }

      // Check if device is connected
      if (!g_deviceConnected) {
        MessageBox(hwnd, "Device not connected. Please connect first.",
                   "Not Connected", MB_OK | MB_ICONWARNING);
        break;
      }

      // Calculate required blocks (16KB per block)
      long romSizeKB = romSize / 1024;
      long freeSpaceKB = g_freeBlocks * BLOCK_SIZE_KB;

      if (romSizeKB > freeSpaceKB) {
        char msg[512];
        sprintf(msg,
                "Not enough storage space!\n\nROM Size: %.1f MB\nFree Space: "
                "%.1f MB\n\nPlease free up space by deleting games.",
                romSize / (1024.0 * 1024.0), freeSpaceKB / 1024.0);
        MessageBox(hwnd, msg, "Insufficient Storage", MB_OK | MB_ICONERROR);
        break;
      }

      // Proceed with flash (placeholder - actual implementation needed)
      char confirmMsg[256];
      sprintf(confirmMsg, "Flash %s (%.1f MB)?\n\nFree space: %.1f MB", romName,
              romSize / (1024.0 * 1024.0), freeSpaceKB / 1024.0);
      if (MessageBox(hwnd, confirmMsg, "Confirm Flash",
                     MB_YESNO | MB_ICONQUESTION) == IDYES) {
        GuiLog("Flashing ROM... (Not yet implemented)\n");
        MessageBox(hwnd, "Flash functionality not yet implemented.",
                   "Coming Soon", MB_OK | MB_ICONINFORMATION);
      }
      break;
    }
    }
    break;
  }

  case WM_DESTROY:
    PostQuitMessage(0);
    DeleteObject(hBrushBg);
    DeleteObject(hBrushPanel);
    break;

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
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
  // Initialize GDI+
  GdiplusStartupInput gdiplusStartupInput;
  GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

  HBRUSH hBrush = CreateSolidBrush(RGB(13, 13, 26));
  WNDCLASSEX wc = {sizeof(WNDCLASSEX),
                   CS_HREDRAW | CS_VREDRAW,
                   WndProc,
                   0,
                   0,
                   hInstance,
                   LoadIcon(NULL, IDI_APPLICATION),
                   LoadCursor(NULL, IDC_ARROW),
                   hBrush,
                   NULL,
                   "iQueModernUI",
                   NULL};
  RegisterClassEx(&wc);

  hMainWindow = CreateWindow("iQueModernUI", "iQue Tools",
                             WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 1060,
                             720, NULL, NULL, hInstance, NULL);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // Shutdown GDI+
  GdiplusShutdown(gdiplusToken);

  return msg.wParam;
}
