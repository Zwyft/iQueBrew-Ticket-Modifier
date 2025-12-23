# iQue Tools

A comprehensive toolkit for iQue Player enthusiasts. Includes tools for USB communication, encryption/decryption, game management, and ROM flashing.

## 📁 Project Structure

```
iQueGithub/
├── aulon/              # USB communication tool for iQue Player
├── iquecrypt/          # Encryption/decryption tool
├── ique_tools.py       # Main GUI application (Python)
├── dist/               # Compiled executables
│   └── iQue_Tools.exe  # Standalone app (no Python needed)
├── installer_output/   # Installer packages
│   └── iQue_Tools_Setup_v2.0.0.exe
├── tools/              # Build tools (Inno Setup)
├── docs/               # Documentation
│   └── ticket.sys_reference.md
├── iQueBrew/           # iQueBrew resources
├── ticket.sys-editor/  # Ticket.sys editor tool
├── ique_cbc_attack/    # CBC attack tool
├── bbp_pocs/           # Proof of concept exploits
├── gui/                # Web GUI prototype
├── iQue@home/          # iQue@home project
├── roms/               # Official ROMs (user-provided)
├── custom_roms/        # Custom/homebrew ROMs
└── working/            # Temporary working directory
```

## 🚀 Quick Start

### Option 1: Run the Installer
1. Download `iQue_Tools_Setup_v2.0.0.exe` from `installer_output/`
2. Run the installer
3. Launch "iQue Tools" from Start Menu

### Option 2: Run Standalone Exe
1. Copy `dist/iQue_Tools.exe` to a folder
2. Double-click to run

### Option 3: Run from Source
```bash
pip install customtkinter
python ique_tools.py
```

## 🎮 Supported Games (14 official titles)

| Game | CID | Release |
|------|-----|---------|
| Super Mario 64 | 1101104 | 2003-11-18 |
| Yoshi's Story | 1102101 | 2004-03-25 |
| Super Smash Bros. | 1201105 | 2005-02-24 |
| Zelda: Ocarina of Time | 2101104 | 2003-11-18 |
| Paper Mario | 2102104 | 2004-06-08 |
| Animal Crossing | 2104108 | 2006-01-27 |
| Custom Robo | 2105103 | 2004-12-25 |
| Star Fox 64 | 4101104 | 2003-11-18 |
| Sin & Punishment | 4102103 | 2004-09-25 |
| Wave Race 64 | 5101104 | 2003-11-18 |
| Excitebike 64 | 5102108 | 2004-06-25 |
| Mario Kart 64 | 5201104 | 2003-12-25 |
| F-Zero X | 5202103 | 2004-02-25 |
| Dr. Mario 64 | 6101104 | 2003-11-18 |

## 🔧 Prerequisites

Before using iQue Tools with your device:

1. **Install WinUSB Driver**
   - Download [Zadig](https://zadig.akeo.ie/)
   - Connect your iQue Player
   - Select the device in Zadig
   - Install WinUSB driver

2. **Add Your ROMs**
   - Place official `.z64` ROMs in the `roms/` folder
   - Name them to match the game titles (e.g., `Super Mario 64.z64`)

## 📦 Building from Source

### Build the Executable
```bash
pip install pyinstaller customtkinter
python -m PyInstaller --onefile --windowed --name "iQue_Tools" ique_tools.py
```

### Build the Installer
1. Open `installer.iss` in Inno Setup (in `tools/InnoSetup/`)
2. Press Ctrl+F9 to compile
3. Installer created in `installer_output/`

## 📋 Features

- **Game Selection** - Choose from 14 official iQue games
- **Device Connection** - Connect via USB (requires WinUSB)
- **ROM Management** - Organize official and custom ROMs
- **ticket.sys Dumping** - Backup your ticket database
- **Trial Conversion** - Convert trial games to full versions
- **Key Extraction** - Extract encryption keys
- **ROM Flashing** - Flash games to your device
- **Compatibility Tracking** - Track tested ROMs

## 📝 License

Various licenses apply - see individual tool directories for details.
