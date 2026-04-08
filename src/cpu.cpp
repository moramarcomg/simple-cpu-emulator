/*
    cpu.cpp - Implementacion de la clase CPU
*/

#include <iostream>
#include <iomanip>
#include "cpu.hpp"

// =============================================================================
// Constructor y reset
// =============================================================================

CPU::CPU() {
    reset();
}

void CPU::reset() {
    memory_.fill(0);
    registers_.fill(0);
    pc_        = 0;
    sp_        = static_cast<uint8_t>(MEMORY_SIZE - 1);
    flags_     = 0;
    halted_    = false;
    debugMode_ = false;
}

// =============================================================================
// Carga de programa
// =============================================================================

void CPU::load(const std::vector<uint8_t>& program) {
    for (std::size_t i = 0; i < program.size() && i < MEMORY_SIZE; ++i) {
        memory_[i] = program[i];
    }
}

// =============================================================================
// Helpers internos
// =============================================================================

void CPU::updateFlags(uint16_t result) {
    flags_ = 0;
    if ((result & 0xFF) == 0) flags_ |= Flags::ZERO;
    if (result > 0xFF)        flags_ |= Flags::OVERFLOW;
    if (result & 0x80)        flags_ |= Flags::NEGATIVE;
}

/*
    printStep() - Resumen compacto del estado antes de ejecutar una instruccion.

    Formato:
        PC=0x06  opcode=0x13  regs: R0=03 R1=01
                 ^instruccion       ^solo registros con valor != 0

    Mostramos el opcode en la posicion actual (pc_) SIN avanzar el PC.
    El fetch() real lo hace step() justo despues.
*/
void CPU::printStep() const {
    std::cout << "PC=0x"
              << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(pc_)
              << "  opcode=0x"
              << std::setw(2) << std::setfill('0')
              << static_cast<int>(memory_[pc_])
              << "  regs:";

    bool anyNonZero = false;
    for (std::size_t i = 0; i < NUM_REGISTERS; ++i) {
        if (registers_[i] != 0) {
            std::cout << " R" << std::dec << i
                      << "=" << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(registers_[i]);
            anyNonZero = true;
        }
    }
    if (!anyNonZero) std::cout << " (todos en 0)";

    // Flags activos
    std::cout << "  flags:[";
    if (flags_ & Flags::ZERO)     std::cout << "Z";
    if (flags_ & Flags::NEGATIVE) std::cout << "N";
    if (flags_ & Flags::OVERFLOW) std::cout << "O";
    std::cout << "]\n";
}

// =============================================================================
// Ciclo Fetch-Decode-Execute
// =============================================================================

uint8_t CPU::fetch() {
    return memory_[pc_++];
}

