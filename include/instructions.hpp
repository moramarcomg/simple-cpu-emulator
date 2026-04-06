#pragma once

#include <cstdint>

// ── Opcodes ──────────────────────────────────────────────────────────────────
// Usamos un enum class para que los opcodes tengan su propio namespace
// y no contaminen el scope global — práctica estándar en C++ moderno.
enum class OpCode : uint8_t {

    // Data movement
    NOP      = 0x00,  // No operation
    MOV_RR   = 0x01,  // MOV Rdst, Rsrc   — copia registro a registro
    MOV_RI   = 0x02,  // MOV Rdst, imm    — carga inmediato en registro
    LOAD     = 0x03,  // LOAD Rdst, addr  — lee memoria a registro
    STORE    = 0x04,  // STORE addr, Rsrc — guarda registro en memoria

    // Arithmetic
    ADD      = 0x10,  // ADD Rdst, Rsrc
    SUB      = 0x11,  // SUB Rdst, Rsrc
    INC      = 0x12,  // INC Rdst
    DEC      = 0x13,  // DEC Rdst

    // Logic
    AND      = 0x20,  // AND Rdst, Rsrc
    OR       = 0x21,  // OR  Rdst, Rsrc
    XOR      = 0x22,  // XOR Rdst, Rsrc
    NOT      = 0x23,  // NOT Rdst

    // Control flow
    JMP      = 0x30,  // JMP addr          — salto incondicional
    JZ       = 0x31,  // JZ  addr          — salta si FLAG_ZERO activo
    JNZ      = 0x32,  // JNZ addr          — salta si FLAG_ZERO NO activo

    HALT     = 0xFF   // Detiene la ejecución
};
