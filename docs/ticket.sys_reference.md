# iQue Player ticket.sys Reference

## Overview
`ticket.sys` is a critical system file on the iQue Player that contains game licenses (tickets) for all installed apps. It uses AES-128-CBC encryption and supports a maximum of **48 tickets**.

## File Structure

### Header
| Offset | Size | Description |
|--------|------|-------------|
| 0x00 | 4 | `numTickets` - Number of tickets in file |

Tickets follow immediately after the header at offset 0x04.

### Individual Ticket Structure (0x2B4C = 11,084 bytes each)

Each ticket = `contentDesc` (0x2800 bytes) + `BbContentMetaDataHead` (0x34C bytes)

#### contentDesc (0x2800 bytes) - Game info, title, thumbnail

| Offset | Size | Field |
|--------|------|-------|
| 0x00-0x03 | 4 | EEPROM RDRAM location |
| 0x04-0x07 | 4 | EEPROM size |
| 0x08-0x0B | 4 | Flash RDRAM location |
| 0x0C-0x0F | 4 | Flash size |
| 0x10-0x13 | 4 | SRAM RDRAM location |
| 0x14-0x17 | 4 | SRAM size |
| 0x18-0x2B | 16 | Controller Pak locations (4x 4 bytes) |
| 0x2C-0x2F | 4 | Controller Pak size |
| 0x30-0x33 | 4 | osRomBase |
| 0x34-0x37 | 4 | osTvType |
| 0x38-0x3B | 4 | osMemSize |
| 0x3C-0x43 | 8 | Unknown values |
| **0x44-0x46** | 3 | **Magic: "CAM"** (validation) |
| 0x47 | 1 | Number of .u0x files |
| 0x44-0x45 | 2 | Thumb image length (at 0x44 within ticket for offset calc) |
| 0x46-0x47 | 2 | Title image length |
| 0x48+ | var | Thumb image (RGBA5551, 56x56, deflate compressed) |
| 0x48+thumb | var | Title image (IA8, 184x24, deflate compressed) |
| End | var | Game title/ISBN (GB2312 encoded, null-separated) |

#### BbContentMetaDataHead (CMD) - Starting at 0x2800

| Offset | Size | Field |
|--------|------|-------|
| 0x2800-0x2803 | 4 | Padding |
| 0x2804-0x2807 | 4 | CA CRL version |
| 0x2808-0x280B | 4 | CP CRL version |
| 0x280C-0x280F | 4 | Content size |
| 0x2810-0x2813 | 4 | Unknown value |
| 0x2814-0x2823 | 16 | Titlekey IV |
| 0x2824-0x2837 | 20 | SHA-1 hash of plaintext |
| 0x2838-0x2847 | 16 | Content IV |
| 0x2848-0x284B | 4 | Exec flags |
| 0x284C-0x284F | 4 | Hardware access rights |
| 0x2850-0x2853 | 4 | Padding |
| 0x2854-0x2857 | 4 | CMD BBID |
| 0x2858-0x2897 | 64 | CMD certificate |
| **0x289C-0x289F** | 4 | **CID (Content ID)** |
| 0x28AC-0x29AB | 256 | Titlekey |
| 0x29AC-0x29AF | 4 | Signature data |
| 0x29B0-0x29B1 | 2 | Ticket BBID |
| **0x29B2-0x29B3** | 2 | **Ticket ID** |
| 0x29B4-0x29B5 | 2 | Trial type (0=full, else trial) |
| 0x29B6-0x29B7 | 2 | Trial minutes/launches |
| 0x29B8-0x29BB | 4 | Padding |
| 0x29BC-0x29CB | 16 | Ticket CRL version |
| 0x29CC-0x2A0B | 64 | Titlekey IV 2 |
| 0x2A0C-0x2A4B | 64 | ECC public key |
| 0x2A4C-0x2B4B | 256 | Ticket certificate |
| End | var | Signature |

## Image Formats

### Thumb Image (56x56)
- Format: **RGBA5551**
- 5 bits red, 5 bits green, 5 bits blue, 1 bit alpha
- Compressed with deflate (raw, no zlib header)

### Title Image (184x24)
- Format: **IA8** (Intensity + Alpha)
- 1 byte intensity (grayscale), 1 byte alpha
- Compressed with deflate

## Validation
- Magic bytes `CAM` at offset 0x44 (from ticket start)
- For individual ticket.dat: Magic at 0x40

## Known Content IDs (CIDs)
Examples from documented sources:
- `004f60b7` - Example game
- `7FFF` - iQue Club (special handling)

## File Sizes
| Format | Size | Description |
|--------|------|-------------|
| ticket.sys | var | Multiple tickets + padding to 0x4000 |
| ticket.dat | 0x2B4C | Single exported ticket |
| .cdesc | 0x2800 | contentDesc only |
| .cmd | 0x29AC | contentDesc + BbContentMetaDataHead |

## Related Files
- `recrypt.sys` - Recryption keys per title
- `.rec` files - Encrypted game content
- `virage2.bin` - Console-specific 256-byte private data

## References
- [iQueBrew Wiki - Ticket.sys](http://www.iquebrew.org/index.php?title=Ticket.sys)
- [iQueBrew Wiki - CMD](http://www.iquebrew.org/index.php?title=CMD)
- [ticket.sys-editor](https://github.com/iQueBrew/ticket.sys-editor) - Python GUI editor
- Local sample: `ticket.sys-editor/files/ticket.dat`
