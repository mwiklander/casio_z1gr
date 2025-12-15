# casio_z1gr

Experiments, tooling, and notes for driving machine-language routines on the Casio Z-1GR / FX-890P pocket computer.

## Repo Layout

- `programs/asm/` – Assembly sources for on-device testing.
- `programs/basic/` – Loader utilities written in BASIC.
- `tools/` – Host-side helpers (`send_serial.py`, `z1asm_lint.py`, opcode table, etc.).
- `docs/agent.md` – Working field notes distilled from emulator code and hardware probing.
- `z1gr_findings.yaml` – Canonical structured log of discoveries.

## Quick Start

1. Install VS Code’s x86 assembly grammar (`13xforever.language-x86-64-assembly`) for highlighting.
2. Either let `send_serial.py` do the preprocessing for you (default) or run `python3 tools/z1asm_lint.py path/to/file.asm --write-sibling` yourself to normalise assembler quirks (hex prefixes, unsupported `OUT DX,AH`, missing `END`).
3. Send the source with `python3 tools/send_serial.py path/to/file.asm --ctrlz` (the script auto-runs the linter unless you opt out with `--no-z1ify`), or use the “Send preprocessed ASM to Z-1GR” VS Code task.
4. On the calculator, allocate ML space (`CLEAR 2048,2048,6014`) and launch with `CALL &H2000` (or your chosen `ORG`).

## Tooling Highlights

- `tools/z1asm_lint.py` – Lints, rewrites unsupported syntax, appends `END`, and can emit an `_z1` sibling. Extend `tools/z1asm_opcodes.json` for missing mnemonics (e.g., `PUSHA`, `POPA`).
- `tools/send_serial.py` – Normalises line endings (CR for ASM), runs `z1asm_lint.py` automatically for ASM/CASM sources (toggle with `--[no-]z1ify`), and streams content over USB serial with optional CTRL-Z terminator.
- `.vscode/tasks.json` – Includes lint-only and preprocess+send tasks wired to the scripts above.

## Further Reading

- `docs/agent.md` summarises confirmed hardware behaviour, port mappings, and open questions.
- `z1gr_findings.yaml` retains the source-of-truth notes—update it first when new quirks are discovered, then mirror highlights into the docs.