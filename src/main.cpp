#include <iostream>
#include <vector>
#include "cpu.hpp"
#include "instructions.hpp"

int main() {
    CPU cpu;

    // ── Programa de prueba ────────────────────────────────────────────────────
    // El enum class OpCode hace que el código sea mucho más legible
    // que escribir bytes crudos como 0x02, 0x10, etc.
    //
    // Equivalente en pseudocódigo:
    //   R0 = 10
    //   R1 = 20
    //   R0 = R0 + R1     → 30
    //   R0 = R0 - R1     → 10
    //   R2 = R0          → 10
    //   R2 = R2 - R2     → 0   (activa FLAG_ZERO)
    //   HALT

    using O = OpCode;  // alias local para no repetir "OpCode::" todo el tiempo

    std::vector<uint8_t> program = {
        (uint8_t)O::MOV_RI, 0, 10,   // R0 = 10
        (uint8_t)O::MOV_RI, 1, 20,   // R1 = 20
        (uint8_t)O::ADD,    0, 1,    // R0 = R0 + R1  → 30
        (uint8_t)O::SUB,    0, 1,    // R0 = R0 - R1  → 10
        (uint8_t)O::MOV_RR, 2, 0,    // R2 = R0
        (uint8_t)O::SUB,    2, 2,    // R2 = 0  → FLAG_ZERO!
        (uint8_t)O::HALT
    };

    cpu.load(program);

    std::cout << "=== Running program ===\n";
    cpu.run();
    cpu.dump();

    return 0;
}
