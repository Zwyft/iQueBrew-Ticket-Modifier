🍺 iQueBrew Ticket Modifier

iQueBrew Ticket Modifier is an open-source, cross-platform utility for communicating with the iQue Player over USB.
It serves as a modern replacement for the official ique_diag tool included with iQue@Home — which is limited to Windows XP.

Unlike the original tool, iQueBrew works on modern operating systems and common PC platforms using libusb.

✨ Features

🔌 Communicates with the iQue Player over USB

💻 Cross-platform (Windows, Linux, macOS*)

🆓 Open-source replacement for ique_diag

⚡ Uses modern libusb 1.0

🛠 Designed for easier setup on modern systems

* macOS support depends on libusb availability and permissions.


📦 Installation
Dependencies

libusb 1.0

On most platforms, libusb is already installed or can be installed easily using your system’s package manager.

Windows Users

  Windows requires a libusb-compatible USB driver, such as WinUSB.

Recommended method:

  Install Zadig

  Connect your iQue Player

  Replace the device driver with WinUSB

Use the correct VID/PID (see project documentation or source)

📘 Refer to the official libusb Windows guide for additional details.

If necessary, download or compile a libusb-1.0 binary and ensure it is accessible to the program.

🛠 Building

See Building.md
 for detailed build instructions.

📜 Licensing

This project uses multiple licenses:

aulon is licensed under the GNU General Public License v3 (or later)
See LICENSE in the root directory.

Portions of ique.c (used in usb.c) are derived from ique_dumper by Mike Ryan,
released under the MIT License (LICENSES/MIT.txt).

libusb is licensed under the LGPL
(LICENSES/LGPL.txt).

⚠️ Known Issues
Windows Command Prompt Input Freezing

When running iQueBrew in Windows Command Prompt, operations may stall if:

QuickEdit Mode or Insert Mode is enabled

You click inside the terminal window during a read/write operation

Workarounds:

Disable QuickEdit and Insert Mode
(Right-click title bar → Properties)

Avoid clicking inside the terminal during USB operations

If the program pauses, press Enter immediately to resume

✅ This issue does not affect other terminals (PowerShell, Windows Terminal, etc.).

🤝 Contributing

Pull requests, issue reports, and documentation improvements are welcome.
If you’re working with iQue hardware or preservation tooling, your help is especially appreciated.
