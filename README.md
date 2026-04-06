# simple-cpu-emulator

An 8-bit CPU emulator written in modern C++17 — built step by step.

## Architecture

- **8 general-purpose registers**: R0–R7 (8-bit each)
- **256 bytes of RAM**
- **Program Counter (PC)** and **Stack Pointer (SP)**
- **Status flags**: ZERO, NEGATIVE, OVERFLOW
- **Instruction set**: MOV, LOAD, STORE, ADD, SUB, INC, DEC, AND, OR, XOR, NOT, JMP, JZ, JNZ, HALT

## Build

```bash
make
./emulator
```

## Roadmap

- [x] Stage 1 — CPU class: registers, memory, flags
- [x] Stage 2 — Fetch-Decode-Execute cycle + instruction set
- [ ] Stage 3 — Jump and branch instructions
- [ ] Stage 4 — Text assembler (write `.asm` files instead of raw bytecode)
- [ ] Stage 5 — Debug mode & step-by-step execution