void CPU::step() {
    if (halted_) return;

    auto opcode = static_cast<OpCode>(fetch());

    switch (opcode) {

        case OpCode::NOP:
            break;

        case OpCode::MOV_RR: {
            auto dst = fetch();
            auto src = fetch();
            registers_[dst] = registers_[src];
            break;
        }

        case OpCode::MOV_RI: {
            auto dst = fetch();
            auto imm = fetch();
            registers_[dst] = imm;
            break;
        }

        case OpCode::LOAD: {
            auto dst  = fetch();
            auto addr = fetch();
            registers_[dst] = memory_[addr];
            break;
        }

        case OpCode::STORE: {
            auto addr = fetch();
            auto src  = fetch();
            memory_[addr] = registers_[src];
            break;
        }

        case OpCode::ADD: {
            auto dst = fetch();
            auto src = fetch();
            uint16_t result = registers_[dst] + registers_[src];
            registers_[dst] = static_cast<uint8_t>(result);
            updateFlags(result);
            break;
        }

        case OpCode::SUB: {
            auto dst = fetch();
            auto src = fetch();
            uint16_t result = registers_[dst] - registers_[src];
            registers_[dst] = static_cast<uint8_t>(result);
            updateFlags(result);
            break;
        }

        case OpCode::INC: {
            auto dst = fetch();
            uint16_t result = registers_[dst] + 1;
            registers_[dst] = static_cast<uint8_t>(result);
            updateFlags(result);
            break;
        }

        case OpCode::DEC: {
            auto dst = fetch();
            uint16_t result = registers_[dst] - 1;
            registers_[dst] = static_cast<uint8_t>(result);
            updateFlags(result);
            break;
        }

        case OpCode::AND: {
            auto dst = fetch();
            auto src = fetch();
            registers_[dst] &= registers_[src];
            updateFlags(registers_[dst]);
            break;
        }

        case OpCode::OR: {
            auto dst = fetch();
            auto src = fetch();
            registers_[dst] |= registers_[src];
            updateFlags(registers_[dst]);
            break;
        }

        case OpCode::XOR: {
            auto dst = fetch();
            auto src = fetch();
            registers_[dst] ^= registers_[src];
            updateFlags(registers_[dst]);
            break;
        }

        case OpCode::NOT: {
            auto dst = fetch();
            registers_[dst] = ~registers_[dst];
            updateFlags(registers_[dst]);
            break;
        }

        case OpCode::JMP: {
            auto addr = fetch();
            pc_ = addr;
            break;
        }

        case OpCode::JZ: {
            auto addr = fetch();
            if (flags_ & Flags::ZERO) pc_ = addr;
            break;
        }

        case OpCode::JNZ: {
            auto addr = fetch();
            if (!(flags_ & Flags::ZERO)) pc_ = addr;
            break;
        }

        case OpCode::HALT:
            halted_ = true;
            std::cout << "[CPU] HALT -> stopped at PC=0x"
                      << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(pc_ - 1) << "\n";
            break;

        default:
            std::cout << "[CPU] ERROR: unknown opcode 0x"
                      << std::hex << static_cast<int>(static_cast<uint8_t>(opcode))
                      << " at PC=0x" << static_cast<int>(pc_ - 1) << "\n";
            halted_ = true;
            break;
    }
}

void CPU::run() {
    while (!halted_) step();
}

// =============================================================================
// Modo debug / step-by-step
// =============================================================================

/*
    runDebug() ejecuta el programa instruccion por instruccion.

    En cada ciclo:
        1. Imprime el estado actual (PC, opcode que se va a ejecutar, registros)
        2. Espera que el usuario presione Enter
        3. Ejecuta la instruccion con step()

    std::cin.ignore() descarta el '\n' que queda en el buffer despues de
    que el usuario presiona Enter. Sin esto, la proxima iteracion leeria
    el '\n' directamente y no esperaria al usuario.
*/
void CPU::runDebug() {
    std::cout << "[DEBUG] Presiona Enter para ejecutar cada instruccion. (q + Enter para salir)\n\n";

    while (!halted_) {
        printStep();

        std::string input;
        std::getline(std::cin, input);

        if (input == "q" || input == "Q") {
            std::cout << "[DEBUG] Salida manual.\n";
            break;
        }

        step();
    }

    // Al terminar, dump completo del estado final
    if (halted_) dump();
}

// =============================================================================
// Debug - dump()
// =============================================================================

void CPU::dump() const {
    std::cout << "\n===== CPU STATE =====\n";

    std::cout << "Registers:\n";
    for (std::size_t i = 0; i < NUM_REGISTERS; ++i) {
        std::cout << "  R" << i
                  << " = 0x"
                  << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(registers_[i])
                  << "  ("
                  << std::dec << std::setw(3) << std::setfill(' ')
                  << static_cast<int>(registers_[i])
                  << ")\n";
    }

    std::cout << "PC = 0x"
              << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(pc_)
              << "  SP = 0x"
              << std::setw(2) << std::setfill('0') << static_cast<int>(sp_)
              << "\n";

    std::cout << "Flags: ["
              << ((flags_ & Flags::ZERO)     ? "ZERO"     : "    ") << "] ["
              << ((flags_ & Flags::NEGATIVE) ? "NEGATIVE" : "        ") << "] ["
              << ((flags_ & Flags::OVERFLOW) ? "OVERFLOW" : "        ") << "]\n";

    std::cout << "Memory (first 32 bytes):\n  ";
    for (int i = 0; i < 32; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(memory_[i]) << " ";
        if ((i + 1) % 8 == 0) std::cout << "\n  ";
    }
    std::cout << "\n=====================\n";
}