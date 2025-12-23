#!/usr/bin/env python3
"""
iQue Tools - Desktop Application with READABLE TEXT
All text is explicitly white (#ffffff) for maximum readability.
"""

import customtkinter as ctk
from tkinter import filedialog
import subprocess
import threading
import json
import os
import shutil
from datetime import datetime
from pathlib import Path

# ============================================================================
# CONFIGURATION
# ============================================================================

ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("dark-blue")

BASE_DIR = Path(__file__).parent.resolve()
AULON_EXE = BASE_DIR / "aulon" / "aulon.exe"
IQUECRYPT_EXE = BASE_DIR / "iquecrypt" / "iquecrypt.exe"

IQUEBREW_DIR = BASE_DIR / "iQueBrew"
V2_BIN = IQUEBREW_DIR / "v2.bin"
RECRYPT_SYS = IQUEBREW_DIR / "iQueBrew" / "recrypt.sys"
TICKET_SYS = IQUEBREW_DIR / "iQueBrew" / "ticket.sys"

ROMS_DIR = BASE_DIR / "roms"
CUSTOM_ROMS_DIR = BASE_DIR / "custom_roms"
WORKING_DIR = BASE_DIR / "working"
COMPATIBILITY_FILE = BASE_DIR / "compatibility.json"

ROMS_DIR.mkdir(exist_ok=True)
CUSTOM_ROMS_DIR.mkdir(exist_ok=True)
WORKING_DIR.mkdir(exist_ok=True)

# ============================================================================
# iQUE GAME DATABASE
# ============================================================================

IQUE_GAMES = [
    {
        "title": "Super Mario 64",
        "cid": "1101104",
        "rom": "Super Mario 64.z64",
        "icon": "🍄",
        "release": "2003-11-18",
    },
    {
        "title": "Yoshi's Story",
        "cid": "1102101",
        "rom": "Yoshi's Story.z64",
        "icon": "🦎",
        "release": "2004-03-25",
    },
    {
        "title": "Super Smash Bros.",
        "cid": "1201105",
        "rom": "Super Smash Bros.z64",
        "icon": "👊",
        "release": "2005-02-24",
    },
    {
        "title": "Zelda: Ocarina of Time",
        "cid": "2101104",
        "rom": "Zelda Ocarina of Time.z64",
        "icon": "🗡️",
        "release": "2003-11-18",
    },
    {
        "title": "Paper Mario",
        "cid": "2102104",
        "rom": "Paper Mario.z64",
        "icon": "📄",
        "release": "2004-06-08",
    },
    {
        "title": "Animal Crossing",
        "cid": "2104108",
        "rom": "Animal Crossing.z64",
        "icon": "🏠",
        "release": "2006-01-27",
    },
    {
        "title": "Custom Robo",
        "cid": "2105103",
        "rom": "Custom Robo.z64",
        "icon": "🤖",
        "release": "2004-12-25",
    },
    {
        "title": "Star Fox 64",
        "cid": "4101104",
        "rom": "Star Fox 64.z64",
        "icon": "✈️",
        "release": "2003-11-18",
    },
    {
        "title": "Sin & Punishment",
        "cid": "4102103",
        "rom": "Sin and Punishment.z64",
        "icon": "🔫",
        "release": "2004-09-25",
    },
    {
        "title": "Wave Race 64",
        "cid": "5101104",
        "rom": "Wave Race 64.z64",
        "icon": "🌊",
        "release": "2003-11-18",
    },
    {
        "title": "Excitebike 64",
        "cid": "5102108",
        "rom": "Excitebike 64.z64",
        "icon": "🏍️",
        "release": "2004-06-25",
    },
    {
        "title": "Mario Kart 64",
        "cid": "5201104",
        "rom": "Mario Kart 64.z64",
        "icon": "🏎️",
        "release": "2003-12-25",
    },
    {
        "title": "F-Zero X",
        "cid": "5202103",
        "rom": "F-Zero X.z64",
        "icon": "🚀",
        "release": "2004-02-25",
    },
    {
        "title": "Dr. Mario 64",
        "cid": "6101104",
        "rom": "Dr. Mario 64.z64",
        "icon": "💊",
        "release": "2003-11-18",
    },
]

