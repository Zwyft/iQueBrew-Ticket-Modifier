# iQue Homebrew Menu

A custom system menu replacement for the iQue Player, written in C using libdragon.

## Features (Planned)

- **Game Selector** - Browse and launch installed games with English titles
- **Settings Menu** - Storage info, trial unlock, system options
- **Modern UI** - Clean, minimal design with N64 aesthetic hints

## Building

### Prerequisites (Ubuntu/Linux)

```bash
# Install dependencies
sudo apt install build-essential git libpng-dev

# Clone and install libdragon
git clone https://github.com/DragonMinded/libdragon.git
cd libdragon
./tools/build_toolchain.sh
make
make install
make tools
make tools-install

# Add to PATH
export N64_INST=/path/to/n64toolchain
export PATH=$N64_INST/bin:$PATH
```

### Build the Menu

```bash
cd homebrew/menu
make
```

This produces `ique_menu.z64` which can be loaded on the iQue Player via the hackit exploit.

## Project Structure

```
homebrew/menu/
├── Makefile          # Build configuration
├── src/
│   ├── main.c        # Entry point
│   ├── ui.c          # UI rendering
│   ├── games.c       # Game list and launching
│   ├── settings.c    # Settings menu
│   └── ticket.c      # ticket.sys parsing
└── include/
    └── *.h           # Headers
```

## Testing

```bash
# Run in ares emulator
ares ique_menu.z64
```

## License

MIT
