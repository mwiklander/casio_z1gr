#!/usr/bin/env python3
"""Lint and preprocess Z-1GR assembler sources.

Handles a handful of confirmed assembler quirks and provides an opcode
escape hatch so that unsupported mnemonics can be mapped to byte
sequences (via DB directives).
"""
from __future__ import annotations

import argparse
import dataclasses
import json
import pathlib
import re
import sys
from typing import Dict, Iterable, List, Optional, Sequence, Tuple

HEX_NEEDS_LEADING_ZERO = re.compile(r"\b([A-Fa-f][0-9A-Fa-f]*)H\b")
OUT_DX_AH_PATTERN = re.compile(
    r"^(?P<indent>\s*)(?:(?P<label>[A-Za-z_][\w$]*):(?P<after_ws>\s*))?(?P<out>OUT)\s+DX\s*,\s*AH\s*$",
    re.IGNORECASE,
)
INSTRUCTION_PATTERN = re.compile(
    r"^(?P<indent>\s*)(?:(?P<label>[A-Za-z_][\w$]*):(?P<after_ws>\s*))?(?P<mnemonic>[A-Za-z_][\w$]*)\b(?P<rest>.*)$",
    re.IGNORECASE,
)
IMMEDIATE_PATTERN = re.compile(r"^(?:0[xX][0-9A-Fa-f]+|[0-9]+|[0-9A-Fa-f]+H)$")
SHIFT_WITH_IMMEDIATE = {"SHL", "SAL", "SHR", "SAR"}
TODO_80186_ONLY = {"ENTER", "LEAVE", "IMUL", "PUSHA", "POPA"}


@dataclasses.dataclass(slots=True)
class Issue:
    path: pathlib.Path
    lineno: Optional[int]
    severity: str  # "error", "warning", "info", "fix", "todo"
    message: str


@dataclasses.dataclass(slots=True)
class OpcodeMapping:
    bytes: Tuple[int, ...]
    comment: Optional[str] = None


def split_comment(line: str) -> Tuple[str, str]:
    idx = line.find(";")
    if idx == -1:
        return line, ""
    return line[:idx], line[idx:]


def normalise_hex_literal(match: re.Match[str], path: pathlib.Path, lineno: int, issues: List[Issue]) -> str:
    token = match.group(0)
    if token.upper() in {"AH", "BH", "CH", "DH"}:
        # CPU register names, not literals.
        return token
    if token.upper().startswith("0"):
        # Already compliant.
        return token
    new_token = "0" + token
    issues.append(
        Issue(
            path=path,
            lineno=lineno,
            severity="fix",
            message=f"prefixed hex literal {token!r} -> {new_token!r}",
        )
    )
    return new_token


def format_bytes(byte_values: Sequence[int]) -> str:
    return ", ".join(f"{b:02X}H" for b in byte_values)


def load_opcode_table(config_path: pathlib.Path) -> Dict[str, OpcodeMapping]:
    if not config_path.exists():
        return {}
    data = json.loads(config_path.read_text(encoding="utf-8"))
    table: Dict[str, OpcodeMapping] = {}
    for mnemonic, value in data.items():
        comment: Optional[str] = None
        bytes_spec: Iterable[int | str]
        if isinstance(value, dict):
            if "bytes" not in value:
                raise ValueError(f"opcode table entry for {mnemonic!r} lacks 'bytes'")
            bytes_spec = value["bytes"]
            comment = value.get("comment")
        elif isinstance(value, list):
            bytes_spec = value
        elif isinstance(value, str):
            bytes_spec = [value]
        else:
            raise TypeError(f"unsupported opcode mapping format for {mnemonic!r}")

        byte_values: List[int] = []
        for entry in bytes_spec:
            if isinstance(entry, int):
                byte = entry
            elif isinstance(entry, str):
                stripped = entry.strip().lower()
                if stripped.startswith("0x"):
                    byte = int(stripped, 16)
                else:
                    byte = int(stripped, 16)
            else:
                raise TypeError(f"opcode byte value must be int or hex string, got {entry!r}")
            if not 0 <= byte <= 0xFF:
                raise ValueError(f"opcode byte out of range (0..255): {entry!r}")
            byte_values.append(byte)

        table[mnemonic.upper()] = OpcodeMapping(bytes=tuple(byte_values), comment=comment)
    return table


