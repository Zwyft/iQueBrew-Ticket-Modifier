#!/usr/bin/env python3
import struct
import shutil
import os
import sys

# iQue Game Title Mappings (GB2312/Raw Bytes -> English ASCII)
# Derived from aulon/gui/modern_gui.cpp
GAME_MAPPINGS = {
    # Dr. Mario 64
    b"\xc2\xed\xc1\xa6\xc5\xb7\xd2\xbd\xc9\xfa": "Dr. Mario 64",
    # Zelda: Ocarina of Time
    b"\xc8\xfb\xb6\xfb\xb4\xef": "Zelda: Ocarina of Time",
    # Super Mario 64
    b"\xc9\xf1\xd3\xce\xc2\xed\xc1\xa6\xc5\xb7": "Super Mario 64",
    # Wave Race 64
    b"\xcb\xae\xc9\xcf\xc4\xa6\xcd\xd0": "Wave Race 64",
    # Star Fox 64
    b"\xd0\xc7\xbc\xca\xbb\xf0\xba\xfc": "Star Fox 64",
    # Yoshi's Story
    b"\xd2\xab\xce\xf7\xb9\xca\xca\xc2": "Yoshi's Story",
    # Super Smash Bros.
    b"\xc8\xce\xcc\xec\xcc\xc3": "Super Smash Bros.",
    # Paper Mario
    b"\xd6\xbd\xc6\xac\xc2\xed\xc1\xa6\xc5\xb7": "Paper Mario",
    # Animal Crossing
    b"\xb6\xaf\xce\xef\xc9\xad\xc1\xd6": "Animal Crossing",
    # Custom Robo
    b"\xd7\xe9\xba\xcf\xbb\xfa\xc6\xf7\xc8\xcb": "Custom Robo",
    # Sin & Punishment
    b"\xd7\xef\xd3\xeb\xb7\xa3": "Sin & Punishment",
    # Excitebike 64
    b"\xd4\xbd\xd2\xb0\xc4\xa6\xcd\xd0": "Excitebike 64",
    # Mario Kart 64
    b"\xc2\xed\xc1\xa6\xc5\xb7\xbf\xa8\xb6\xa1\xb3\xb5": "Mario Kart 64",
    # F-Zero X (Already ASCII, but good to ensure consistency)
    b"F-Zero X": "F-Zero X",
}

TICKET_SIZE = 0x2B4C
HEADER_SIZE = 4


def localize_ticket(input_path, output_path):
    if not os.path.exists(input_path):
        print(f"Error: Input file '{input_path}' not found.")
        return

    print(f"Reading {input_path}...")
    with open(input_path, "rb") as f:
        data = bytearray(f.read())

    # Read num_tickets (Big Endian)
    num_tickets = struct.unpack(">I", data[0:4])[0]
    print(f"Found {num_tickets} tickets.")

    filesize = len(data)
    expected_size = HEADER_SIZE + (num_tickets * TICKET_SIZE)

    # iQue ticket.sys often has padding to 0x4000 boundaries, check alignment
    if filesize < expected_size:
        print(
            f"Warning: File size ({filesize}) is smaller than expected for {num_tickets} tickets ({expected_size})."
        )

    count = 0

    for i in range(num_tickets):
        offset = HEADER_SIZE + (i * TICKET_SIZE)

        # Safety check for file bounds
        if offset + TICKET_SIZE > len(data):
            print("Error: Ticket data inconsistent with file size.")
            break

        # Extract ticket buffer for analysis
        ticket_data = data[offset : offset + TICKET_SIZE]

        # Calculate Name Offset
        # thumb_len at 0x44 (2 bytes), title_len at 0x46 (2 bytes) - Big Endian
        thumb_len = struct.unpack(">H", ticket_data[0x44:0x46])[0]
        title_len = struct.unpack(">H", ticket_data[0x46:0x48])[0]

        name_offset_rel = 0x48 + thumb_len + title_len
        if name_offset_rel >= 0x2800:  # Name must be within contentDesc
            print(f"Ticket {i}: Name offset out of bounds.")
            continue

        # Calculate absolute position of name in the main data buffer
        name_abs_start = offset + name_offset_rel
        # Determine max length (up to end of contentDesc or reasonable limit)
        # contentDesc ends at offset + 0x2800
        name_max_len = (offset + 0x2800) - name_abs_start

        # Read the current name
        current_name_bytes = bytearray()
        for k in range(name_max_len):
            b = data[name_abs_start + k]
            if b == 0:  # Null termination
                break
            current_name_bytes.append(b)

        current_name_raw = bytes(current_name_bytes)

        found = False
        new_name = ""

        # Try to match against known mappings
        for zh_bytes, en_name in GAME_MAPPINGS.items():
            # Check if the raw bytes contain the Chinese name or are exactly it
            if zh_bytes in current_name_raw:
                new_name = en_name
                found = True
                break
            # Fallback: Check if it's already the English name
            elif current_name_raw == en_name.encode("ascii"):
                print(f"Ticket {i}: Already English ({en_name})")
                found = True  # No change needed but valid
                break

        if found and new_name:
            print(f"Ticket {i}: Patching '{current_name_raw.hex()}' -> '{new_name}'")

            # Write new name
            new_name_bytes = new_name.encode("ascii")

            # Write bytes
            for k, b in enumerate(new_name_bytes):
                data[name_abs_start + k] = b

            # Write Null Terminator
            data[name_abs_start + len(new_name_bytes)] = 0
            count += 1

        elif not found:
            print(
                f"Ticket {i}: No mapping found for name bytes: {current_name_raw.hex()} ({current_name_raw})"
            )

    print(f"Patched {count} titles.")

    with open(output_path, "wb") as f:
        f.write(data)
    print(f"Saved to {output_path}")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python localize_ticket.py <ticket.sys> [output_file]")
        sys.exit(1)

    input_f = sys.argv[1]
    output_f = "ticket_english.sys"
    if len(sys.argv) > 2:
        output_f = sys.argv[2]

    localize_ticket(input_f, output_f)
