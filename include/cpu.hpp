#pragma once

#include <array>
#include <cstdint>
#include <vector>
#include "instructions.hpp"

static constexpr std::size_t MEMORY_SIZE   = 256;
static constexpr std::size_t NUM_REGISTERS = 8;

namespace Flags {
    static constexpr uint8_t ZERO     = 0x01;
    static constexpr uint8_t NEGATIVE = 0x02;
    static constexpr uint8_t OVERFLOW = 0x04;
}

class CPU {
public:
    CPU();
    void reset();
    void load(const std::vector<uint8_t>& program);

    uint8_t fetch();
    void    step();
    void    run();

    /*
        Modo debug / step-by-step

        setDebug(true) activa el modo debug. Una vez activo:
            - runDebug() ejecuta el programa instruccion por instruccion.
            - Antes de cada instruccion imprime el estado actual de la CPU.
            - Despues de cada instruccion espera que el usuario presione Enter
              para continuar, permitiendo inspeccionar el estado en cada paso.

        Por que separar runDebug() de run()?
            run() esta pensado para ejecucion normal (sin pausas, sin output).
            runDebug() es la version interactiva. Tenerlos separados mantiene
            run() limpio y sin condicionales extra en el loop principal.
    */
    void setDebug(bool enabled) { debugMode_ = enabled; }
    void runDebug();

    void dump() const;

    bool    halted()        const { return halted_; }
    uint8_t pc()            const { return pc_; }
    uint8_t sp()            const { return sp_; }
    uint8_t flags()         const { return flags_; }
    uint8_t reg(int i)      const { return registers_[i]; }
    uint8_t memAt(int addr) const { return memory_[addr]; }

private:
    std::array<uint8_t, MEMORY_SIZE>   memory_;
    std::array<uint8_t, NUM_REGISTERS> registers_;

    uint8_t pc_;
    uint8_t sp_;
    uint8_t flags_;
    bool    halted_;
    bool    debugMode_;   // true = modo step-by-step activo

    void updateFlags(uint16_t result);

    /*
        printStep() imprime una linea resumida del estado actual antes de
        ejecutar la instruccion. Es menos verboso que dump(): solo muestra
        PC, la instruccion que se va a ejecutar y los registros no-cero.

        Por que no reusar dump()?
            dump() imprime todo (32 bytes de memoria, todos los registros).
            En debug paso a paso eso es demasiado ruido. printStep() muestra
            solo lo relevante para seguir el flujo de ejecucion.
    */
    void printStep() const;
};