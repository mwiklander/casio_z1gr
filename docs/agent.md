# Casio Z-1GR Field Notes

This document distills the current working knowledge about writing and deploying machine-language routines to the Casio Z-1GR / FX-890P based on on-device experiments and analysis of the `z1f9_20210529` emulator sources. It should save future spelunking sessions a few hours.

## Hardware Snapshot

- CPU: Intel 80188-class (TS80L188EB), iAPX 186/188 compatible.
- Memory model (emulator): 20-bit physical address map with RAM at `00000h–3FFFFh` and ROM windowed at `E0000h–FFFFFh`.
- BASIC hands off to ML code through a dispatcher that expects you to return with `IRET`.

## Workflow Overview

1. Allocate ML workspace from BASIC using `CLEAR string, asm, var` (example: `CLEAR 2048,2048,6014`).
2. Assemble source with the on-device assembler or preprocess it on the host using `tools/z1asm_lint.py`.
3. Send the ASCII program over serial via `python3 tools/send_serial.py path --ctrlz` (VS Code task available).
4. Launch from BASIC with `CALL &Haddr` where `addr` matches your `ORG`.

## Assembler Quirks (Confirmed on Device)

- Labels **must** end with `:`.
- Hex literals must end with `H` and start with a digit; prefix values like `A000H` as `0A000H`.
- `DUP`, `?`, and other MASM-style placeholders are rejected—define data explicitly.
- `OUT DX,AL` and `OUT DX,AX` are accepted; `OUT DX,AH` is not (rewrite through `MOV AL,AH`).
- `IN AX,DX` works; `IN AH,DX` does not.
- `IRET` is mandatory for returning to BASIC; `RET/RETF` causes a freeze.
- `EQU` labels used as 16-bit immediates proved unreliable. Load constants via byte halves instead.

### Helper Script: `z1asm_lint.py`

`python3 tools/z1asm_lint.py file.asm --write-sibling`

- Normalises hex literals, strips unsupported constructs, and appends `END` when missing.
- Rewrites `OUT DX,AH` into `MOV AL,AH` + `OUT DX,AL`.
- Provides an opcode escape hatch via `tools/z1asm_opcodes.json` (e.g., `PUSHA`/`POPA`).
- Emits warnings for suspected 80186-only instructions so you can supply byte sequences.

## Key I/O Ports

### Keyboard (confirmed on device)

- Strobe mask: `0200h` (write, 16-bit).
- Matrix read: `0202h` (read, 16-bit). Idle value observed: `8000h`.

### Buzzer (confirmed on device)

- Data port: `0206h` (write, 8-bit). Value `03h` yields a short tick; `00h` silences.

### Timers (emulator-derived, unverified)

- `0030h–0036h` map to Timer0 counter/interval/control (16-bit).
- Similar layouts for Timer1/2. Enable bit is bit15 (`8000h`). Needs on-device validation.

### Interrupt Controller (emulator-derived)

- Global mask: `0008h` (8-bit, R/W).
- Current IRQ: `000Ch` (8-bit, R).
- End-of-interrupt: `0002h` (write `0xFF`).

## VRAM / LCD Warnings

- Emulator maps LCD to `A0000h`, but direct writes with `DS=0A000h` have locked real hardware.
- Avoid poking VRAM until we confirm the proper ROM routines or port protocol.

## Known Working Snippet

```asm
ORG  2000H

START:
      MOV  DX,0200H
      MOV  AX,0FFFH
      OUT  DX,AX

WAIT:
      MOV  DX,0202H
      IN   AX,DX
      CMP  AX,8000H
      JE   WAIT

      MOV  DX,0206H
      MOV  AL,03H
      OUT  DX,AL
      MOV  AL,00H
      OUT  DX,AL

      IRET
```

## Open Questions

- **LCD access:** Determine the sanctioned way to draw pixels (ROM call vs. hidden port setup).
- **Timer behaviour:** Confirm counters and interrupts on real hardware.
- **Keyboard map:** Build a row/column → key table by strobing individual bits and logging results.

### Suggested Next Steps

1. Instrument `z1asm_lint.py` to flag `EQU`-based immediates and offer BH/BL scaffolding.
2. Capture serial session logs for successful uploads to double-check newline pacing.
3. Extract ROM call vector table from emulator or ROM dump and map to useful services (LCD, keyboard, timers).

Keep this file updated as you confirm more behaviour on the real hardware.
