#include <iostream>
#include <iomanip>
#include "cpu.hpp"

// ── Constructor ───────────────────────────────────────────────────────────────
CPU::CPU() {
    reset();
}

void CPU::reset() {
    memory_.fill(0);
    registers_.fill(0);
    pc_     = 0;
    sp_     = static_cast<uint8_t>(MEMORY_SIZE - 1);  // Stack desde el final
    flags_  = 0;
    halted_ = false;
}

// ── Carga de programa ─────────────────────────────────────────────────────────
void CPU::load(const std::vector<uint8_t>& program) {
    for (std::size_t i = 0; i < program.size() && i < MEMORY_SIZE; ++i) {
        memory_[i] = program[i];
    }
}

// ── FETCH ─────────────────────────────────────────────────────────────────────
// Lee el byte en memory[PC] y avanza el PC.
uint8_t CPU::fetch() {
    return memory_[pc_++];
}

// ── HELPERS ───────────────────────────────────────────────────────────────────
void CPU::updateFlags(uint16_t result) {
    flags_ = 0;
    if ((result & 0xFF) == 0)  flags_ |= Flags::ZERO;
    if (result > 0xFF)         flags_ |= Flags::OVERFLOW;
    if (result & 0x80)         flags_ |= Flags::NEGATIVE;  // bit 7 activado
}

// ── FETCH-DECODE-EXECUTE ──────────────────────────────────────────────────────
void CPU::step() {
    if (halted_) return;

    // Fetch: leemos el opcode y lo casteamos al enum
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
            auto dst = fetch(), src = fetch();
            registers_[dst] &= registers_[src];
            updateFlags(registers_[dst]);
            break;
        }

        case OpCode::OR: {
            auto dst = fetch(), src = fetch();
            registers_[dst] |= registers_[src];
            updateFlags(registers_[dst]);
            break;
        }

        case OpCode::XOR: {
            auto dst = fetch(), src = fetch();
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
            std::cout << "[CPU] HALT — stopped at PC=0x"
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

// ── RUN ───────────────────────────────────────────────────────────────────────
void CPU::run() {
    while (!halted_) step();
}

// ── DUMP ──────────────────────────────────────────────────────────────────────
void CPU::dump() const {
    using namespace std;
    cout << "\n===== CPU STATE =====\n";

    cout << "Registers:\n";
    for (std::size_t i = 0; i < NUM_REGISTERS; ++i) {
        cout << "  R" << i
             << " = 0x" << hex << setw(2) << setfill('0') << static_cast<int>(registers_[i])
             << "  (" << dec << setw(3) << static_cast<int>(registers_[i]) << ")\n";
    }

    cout << "PC = 0x" << hex << setw(2) << setfill('0') << static_cast<int>(pc_)
         << "  SP = 0x" << setw(2) << setfill('0') << static_cast<int>(sp_) << "\n";

    cout << "Flags: ["
         << ((flags_ & Flags::ZERO)     ? "ZERO"     : "    ") << "] ["
         << ((flags_ & Flags::NEGATIVE) ? "NEGATIVE" : "        ") << "] ["
         << ((flags_ & Flags::OVERFLOW) ? "OVERFLOW" : "        ") << "]\n";

    cout << "Memory (first 32 bytes):\n  ";
    for (int i = 0; i < 32; ++i) {
        cout << hex << setw(2) << setfill('0') << static_cast<int>(memory_[i]) << " ";
        if ((i + 1) % 8 == 0) cout << "\n  ";
    }
    cout << "\n=====================\n";
}
