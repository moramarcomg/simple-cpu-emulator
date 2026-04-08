# simple-cpu-emulator

An 8-bit CPU emulator written in modern C++17, built from scratch stage by stage — from raw registers to a working assembler with debug support.

---

## Why this project matters

Most developers use CPUs every day without knowing what happens below the operating system. This project closes that gap.

By building a CPU emulator from scratch you stop treating the processor as a black box. You understand *why* programs have a program counter, *why* flags exist, *why* assembly has jump instructions, and *why* compilers emit the bytecode they do. That mental model makes you a better systems programmer, a better debugger, and a better engineer overall.

This is also the kind of project that cannot be faked. Either the fetch-decode-execute cycle works or it doesn't.

---

## What you learn building this

**Computer architecture fundamentals**
- How a CPU fetches, decodes, and executes instructions one cycle at a time
- How registers, a program counter, and a stack pointer work together
- How status flags (ZERO, NEGATIVE, OVERFLOW) enable conditional branching

**C++17 in a systems context**
- `std::array` vs C arrays, and why size safety matters
- `enum class` with explicit underlying types (`uint8_t`) instead of raw `#define`
- `uint8_t` / `uint16_t` and why using the right integer size prevents bugs
- Bitmask operations (`|=`, `&=`, `~`) for compact flag storage
- Separation of interface (`.hpp`) and implementation (`.cpp`)
- `static constexpr` vs `#define` and when each is appropriate
- `static_cast` and why implicit integer conversions are dangerous

**Assembler design**
- Two-pass assembly: why one pass is not enough to resolve forward labels
- Tokenization and parsing of a simple text format
- Building a label table and resolving symbolic addresses to numeric ones

**Software design**
- Encapsulation: keeping CPU state private and exposing only what's needed
- Separation of concerns: CPU, assembler, and programs are independent layers
- How a debug mode can be added non-invasively without touching core logic

---

## Architecture

| Component | Details |
|---|---|
| Registers | 8 general-purpose, R0–R7, 8-bit each |
| RAM | 256 bytes |
| Program Counter | 8-bit, points to the next instruction |
| Stack Pointer | 8-bit, grows downward from 0xFF |
| Status flags | ZERO, NEGATIVE, OVERFLOW (bitmask in one byte) |

### Instruction set

| Category | Instructions |
|---|---|
| Data movement | `MOV_RR`, `MOV_RI`, `LOAD`, `STORE` |
| Arithmetic | `ADD`, `SUB`, `INC`, `DEC` |
| Logic | `AND`, `OR`, `XOR`, `NOT` |
| Control flow | `JMP`, `JZ`, `JNZ`, `HALT` |
| Misc | `NOP` |

---

## Project structure

```
simple-cpu-emulator/
├── include/
│   ├── cpu.hpp            # CPU class interface
│   ├── instructions.hpp   # OpCode enum (the instruction set)
│   └── assembler.hpp      # Assembler interface
├── src/
│   ├── cpu.cpp            # CPU implementation: fetch-decode-execute, debug
│   ├── assembler.cpp      # Two-pass text assembler
│   └── main.cpp           # Entry point
└── programs/
    └── loop.asm           # Example program: countdown loop
```

---

## Build & run

**Compile:**
```bash
g++ -std=c++17 -Wall -o emulator.exe src/main.cpp src/cpu.cpp src/assembler.cpp -Iinclude
```

**Run the default program** (`programs/loop.asm`):
```bash
./emulator.exe
```

**Run a specific `.asm` file:**
```bash
./emulator.exe programs/loop.asm
```

**Step-by-step debug mode** — pauses before each instruction and prints CPU state:
```bash
./emulator.exe programs/loop.asm --debug
```

In debug mode, press `Enter` to execute the next instruction or `q` to quit.

---

## Writing your own programs

Create a `.asm` file in `programs/`. One instruction per line. Comments start with `;`. Labels end with `:`.

```asm
; example.asm — adds two numbers and stores the result in memory

MOV_RI 0 40       ; R0 = 40
MOV_RI 1 60       ; R1 = 60
ADD    0 1        ; R0 = R0 + R1  →  100
STORE  200 0      ; memory[200] = R0
HALT
```

Then run it:
```bash
./emulator.exe programs/example.asm
```

---

## Stages completed

- [x] Stage 1 — CPU class: registers, memory, flags
- [x] Stage 2 — Fetch-Decode-Execute cycle + full instruction set
- [x] Stage 3 — Jump and branch instructions (`JMP`, `JZ`, `JNZ`)
- [x] Stage 4 — Text assembler: write `.asm` files instead of raw bytecode
- [x] Stage 5 — Debug mode & step-by-step execution