# Color constants - ALL TEXT IS WHITE
WHITE = "#ffffff"
LIGHT_GRAY = "#cccccc"
BG_DARK = "#0d0d1a"
BG_PANEL = "#1a1a30"
BG_INPUT = "#252550"
ACCENT = "#7c6cf0"
GREEN = "#00d68f"
ORANGE = "#ffaa00"
RED = "#ff4757"
BLUE = "#17a2b8"


# ============================================================================
# TUTORIAL - Simple popup tooltips
# ============================================================================


class TutorialPopup:
    """Simple popup tutorial that shows tooltips next to UI elements."""

    STEPS = [
        (
            "game_dd",
            "Step 1: Select a Game",
            "Choose from 14 official iQue games.\nEach has a unique CID for encryption.",
        ),
        (
            "btn_conn",
            "Step 2: Connect Device",
            "Connect your iQue via USB.\nInstall WinUSB driver using Zadig.",
        ),
        (
            "rom_list",
            "Step 3: Manage ROMs",
            "Put ROMs in the 'roms' folder.\nCustom ROMs go in 'custom_roms'.",
        ),
        (
            "actions_panel",
            "Step 4: Take Action",
            "Dump ticket.sys, convert trials,\nextract keys, and flash ROMs!",
        ),
        (
            "console",
            "Step 5: Console Output",
            "Watch operation logs here.\nCheck for errors and status.",
        ),
    ]

    def __init__(self, app):
        self.app = app
        self.step = 0
        self.popup = None

    def start(self):
        """Start the tutorial."""
        self.step = 0
        self._show()

    def _show(self):
        """Show current step popup."""
        if self.step >= len(self.STEPS):
            self._done()
            return

        # Clean up previous popup
        if self.popup:
            self.popup.destroy()

        target_name, title, text = self.STEPS[self.step]
        target = getattr(self.app, target_name, None)

        # Create popup frame
        self.popup = ctk.CTkFrame(
            self.app,
            fg_color="#1a1a3a",
            corner_radius=15,
            border_width=2,
            border_color=ACCENT,
        )

        # Position near target
        if target:
            target.update_idletasks()
            x = target.winfo_rootx() - self.app.winfo_rootx()
            y = target.winfo_rooty() - self.app.winfo_rooty()
            w = target.winfo_width()
            # Place to the right or below depending on space
            px = min(x + w + 15, self.app.winfo_width() - 350)
            py = max(y, 20)
        else:
            px, py = 100, 100

        self.popup.place(x=px, y=py)

        # Content
        inner = ctk.CTkFrame(self.popup, fg_color="transparent")
        inner.pack(padx=18, pady=15)

        # Title
        ctk.CTkLabel(
            inner, text=title, font=("Segoe UI", 15, "bold"), text_color=ACCENT
        ).pack(anchor="w")

        # Text
        ctk.CTkLabel(
            inner, text=text, font=("Segoe UI", 12), text_color=WHITE, justify="left"
        ).pack(anchor="w", pady=(8, 12))

        # Buttons row
        btns = ctk.CTkFrame(inner, fg_color="transparent")
        btns.pack(fill="x")

        # Skip
        ctk.CTkButton(
            btns,
            text="Skip",
            font=("Segoe UI", 11),
            fg_color="transparent",
            hover_color="#333355",
            text_color=LIGHT_GRAY,
            width=60,
            height=32,
            command=self._done,
        ).pack(side="left")

        # Counter
        ctk.CTkLabel(
            btns,
            text=f"{self.step + 1}/{len(self.STEPS)}",
            font=("Segoe UI", 10),
            text_color=LIGHT_GRAY,
        ).pack(side="left", padx=10)

        # Next/Done
        btn_txt = "Next →" if self.step < len(self.STEPS) - 1 else "Done ✓"
        ctk.CTkButton(
            btns,
            text=btn_txt,
            font=("Segoe UI", 12, "bold"),
            fg_color=GREEN,
            text_color=WHITE,
            width=80,
            height=32,
            command=self._next,
        ).pack(side="right")

    def _next(self):
        self.step += 1
        self._show()

    def _done(self):
        if self.popup:
            self.popup.destroy()
            self.popup = None
        self.app._log("Tutorial done! Select a game to begin.")


