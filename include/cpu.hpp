#pragma once

#include <array>
#include <cstdint>
#include <vector>
#include "instructions.hpp"

// ── Constantes de la arquitectura ────────────────────────────────────────────
inline constexpr std::size_t MEMORY_SIZE   = 256;
inline constexpr std::size_t NUM_REGISTERS = 8;

// ── Flags del procesador (bitmask) ───────────────────────────────────────────
namespace Flags {
    inline constexpr uint8_t ZERO     = 0x01;
    inline constexpr uint8_t NEGATIVE = 0x02;
    inline constexpr uint8_t OVERFLOW = 0x04;
}

// ── CPU ───────────────────────────────────────────────────────────────────────
// Modela una CPU de 8 bits con 8 registros, 256 bytes de RAM y un set
// básico de instrucciones.
class CPU {
public:
    // ── Constructor / reset ──────────────────────────────────────────────────
    CPU();              // Inicializa todo a cero / estado limpio
    void reset();       // Permite reiniciar sin recrear el objeto

    // ── Carga de programas ───────────────────────────────────────────────────
    // Carga un vector de bytes en memoria desde la dirección 0
    void load(const std::vector<uint8_t>& program);

    // ── Ciclo Fetch-Decode-Execute ───────────────────────────────────────────
    uint8_t fetch();    // Lee el byte en PC y avanza PC
    void    step();     // Un ciclo completo
    void    run();      // Corre hasta HALT

    // ── Debug ────────────────────────────────────────────────────────────────
    void dump() const;  // Imprime el estado completo de la CPU

    // ── Estado observable (getters) ──────────────────────────────────────────
    bool    halted()        const { return halted_; }
    uint8_t pc()            const { return pc_; }
    uint8_t sp()            const { return sp_; }
    uint8_t flags()         const { return flags_; }
    uint8_t reg(int i)      const { return registers_[i]; }
    uint8_t memAt(int addr) const { return memory_[addr]; }

private:
    // ── Estado interno ───────────────────────────────────────────────────────
    std::array<uint8_t, MEMORY_SIZE>   memory_;      // RAM
    std::array<uint8_t, NUM_REGISTERS> registers_;   // R0–R7
    uint8_t pc_;      // Program Counter
    uint8_t sp_;      // Stack Pointer
    uint8_t flags_;   // Flags de estado
    bool    halted_;  // true cuando se ejecuta HALT

    // ── Helpers internos ─────────────────────────────────────────────────────
    void updateFlags(uint16_t result);  // Recalcula flags según el resultado
};
