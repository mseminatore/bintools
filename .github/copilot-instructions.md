# Copilot Instructions

## Build

```bash
# Build all tools
make

# Build a single tool
make -C ln       # or: asm, cisc, dumpbin, strip

# Build assembler + assemble the sample programs
make -C asm

# Clean
make clean
```

Build uses **C++14** (`-std=c++14`) via Clang on macOS and MSVC on Windows. Each subdirectory produces a standalone binary named after the tool.

## Architecture

The project is a suite of five tools that work together around the [a.out](https://en.wikipedia.org/wiki/A.out) object file format and a custom 8-bit CPU called **cisc**:

```
asm  →  *.o (a.out object files)  →  ln  →  executable a.out
                                            ↓
                              cisc (loads and runs the executable)
                              dumpbin (inspects headers/symbols/relocs)
                              strip (removes symbols and relocations)
```

**Shared core (`aout.h` / `aout.cpp`)** — All tools link against this. It defines:
- `AOUT_HEADER` — the 32-byte file header (`a_magic == 263` identifies a valid file)
- `RelocationEntry` (8 bytes), `SymbolEntity` (12 bytes) — validated by `static_assert`
- `ObjectFile` — the main class for reading, writing, manipulating, and relocating a.out files

**`cpu_cisc.h`** — Defines the custom ISA: opcodes (`OP_*`), memory map (interrupt vectors at `0xFFF8–0xFFFE`, MMIO at `0xFF00`, RAM ends at `0xFE00`), and register conventions. Shared by `asm`, `cisc`, and `ln`.

**`asm/ParserKit/`** — A vendored parser framework (`BaseParser`, `LexicalAnalyzer`, `SymbolTable`) that `AsmParser` extends. The assembler subclasses `BaseParser` and overrides `yyparse()`.

## Key Conventions

- `_CRT_SECURE_NO_WARNINGS` is defined at the top of each `main.cpp` for Windows/MSVC compatibility.
- Hex literals in assembly source use the `$` prefix (e.g. `$FF00`), not `0x`. This is set by `HEX_PREFIX` in `aout.h`.
- Struct sizes are enforced with `static_assert` — do not add fields to `AOUT_HEADER`, `RelocationEntry`, or `SymbolEntity` without updating the corresponding assert and the file I/O code.
- Each tool lives in its own subdirectory with a `main.cpp` and `Makefile`. The subdirectory `Makefile` references `../aout.o` and `../aout.h` directly; there is no install step.
- Symbol type flags (`SET_EXTERN`, `SET_TEXT`, etc.) are bitmask values that can be OR'd together.
- The `asm` `Makefile` has a `code` target that assembles `main.asm`, `rtl.asm`, `io.asm`, `string.asm`, and `os.asm` and links them — this acts as the integration test.

## Git

- Do not add a Copilot co-author trailer (or any `Co-authored-by:` line) to commit messages.
- Do not run `git commit` or `git push` without explicit approval from the user.