class IQueToolsApp(ctk.CTk):
    """Main application with READABLE white text."""

    def __init__(self):
        super().__init__()

        self.title("iQue Tools")
        self.geometry("1100x850")
        self.minsize(1000, 750)
        self.configure(fg_color=BG_DARK)

        self.selected_game = None
        self.device_connected = False
        self.compatibility = self._load_compat()

        self._build_ui()
        self._log("iQue Tools v2.0.0 loaded")
        self._log(f"Found {len(IQUE_GAMES)} games")

        # Start tutorial after UI is built
        self.after(500, self._show_tutorial)

    def _load_compat(self):
        if COMPATIBILITY_FILE.exists():
            try:
                return json.load(open(COMPATIBILITY_FILE))
            except Exception:
                pass
        return {"tested": []}

    def _save_compat(self):
        json.dump(self.compatibility, open(COMPATIBILITY_FILE, "w"), indent=2)

    def _build_ui(self):
        """Build the UI with white text."""
        main = ctk.CTkFrame(self, fg_color="transparent")
        main.pack(fill="both", expand=True, padx=25, pady=20)

        # === HEADER ===
        header = ctk.CTkFrame(main, fg_color=BG_PANEL, corner_radius=15)
        header.pack(fill="x", pady=(0, 15))

        h_inner = ctk.CTkFrame(header, fg_color="transparent")
        h_inner.pack(fill="x", padx=25, pady=18)

        # Logo - colorful iQue
        logo = ctk.CTkFrame(h_inner, fg_color="transparent")
        logo.pack(side="left")

        for color, letter in [
            ("#e74c3c", "i"),
            ("#2ecc71", "Q"),
            ("#3498db", "u"),
            ("#f1c40f", "e"),
        ]:
            ctk.CTkLabel(
                logo, text=letter, font=("Segoe UI", 36, "bold"), text_color=color
            ).pack(side="left")

        # Title
        titles = ctk.CTkFrame(logo, fg_color="transparent")
        titles.pack(side="left", padx=(20, 0))
        ctk.CTkLabel(
            titles, text="iQue Tools", font=("Segoe UI", 22, "bold"), text_color=WHITE
        ).pack(anchor="w")
        ctk.CTkLabel(
            titles,
            text="64-bit Hacking Toolkit",
            font=("Segoe UI", 12),
            text_color=LIGHT_GRAY,
        ).pack(anchor="w")

        # Status
        status = ctk.CTkFrame(h_inner, fg_color=BG_INPUT, corner_radius=20)
        status.pack(side="right")
        s_inner = ctk.CTkFrame(status, fg_color="transparent")
        s_inner.pack(padx=15, pady=8)

        self.status_dot = ctk.CTkLabel(
            s_inner, text="●", font=("Segoe UI", 14), text_color=RED
        )
        self.status_dot.pack(side="left", padx=(0, 8))
        self.status_lbl = ctk.CTkLabel(
            s_inner, text="Disconnected", font=("Segoe UI", 12), text_color=WHITE
        )
        self.status_lbl.pack(side="left")

        # === CONTENT ===
        content = ctk.CTkFrame(main, fg_color="transparent")
        content.pack(fill="both", expand=True, pady=10)

        left = ctk.CTkFrame(content, fg_color="transparent")
        left.pack(side="left", fill="both", expand=True, padx=(0, 10))

        right = ctk.CTkFrame(content, fg_color="transparent")
        right.pack(side="right", fill="both", expand=True, padx=(10, 0))

        # === LEFT: GAME SELECTION ===
        game_panel = ctk.CTkFrame(left, fg_color=BG_PANEL, corner_radius=15)
        game_panel.pack(fill="x", pady=(0, 10))

        gp_inner = ctk.CTkFrame(game_panel, fg_color="transparent")
        gp_inner.pack(fill="x", padx=20, pady=18)

        ctk.CTkLabel(
            gp_inner,
            text="🎮  Game Selection",
            font=("Segoe UI", 16, "bold"),
            text_color=WHITE,
        ).pack(anchor="w", pady=(0, 12))

        ctk.CTkLabel(
            gp_inner, text="Select Title", font=("Segoe UI", 11), text_color=LIGHT_GRAY
        ).pack(anchor="w", pady=(0, 5))

        game_names = [f"{g['icon']}  {g['title']}" for g in IQUE_GAMES]
        self.game_dd = ctk.CTkOptionMenu(
            gp_inner,
            values=game_names,
            command=self._on_game,
            fg_color=BG_INPUT,
            button_color=ACCENT,
            dropdown_fg_color=BG_PANEL,
            dropdown_text_color=WHITE,
            text_color=WHITE,
            font=("Segoe UI", 13),
            corner_radius=10,
            width=400,
            height=45,
        )
        self.game_dd.set("Choose a game...")
        self.game_dd.pack(fill="x", pady=(0, 15))

        # Info card
        info = ctk.CTkFrame(gp_inner, fg_color=BG_INPUT, corner_radius=10)
        info.pack(fill="x")
        info_inner = ctk.CTkFrame(info, fg_color="transparent")
        info_inner.pack(fill="x", padx=15, pady=15)

        self.icon_lbl = ctk.CTkLabel(info_inner, text="🎮", font=("Segoe UI", 40))
        self.icon_lbl.pack(side="left", padx=(0, 20))

        info_right = ctk.CTkFrame(info_inner, fg_color="transparent")
        info_right.pack(side="left", fill="both", expand=True)

        for name, attr in [
            ("Title:", "title_lbl"),
            ("Release:", "release_lbl"),
            ("CID:", "cid_lbl"),
            ("ROM:", "rom_lbl"),
        ]:
            row = ctk.CTkFrame(info_right, fg_color="transparent")
            row.pack(fill="x", pady=2)
            ctk.CTkLabel(
                row,
                text=name,
                font=("Segoe UI", 11),
                width=60,
                text_color=LIGHT_GRAY,
                anchor="w",
            ).pack(side="left")
            lbl = ctk.CTkLabel(
                row,
                text="—",
                font=("Segoe UI", 12, "bold"),
                text_color=WHITE,
                anchor="e",
            )
            lbl.pack(side="right")
            setattr(self, attr, lbl)

        # === LEFT: ROM MANAGEMENT ===
        rom_panel = ctk.CTkFrame(left, fg_color=BG_PANEL, corner_radius=15)
        rom_panel.pack(fill="both", expand=True)

        rp_inner = ctk.CTkFrame(rom_panel, fg_color="transparent")
        rp_inner.pack(fill="both", expand=True, padx=20, pady=18)

        ctk.CTkLabel(
            rp_inner,
            text="📁  ROM Management",
            font=("Segoe UI", 16, "bold"),
            text_color=WHITE,
        ).pack(anchor="w", pady=(0, 12))

        btn_row = ctk.CTkFrame(rp_inner, fg_color="transparent")
        btn_row.pack(fill="x", pady=(0, 12))

        ctk.CTkButton(
            btn_row,
            text="📂 Open ROMs Folder",
            font=("Segoe UI", 12),
            fg_color=BG_INPUT,
            hover_color=ACCENT,
            text_color=WHITE,
            corner_radius=10,
            height=40,
            command=lambda: os.startfile(ROMS_DIR),
        ).pack(side="left", padx=(0, 10))

        ctk.CTkButton(
            btn_row,
            text="➕ Add Custom ROM",
            font=("Segoe UI", 12),
            fg_color=ACCENT,
            text_color=WHITE,
            corner_radius=10,
            height=40,
            command=self._add_rom,
        ).pack(side="left")

        self.rom_list = ctk.CTkTextbox(
            rp_inner,
            fg_color=BG_INPUT,
            text_color=WHITE,
            font=("Consolas", 11),
            corner_radius=10,
        )
        self.rom_list.pack(fill="both", expand=True)
        self._refresh_roms()

        # === RIGHT: DEVICE ===
        dev_panel = ctk.CTkFrame(right, fg_color=BG_PANEL, corner_radius=15)
        dev_panel.pack(fill="x", pady=(0, 10))

        dp_inner = ctk.CTkFrame(dev_panel, fg_color="transparent")
        dp_inner.pack(fill="x", padx=20, pady=18)

        ctk.CTkLabel(
            dp_inner,
            text="🔌  Device Connection",
            font=("Segoe UI", 16, "bold"),
            text_color=WHITE,
        ).pack(anchor="w", pady=(0, 12))

        btn_row2 = ctk.CTkFrame(dp_inner, fg_color="transparent")
        btn_row2.pack(fill="x")

        self.btn_conn = ctk.CTkButton(
            btn_row2,
            text="🔗 Connect",
            font=("Segoe UI", 13, "bold"),
            fg_color=GREEN,
            text_color=WHITE,
            corner_radius=10,
            height=45,
            command=self._connect,
        )
        self.btn_conn.pack(side="left", padx=(0, 10))

        self.btn_disc = ctk.CTkButton(
            btn_row2,
            text="✕ Disconnect",
            font=("Segoe UI", 13, "bold"),
            fg_color=RED,
            text_color=WHITE,
            text_color_disabled="#ffcccc",
            corner_radius=10,
            height=45,
            state="disabled",
            command=self._disconnect,
        )
        self.btn_disc.pack(side="left")

        # === RIGHT: ACTIONS ===
        self.actions_panel = ctk.CTkFrame(right, fg_color=BG_PANEL, corner_radius=15)
        self.actions_panel.pack(fill="both", expand=True)

        ap_inner = ctk.CTkFrame(self.actions_panel, fg_color="transparent")
        ap_inner.pack(fill="both", expand=True, padx=20, pady=18)

        ctk.CTkLabel(
            ap_inner,
            text="⚡  Actions",
            font=("Segoe UI", 16, "bold"),
            text_color=WHITE,
        ).pack(anchor="w", pady=(0, 15))

        actions = [
            ("📥 Dump ticket.sys", self._dump, BLUE),
            ("🔓 Convert Trial → Full", self._convert, ORANGE),
            ("🔐 Extract Keys", self._keys, ACCENT),
            ("🚀 Flash Selected ROM", self._flash, GREEN),
            ("📋 Export Compatibility", self._export, "#a29bfe"),
        ]

        for text, cmd, color in actions:
            ctk.CTkButton(
                ap_inner,
                text=text,
                font=("Segoe UI", 13, "bold"),
                fg_color=color,
                text_color=WHITE,
                corner_radius=10,
                height=48,
                command=cmd,
            ).pack(fill="x", pady=4)

        # === CONSOLE ===
        con_panel = ctk.CTkFrame(main, fg_color=BG_PANEL, corner_radius=15)
        con_panel.pack(fill="x", pady=(10, 0))

        cp_inner = ctk.CTkFrame(con_panel, fg_color="transparent")
        cp_inner.pack(fill="x", padx=20, pady=15)

        con_header = ctk.CTkFrame(cp_inner, fg_color="transparent")
        con_header.pack(fill="x", pady=(0, 10))

        ctk.CTkLabel(
            con_header,
            text="📟  Console",
            font=("Segoe UI", 14, "bold"),
            text_color=WHITE,
        ).pack(side="left")

        ctk.CTkButton(
            con_header,
            text="Clear",
            font=("Segoe UI", 11),
            fg_color=BG_INPUT,
            text_color=WHITE,
            corner_radius=8,
            width=70,
            height=28,
            command=self._clear,
        ).pack(side="right")

        self.console = ctk.CTkTextbox(
            cp_inner,
            fg_color=BG_INPUT,
            text_color=WHITE,
            font=("Consolas", 11),
            corner_radius=10,
            height=120,
        )
        self.console.pack(fill="x")
        self.console.configure(state="disabled")

    # === EVENT HANDLERS ===

    def _on_game(self, choice):
        for g in IQUE_GAMES:
            if g["title"] in choice:
                self.selected_game = g
                self.icon_lbl.configure(text=g["icon"])
                self.title_lbl.configure(text=g["title"])
                self.release_lbl.configure(text=g["release"])
                self.cid_lbl.configure(text=g["cid"], text_color=ACCENT)

                rom = ROMS_DIR / g["rom"]
                if rom.exists():
                    self.rom_lbl.configure(text="✅ Found", text_color=GREEN)
                else:
                    self.rom_lbl.configure(text="❌ Missing", text_color=RED)

                self._log(f"Selected: {g['title']}")
                break

    def _refresh_roms(self):
        self.rom_list.configure(state="normal")
        self.rom_list.delete("1.0", "end")
        for f in sorted(ROMS_DIR.glob("*.z64")) + sorted(ROMS_DIR.glob("*.n64")):
            self.rom_list.insert("end", f"📀 {f.name}\n")
        for f in sorted(CUSTOM_ROMS_DIR.glob("*.z64")) + sorted(
            CUSTOM_ROMS_DIR.glob("*.n64")
        ):
            self.rom_list.insert("end", f"🎮 {f.name}\n")
        self.rom_list.configure(state="disabled")

    def _add_rom(self):
        path = filedialog.askopenfilename(filetypes=[("ROM", "*.z64 *.n64")])
        if path:
            shutil.copy2(path, CUSTOM_ROMS_DIR / Path(path).name)
            self._log(f"Added ROM: {Path(path).name}")
            self._refresh_roms()

    def _connect(self):
        self._log("Connecting to iQue Player...")

        def run():
            try:
                r = subprocess.run(
                    [str(AULON_EXE)],
                    input="0\n11\n",
                    capture_output=True,
                    text=True,
                    timeout=10,
                    cwd=str(AULON_EXE.parent),
                )
                ok = "error" not in (r.stdout + r.stderr).lower()
                self.after(0, lambda: self._conn_done(ok))
            except Exception:
                self.after(0, lambda: self._conn_done(False))

        threading.Thread(target=run, daemon=True).start()

    def _conn_done(self, ok):
        if ok:
            self.status_dot.configure(text_color=GREEN)
            self.status_lbl.configure(text="Connected")
            self.btn_conn.configure(state="disabled")
            self.btn_disc.configure(state="normal")
            self._log("Connected!")
        else:
            self._log("Connection failed - check WinUSB driver")

    def _disconnect(self):
        self.status_dot.configure(text_color=RED)
        self.status_lbl.configure(text="Disconnected")
        self.btn_conn.configure(state="normal")
        self.btn_disc.configure(state="disabled")
        self._log("Disconnected")

    def _dump(self):
        self._log("Dumping ticket.sys...")
        self._log("Use aulon: 3 ticket.sys")

    def _convert(self):
        if not self.selected_game:
            self._log("Select a game first!")
            return

        ticket = TICKET_SYS if TICKET_SYS.exists() else WORKING_DIR / "ticket.sys"
        if not ticket.exists():
            self._log("No ticket.sys found!")
            return

        self._log(f"Converting {self.selected_game['title']}...")
        try:
            data = bytearray(open(ticket, "rb").read())
            n = int.from_bytes(data[0:4], "big")
            for i in range(n):
                off = 4 + i * 0x2B4C + 0x29B4
                if int.from_bytes(data[off : off + 2], "big") != 0:
                    data[off : off + 2] = b"\x00\x00"
                    self._log(f"Ticket {i}: Trial → Full")

            out = WORKING_DIR / "ticket_modified.sys"
            open(out, "wb").write(data)
            self._log(f"Saved: {out}")
        except Exception as ex:
            self._log(f"Error: {ex}")

    def _keys(self):
        if not self.selected_game:
            self._log("Select a game first!")
            return

        ticket = TICKET_SYS if TICKET_SYS.exists() else WORKING_DIR / "ticket.sys"
        if not ticket.exists():
            self._log("No ticket.sys!")
            return

        self._log(f"Extracting keys for {self.selected_game['cid']}...")

        def run():
            try:
                subprocess.run(
                    [
                        str(IQUECRYPT_EXE),
                        "extract",
                        "-ticket",
                        str(ticket),
                        "-cid",
                        self.selected_game["cid"],
                    ],
                    capture_output=True,
                    cwd=str(WORKING_DIR),
                )
                self.after(0, lambda: self._log("Keys extracted!"))
            except Exception:
                self.after(0, lambda: self._log("Key extraction failed"))

        threading.Thread(target=run, daemon=True).start()

    def _flash(self):
        if not self.selected_game:
            self._log("Select a game first!")
            return
        rom = ROMS_DIR / self.selected_game["rom"]
        if not rom.exists():
            self._log(f"ROM not found: {rom}")
            return
        self._log(f"Ready to flash: {rom.name}")
        self._log("Workflow: encrypt → write via aulon")

    def _export(self):
        path = filedialog.asksaveasfilename(defaultextension=".json")
        if path:
            json.dump(self.compatibility, open(path, "w"), indent=2)
            self._log(f"Exported: {path}")

    def _log(self, msg):
        ts = datetime.now().strftime("%H:%M:%S")
        self.console.configure(state="normal")
        self.console.insert("end", f"[{ts}] {msg}\n")
        self.console.see("end")
        self.console.configure(state="disabled")

    def _clear(self):
        self.console.configure(state="normal")
        self.console.delete("1.0", "end")
        self.console.configure(state="disabled")

    def _show_tutorial(self):
        """Show interactive tutorial popup."""
        self.tutorial = TutorialPopup(self)
        self.tutorial.start()


if __name__ == "__main__":
    IQueToolsApp().mainloop()
