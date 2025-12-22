#ifndef GUI_REDIRECT_H
#define GUI_REDIRECT_H

#include <stdarg.h>
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration of the GUI logging function
void GuiLog(const char *format, ...);
void GuiLogV(const char *format, va_list args);

// Macro redefinitions
#define printf(...) GuiLog(__VA_ARGS__)
#define fprintf(stream, ...) GuiLog(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // GUI_REDIRECT_H