def process_line(
    line: str,
    path: pathlib.Path,
    lineno: int,
    issues: List[Issue],
    opcode_table: Dict[str, OpcodeMapping],
) -> Tuple[List[str], bool]:
    original_line = line
    code_raw, comment = split_comment(line)
    code = code_raw
    comment_padding = ""
    if comment:
        trimmed = code.rstrip(" \t")
        comment_padding = code[len(trimmed):]
        code = trimmed

    if code.strip() == "" and not comment:
        return [original_line], False
    if code.strip() == "" and comment:
        # Comment-only line.
        return [original_line], False

    # Handle DUP usage (unsupported)
    if re.search(r"\bDUP\b", code, re.IGNORECASE):
        issues.append(
            Issue(
                path=path,
                lineno=lineno,
                severity="error",
                message="DUP construct removed (not supported by Z-1 assembler)",
            )
        )
        # Preserve standalone comment if present.
        if comment:
            indent = code_raw[: len(code_raw) - len(code_raw.lstrip())]
            comment_line = indent + comment.lstrip()
            return ([comment_line], True)
        return ([], True)

    # Replace OUT DX, AH with MOV+OUT sequence
    out_match = OUT_DX_AH_PATTERN.match(code)
    if out_match:
        out_column = out_match.start("out")
        prefix = code[:out_column]
        # Ensure at least one space between label and MOV
        if out_match.group("label") and not prefix.endswith((" ", "\t")):
            prefix += " "
            out_column += 1
        first_line = prefix + "MOV AL, AH"
        second_line = " " * out_column + "OUT DX, AL"
        if comment_padding:
            second_line += comment_padding
        if comment:
            second_line += comment
        issues.append(
            Issue(
                path=path,
                lineno=lineno,
                severity="fix",
                message="rewrote 'OUT DX, AH' into MOV AL,AH + OUT DX,AL",
            )
        )
        return ([first_line.rstrip(), second_line.rstrip()], True)

    # Opcode escape hatch via DB mappings
    inst_match = INSTRUCTION_PATTERN.match(code)
    replaced_by_mapping = False
    if inst_match:
        mnemonic = inst_match.group("mnemonic")
        operands = inst_match.group("rest")
        mnemonic_upper = mnemonic.upper()
        if mnemonic_upper in opcode_table and operands.strip() == "":
            mapping = opcode_table[mnemonic_upper]
            prefix = code[: inst_match.start("mnemonic")]
            converted = prefix + "DB " + format_bytes(mapping.bytes)
            existing_comment = comment[1:].strip() if comment else ""
            auto_comment_bits = [f"{mnemonic_upper} (auto via z1asm_lint)"]
            if mapping.comment:
                auto_comment_bits.append(mapping.comment)
            if existing_comment:
                auto_comment_bits.append(existing_comment)
            final_comment = f" ; {' | '.join(auto_comment_bits)}"
            if comment_padding:
                converted += comment_padding
            converted += final_comment
            issues.append(
                Issue(
                    path=path,
                    lineno=lineno,
                    severity="fix",
                    message=f"replaced {mnemonic_upper} with DB {format_bytes(mapping.bytes)}",
                )
            )
            return ([converted.rstrip()], True)
        # Flag potential TODO cases
        operand_string = operands.strip()
        if mnemonic_upper in SHIFT_WITH_IMMEDIATE and "," in operand_string:
            parts = operand_string.split(",", 1)
            if len(parts) == 2 and IMMEDIATE_PATTERN.match(parts[1].strip()):
                issues.append(
                    Issue(
                        path=path,
                        lineno=lineno,
                        severity="todo",
                        message="shift-by-immediate detected; verify assembler accepts it or expand manually",
                    )
                )
        if mnemonic_upper in TODO_80186_ONLY and mnemonic_upper not in opcode_table:
            issues.append(
                Issue(
                    path=path,
                    lineno=lineno,
                    severity="todo",
                    message=f"{mnemonic_upper} flagged as 80186-only; add mapping to tools/z1asm_opcodes.json if assembler rejects it",
                )
            )
        replaced_by_mapping = mnemonic_upper in opcode_table and operands.strip() == ""

    # Hex literal normalization (after other rewrites to avoid double-processing)
    new_code = HEX_NEEDS_LEADING_ZERO.sub(
        lambda m: normalise_hex_literal(m, path, lineno, issues),
        code,
    )

    changed = new_code != code
    if changed or replaced_by_mapping:
        rebuilt = new_code
        if comment_padding:
            rebuilt += comment_padding
        if comment:
            rebuilt += comment
        return ([rebuilt.rstrip()], True)

    return [original_line], False


