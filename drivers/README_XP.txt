XP Driver Installation Instructions
===================================

The included `zadig.exe` is too new for Windows XP.
You must download the XP-compatible version.

1. On your MAIN PC (Host), download Zadig 2.2 from here:
   https://github.com/pbatard/libwdi/releases/download/b721/zadig_2.2.exe

   (Note: Zadig 2.2 is the last version to support Windows XP)

2. Copy `zadig_2.2.exe` into this `drivers` folder.

3. In your XP VM:
   - Run `zadig_2.2.exe`
   - Go to "Options" -> "List All Devices"
   - Select "iQue Player" (VID 1527)
   - Select "WinUSB" (or "libusb0" if WinUSB fails)
   - Click "Install Driver"

4. Restart the server script (`start_server_fixed.bat`)

Alternative:
If you cannot download Zadig, you can try to manually install the 
`ique_player.inf` via Device Manager, but this requires 
WinUSBCoInstaller2.dll which might be missing on your system.
zadig_2.2.exe is the recommended method.
