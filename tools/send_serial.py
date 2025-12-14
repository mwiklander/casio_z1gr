#!/usr/bin/env python3
import argparse
import os
import sys
import time
import serial

EOL_MAP = {
    ".bas": "\r\n",
    ".asm": "\r",
    ".casm": "\r",
    # If you later learn C-editor wants a specific EOL, add it here:
    # ".c": "\r"  or "\r\n" or "\n"
}

def normalize_text_for_z1(text: str, eol: str) -> str:
    # Normalize all incoming newlines first, then re-emit with target EOL
    text = text.replace("\r\n", "\n").replace("\r", "\n")
    lines = text.split("\n")

    # If file ends with newline, split() leaves last element "" — keep it as "already terminated"
    ended_with_newline = (len(lines) > 0 and lines[-1] == "")
    if ended_with_newline:
        lines = lines[:-1]

    out = eol.join(lines)

    # Always terminate the last line
    out += eol
    return out

def choose_eol(path: str, override: str | None) -> str:
    if override:
        # Allow tokens: CR, LF, CRLF
        key = override.upper()
        if key == "CR":
            return "\r"
        if key == "LF":
            return "\n"
        if key == "CRLF":
            return "\r\n"
        raise ValueError(f"Unknown --eol {override!r}. Use CR, LF, or CRLF.")

    ext = os.path.splitext(path)[1].lower()
    return EOL_MAP.get(ext, "\n")

def main():
    ap = argparse.ArgumentParser(description="Send a text program to Casio Z-1GR over USB serial.")
    ap.add_argument("path", help="Path to file to send (BAS/ASM/etc).")
    ap.add_argument("--port", default="/dev/cu.usbserial-B002AO7I")
    ap.add_argument("--baud", type=int, default=9600)
    ap.add_argument("--delay", type=float, default=0.05, help="Seconds between lines/blocks (default 0.05).")
    ap.add_argument("--ctrlz", action="store_true", help="Send CTRL-Z at end (recommended).")
    ap.add_argument("--eol", default=None, help="Override EOL: CR, LF, or CRLF.")
    ap.add_argument("--raw", action="store_true", help="Send file bytes as-is (no EOL normalization).")
    args = ap.parse_args()

    if not os.path.exists(args.path):
        print(f"File not found: {args.path}", file=sys.stderr)
        sys.exit(2)

    # Read file
    if args.raw:
        payload = open(args.path, "rb").read()
    else:
        # Read as text, but keep it strict-ish: you want to know if non-ASCII sneaks in
        text = open(args.path, "r", encoding="ascii", errors="strict").read()
        eol = choose_eol(args.path, args.eol)
        text = normalize_text_for_z1(text, eol)
        payload = text.encode("ascii")

    # Send
    ser = serial.Serial(args.port, args.baud, bytesize=8, parity="N", stopbits=1, timeout=1)
    try:
        # Small settle so the Z-1 is definitely listening
        time.sleep(args.delay)

        # Send in logical "lines" to respect pacing. For CR-only files, split on CR, etc.
        if args.raw:
            ser.write(payload)
            ser.flush()
        else:
            # Pace by line terminator used
            eol = choose_eol(args.path, args.eol)
            sep = eol.encode("ascii")
            parts = payload.split(sep)
            # split() drops terminators, so add them back
            for i, part in enumerate(parts):
                if part == b"" and i == len(parts) - 1:
                    break  # trailing after final terminator
                ser.write(part + sep)
                ser.flush()
                time.sleep(args.delay)

        # Let receiver digest final newline
        ser.flush()
        time.sleep(args.delay)

        if args.ctrlz:
            ser.write(b"\x1A")
            ser.flush()
            time.sleep(args.delay)

    finally:
        ser.close()

if __name__ == "__main__":
    main()
