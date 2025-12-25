# iQue Homebrew Menu - Gemini CLI Project Prompt

Copy and paste this entire prompt into Gemini CLI on your Ubuntu machine:

---

## PROJECT CONTEXT

I am working on a **custom homebrew system menu for the iQue Player**, a China-exclusive Nintendo 64-based console. The code has been pushed to GitHub and I need you to help me build it.

### Repository Info
- **Repo:** https://github.com/Zwyft/iQueBrew-Ticket-Modifier
- **Branch:** `homebrew-menu`
- **Project Path:** `homebrew/menu/`

### Project Goal
Create a custom N64 ROM that replaces the stock iQue system menu with an English homebrew alternative. The menu should:
1. Display a list of installed games (parsed from `ticket.sys`)
2. Allow navigation with D-pad
3. Launch games with A button
4. Provide a Settings menu with storage info and trial unlock features

### UI Design Requirements
- **Style:** Modern, minimal, with subtle N64 aesthetic hints
- **Colors:** Dark background (~RGB 20,20,30), light text (~RGB 220,220,230), blue highlights (~RGB 100,150,255)
- **Phase 1:** Text-only MVP (no graphics yet)
- **Phase 2:** Add box art and polish after text is working

---

## CURRENT STATE

The `homebrew/menu/` folder contains:
- `Makefile` - Build configuration for libdragon
- `src/main.c` - Basic menu structure with navigation
- `README.md` - Build instructions

A setup script `homebrew/setup_ubuntu.sh` is provided to install libdragon.

---

## YOUR TASKS

### Step 1: Clone and Setup
```bash
# If not already cloned
git clone https://github.com/Zwyft/iQueBrew-Ticket-Modifier.git
cd iQueBrew-Ticket-Modifier
git checkout homebrew-menu

# Run setup (installs libdragon toolchain - allow 30-60 min first time)
chmod +x homebrew/setup_ubuntu.sh
./homebrew/setup_ubuntu.sh

# Add to PATH (if not already done)
export N64_INST=$HOME/n64_toolchain
export PATH=$N64_INST/bin:$PATH
```

### Step 2: Build and Test
```bash
cd homebrew/menu
make
```
This should produce `ique_menu.z64`. Test it in an N64 emulator like `ares` or `simple64`.

### Step 3: Fix Any Build Errors
The current `main.c` may need adjustments for the latest libdragon API. Common fixes:
- `display_init()` signature changes between libdragon versions
- `graphics_draw_text()` may need different parameters
- Controller API might differ

### Step 4: Implement Core Features
Once building and running in emulator, implement:

1. **Game List Parsing** - Create `src/games.c`:
   - Read `ticket.sys` file format (iQue-specific)
   - Parse Content IDs and game titles
   - Store in memory for display

2. **Game List UI** - Update `main.c`:
   - Display scrollable list of games
   - Highlight selected game
   - Show game details (trial status, etc.)

3. **Settings Menu** - Create `src/settings.c`:
   - Show storage statistics (free/used blocks)
   - Trial unlock option (modify ticket in memory)
   - About screen

### Step 5: Test on Hardware (Later)
The built ROM will be loaded onto the iQue Player using the `hackit` exploit we already have working. For now, focus on emulator testing.

---

## TECHNICAL NOTES

### Libdragon API Reference
- https://dragonminded.github.io/libdragon/
- Use the `preview` branch for latest features

### iQue-Specific Considerations
- The iQue has 64MB of NAND flash storage
- Games are stored as encrypted `.rec` files
- `ticket.sys` contains the game list with Content IDs and license info
- The hackit exploit gives us unsigned code execution

### ticket.sys Structure (from earlier analysis)
Each ticket entry contains:
- Content ID (8 bytes, e.g., `0000000100010100` for Dr. Mario)
- Title (Chinese GB2312 encoding)
- License type (trial limit, full game)
- ECC public key and encrypted title key

---

## EXPECTED OUTPUT

A working `ique_menu.z64` that:
1. Displays "iQue Homebrew Menu" title
2. Shows "Play Game", "Settings", "About" options
3. Navigates with D-pad (up/down to move, A to select, B to go back)
4. Has placeholder screens for Games and Settings sub-menus

---

## COMMANDS TO START

```bash
cd ~/iQueBrew-Ticket-Modifier  # or wherever you cloned it
git checkout homebrew-menu
cd homebrew/menu
make
```

If libdragon is not installed, run `./homebrew/setup_ubuntu.sh` first.

Let me know when you have the environment set up and we can iterate on the code!
