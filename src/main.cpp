/*
    main.cpp - Programa de prueba: Etapa 3 (Jump y Branch)

    Demuestra las tres instrucciones de control de flujo:
        JMP  - salto incondicional
        JZ   - salta si FLAG_ZERO esta activo (resultado == 0)
        JNZ  - salta si FLAG_ZERO NO esta activo (resultado != 0)

    Programa que se ejecuta:
        R0 = 3          <- contador
        R1 = 1          <- valor constante para decrementar
        loop:
            DEC R0      <- R0 = R0 - 1  (actualiza flags)
            JNZ loop    <- si R0 != 0, volver al inicio del loop
        R2 = 42         <- solo se ejecuta al salir del loop
        HALT

    Resultado esperado: R0=0, R1=1, R2=42, FLAG_ZERO activo
*/

#include <iostream>
#include <vector>
#include "cpu.hpp"
#include "instructions.hpp"

int main() {
    CPU cpu;

    using O = OpCode;

    /*
        Mapa de memoria del programa:
        Addr  Bytes            Instruccion
        0x00  02 00 03         MOV_RI R0, 3
        0x03  02 01 01         MOV_RI R1, 1
        0x06  13 00            DEC R0          <- inicio del loop (addr=0x06)
        0x08  32 06            JNZ 0x06        <- si R0 != 0, volver a 0x06
        0x0A  02 02 2A         MOV_RI R2, 42   <- solo ejecuta al salir
        0x0D  FF               HALT
    */
    std::vector<uint8_t> program = {
        (uint8_t)O::MOV_RI, 0, 3,      // 0x00: R0 = 3  (contador)
        (uint8_t)O::MOV_RI, 1, 1,      // 0x03: R1 = 1  (no se usa directamente, ilustrativo)
        (uint8_t)O::DEC,    0,         // 0x06: R0--    <- top of loop
        (uint8_t)O::JNZ,    0x06,      // 0x08: si R0 != 0, saltar a 0x06
        (uint8_t)O::MOV_RI, 2, 42,     // 0x0A: R2 = 42 (post-loop)
        (uint8_t)O::HALT               // 0x0D
    };

    cpu.load(program);

    std::cout << "=== Stage 3: Jump & Branch ===\n";
    std::cout << "Programa: loop que decrementa R0 desde 3 hasta 0\n\n";

    cpu.run();
    cpu.dump();

    /*
        Verificacion programatica del resultado esperado.
        Util para detectar regresiones si modificas la CPU en etapas futuras.
    */
    std::cout << "\n=== Verificacion ===\n";
    bool ok = true;

    auto check = [&](const char* label, uint8_t got, uint8_t expected) {
        bool pass = (got == expected);
        std::cout << (pass ? "[OK] " : "[FAIL] ")
                  << label << ": " << (int)got
                  << (pass ? "" : (" (esperado: " + std::to_string(expected) + ")"))
                  << "\n";
        if (!pass) ok = false;
    };

    check("R0 == 0",            cpu.reg(0), 0);
    check("R1 == 1",            cpu.reg(1), 1);
    check("R2 == 42",           cpu.reg(2), 42);
    check("FLAG_ZERO activo",   cpu.flags() & Flags::ZERO, Flags::ZERO);
    check("CPU detenida",       cpu.halted(), true);

    std::cout << (ok ? "\nTodos los checks pasaron.\n" : "\nAlgun check fallo.\n");

    return ok ? 0 : 1;
}