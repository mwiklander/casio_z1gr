#!/usr/bin/env python3
import argparse, time
import serial

# Usage
# python3 tools/send_serial.py programs/HELLO.BAS --ctrlz


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("path")
    ap.add_argument("--port", default="/dev/cu.usbserial-B002AO7I")
    ap.add_argument("--baud", type=int, default=9600)
    ap.add_argument("--delay", type=float, default=0.05, help="seconds between lines")
    ap.add_argument("--ctrlz", action="store_true", help="send Ctrl-Z at end")
    args = ap.parse_args()

    ser = serial.Serial(args.port, args.baud, bytesize=8, parity="N", stopbits=1, timeout=1)

    with open(args.path, "rb") as f:
        for line in f:
            ser.write(line)
            time.sleep(args.delay)

    if args.ctrlz:
        ser.write(b"\x1A")      # CTRL-Z
        ser.flush()
        time.sleep(args.delay)  # give it time to see EOF

    ser.close()

if __name__ == "__main__":
    main()