def preprocess_text(
    path: pathlib.Path,
    text: str,
    opcode_table: Dict[str, OpcodeMapping],
) -> Tuple[str, List[Issue], bool]:
    lines = text.splitlines()
    processed_lines: List[str] = []
    issues: List[Issue] = []
    modified = False

    for idx, line in enumerate(lines, start=1):
        new_lines, changed = process_line(line, path, idx, issues, opcode_table)
        if changed:
            modified = True
        processed_lines.extend(new_lines)

    # Ensure END at the bottom
    last_non_empty = None
    for i in range(len(processed_lines) - 1, -1, -1):
        if processed_lines[i].strip() == "":
            continue
        last_non_empty = processed_lines[i]
        break
    if last_non_empty is None or not last_non_empty.strip().upper().startswith("END"):
        processed_lines.append("END")
        issues.append(
            Issue(
                path=path,
                lineno=len(processed_lines),
                severity="fix",
                message="appended END directive",
            )
        )
        modified = True

    result = "\n".join(processed_lines)
    if result and not result.endswith("\n"):
        result += "\n"
    return result, issues, modified


def sibling_with_suffix(path: pathlib.Path, append: str) -> pathlib.Path:
    """Return *path* with *append* inserted before the final suffix, if any."""
    suffixes = "".join(path.suffixes)
    name = path.name
    if suffixes:
        base = name[: -len(suffixes)]
        new_name = f"{base}{append}{suffixes}"
    else:
        new_name = f"{name}{append}"
    return path.with_name(new_name)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Lint and preprocess Casio Z-1 assembly sources",
    )
    parser.add_argument("path", type=pathlib.Path, help="Path to .asm/.casm source")
    parser.add_argument(
        "--stdout",
        action="store_true",
        help="Emit transformed source to stdout",
    )
    parser.add_argument(
        "--write",
        action="store_true",
        help="Overwrite the input file with the transformed output",
    )
    parser.add_argument(
        "--output",
        type=pathlib.Path,
        help="Write the transformed output to this path",
    )
    parser.add_argument(
        "--write-sibling",
        action="store_true",
        help="Write transformed output alongside source with '_z1' inserted before the extension",
    )
    parser.add_argument(
        "--opcode-table",
        type=pathlib.Path,
        help="Custom opcode mapping file (defaults to tools/z1asm_opcodes.json)",
    )
    args = parser.parse_args()

    if args.write and args.output:
        parser.error("--write and --output are mutually exclusive")
    if args.write and args.write_sibling:
        parser.error("--write and --write-sibling are mutually exclusive")
    if args.output and args.write_sibling:
        parser.error("--output and --write-sibling are mutually exclusive")

    source_path = args.path.resolve()
    if not source_path.exists():
        print(f"File not found: {source_path}", file=sys.stderr)
        return 2

    try:
        raw_text = source_path.read_text(encoding="ascii")
    except UnicodeDecodeError as exc:
        print(f"{source_path}: non-ASCII input not supported ({exc})", file=sys.stderr)
        return 2

    config_path = args.opcode_table
    if config_path is None:
        config_path = pathlib.Path(__file__).resolve().with_name("z1asm_opcodes.json")

    opcode_table = load_opcode_table(config_path)

    transformed_text, issues, modified = preprocess_text(source_path, raw_text, opcode_table)

    for issue in issues:
        prefix = f"{issue.path}:{issue.lineno}" if issue.lineno is not None else f"{issue.path}:"
        print(f"{prefix}: {issue.severity}: {issue.message}", file=sys.stderr)

    if args.write:
        if modified:
            source_path.write_text(transformed_text, encoding="ascii")
            print(f"{source_path}: updated", file=sys.stderr)
        else:
            print(f"{source_path}: already clean", file=sys.stderr)
    if args.output:
        args.output.write_text(transformed_text, encoding="ascii")
        print(f"wrote transformed output to {args.output}", file=sys.stderr)
    if args.write_sibling:
        sibling_path = sibling_with_suffix(source_path, "_z1")
        sibling_path.write_text(transformed_text, encoding="ascii")
        print(f"wrote transformed output to {sibling_path}", file=sys.stderr)
    if args.stdout:
        sys.stdout.write(transformed_text)

    if not (args.write or args.output or args.stdout):
        if modified:
            print(
                "source requires fixes; rerun with --write or choose an output destination",
                file=sys.stderr,
            )
        else:
            print("no changes required", file=sys.stderr)

    has_errors = any(issue.severity == "error" for issue in issues)
    return 1 if has_errors else 0


if __name__ == "__main__":
    sys.exit(main())
