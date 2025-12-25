/**
 * iQue Homebrew Menu
 *
 * Custom system menu replacement for iQue Player
 * Modern, minimal UI with N64 aesthetic hints
 *
 * Phase 1: Text-only MVP with game selection
 */

#include <libdragon.h>
#include <stdio.h>
#include <string.h>

// Menu state
typedef enum { MENU_MAIN, MENU_GAMES, MENU_SETTINGS } MenuState;

static MenuState current_menu = MENU_MAIN;
static int selected_item = 0;

// Main menu items
static const char *main_menu_items[] = {"Play Game", "Settings", "About"};
#define MAIN_MENU_COUNT 3

// Colors (modern minimal palette)
#define COLOR_BG graphics_make_color(20, 20, 30, 255)
#define COLOR_TEXT graphics_make_color(220, 220, 230, 255)
#define COLOR_HIGHLIGHT graphics_make_color(100, 150, 255, 255)
#define COLOR_DIM graphics_make_color(100, 100, 120, 255)

/**
 * Draw the main menu
 */
void draw_main_menu(display_context_t disp) {
  // Clear screen with dark background
  graphics_fill_screen(disp, COLOR_BG);

  // Title
  graphics_set_color(COLOR_HIGHLIGHT, 0);
  graphics_draw_text(disp, 100, 40, "iQue Homebrew Menu");

  // Draw menu items
  for (int i = 0; i < MAIN_MENU_COUNT; i++) {
    int y = 100 + (i * 30);

    if (i == selected_item) {
      // Highlight selected item
      graphics_set_color(COLOR_HIGHLIGHT, 0);
      graphics_draw_text(disp, 90, y, ">");
      graphics_set_color(COLOR_TEXT, 0);
    } else {
      graphics_set_color(COLOR_DIM, 0);
    }

    graphics_draw_text(disp, 110, y, main_menu_items[i]);
  }

  // Footer
  graphics_set_color(COLOR_DIM, 0);
  graphics_draw_text(disp, 80, 200, "D-Pad: Navigate  A: Select");
}

/**
 * Handle controller input
 */
void handle_input(void) {
  controller_scan();
  struct controller_data keys = get_keys_down();

  // D-Pad navigation
  if (keys.c[0].up) {
    selected_item--;
    if (selected_item < 0)
      selected_item = MAIN_MENU_COUNT - 1;
  }
  if (keys.c[0].down) {
    selected_item++;
    if (selected_item >= MAIN_MENU_COUNT)
      selected_item = 0;
  }

  // A button - select
  if (keys.c[0].A) {
    switch (selected_item) {
    case 0: // Play Game
      current_menu = MENU_GAMES;
      break;
    case 1: // Settings
      current_menu = MENU_SETTINGS;
      break;
    case 2: // About
      // TODO: Show about screen
      break;
    }
  }

  // B button - back
  if (keys.c[0].B) {
    current_menu = MENU_MAIN;
    selected_item = 0;
  }
}

/**
 * Main entry point
 */
int main(void) {
  // Initialize libdragon subsystems
  display_init(RESOLUTION_320x240, DEPTH_32_BPP, 2, GAMMA_NONE,
               ANTIALIAS_RESAMPLE);
  controller_init();

  // Main loop
  while (1) {
    // Get display buffer
    display_context_t disp = display_get();

    // Handle input
    handle_input();

    // Draw current menu
    switch (current_menu) {
    case MENU_MAIN:
      draw_main_menu(disp);
      break;
    case MENU_GAMES:
      // TODO: Draw game list
      graphics_fill_screen(disp, COLOR_BG);
      graphics_set_color(COLOR_TEXT, 0);
      graphics_draw_text(disp, 100, 100, "Game List (TODO)");
      graphics_draw_text(disp, 100, 130, "Press B to go back");
      break;
    case MENU_SETTINGS:
      // TODO: Draw settings
      graphics_fill_screen(disp, COLOR_BG);
      graphics_set_color(COLOR_TEXT, 0);
      graphics_draw_text(disp, 100, 100, "Settings (TODO)");
      graphics_draw_text(disp, 100, 130, "Press B to go back");
      break;
    }

    // Show frame
    display_show(disp);
  }

  return 0;
}
