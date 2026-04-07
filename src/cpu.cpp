/*
    cpu.cpp - Implementacion de la clase CPU

    Aqui van los cuerpos de todos los metodos declarados en cpu.hpp.
    La separacion hpp/cpp es una convencion de C++:
    - hpp: el "contrato" (que existe y que hace)
    - cpp: el "como" (la implementacion real)
*/

#include <iostream>  // std::cout: para imprimir en terminal
#include <iomanip>   // std::hex, std::setw, std::setfill: para formatear numeros
#include "cpu.hpp"

// =============================================================================
// Constructor y reset
// =============================================================================

/*
    El constructor llama a reset() para no repetir codigo de inicializacion.
    Si en el futuro cambias como se inicializa la CPU, solo tocas reset().
*/
CPU::CPU() {
    reset();
}

void CPU::reset() {
    /*
        .fill(0) inicializa todos los elementos del array a 0.
        Es equivalente a hacer un loop, pero mas claro y seguro.
        No podemos usar memset aqui porque std::array es un objeto C++,
        aunque en la practica .fill() compila igual de eficiente.
    */
    memory_.fill(0);
    registers_.fill(0);

    pc_     = 0;

    /*
        El stack crece hacia abajo en la mayoria de arquitecturas reales
        (x86, ARM, MIPS). Empieza al final de la memoria y va bajando
        cada vez que se agrega algo.

        static_cast<uint8_t>(...) es necesario porque MEMORY_SIZE es
        std::size_t (64 bits en sistemas modernos) y sp_ es uint8_t (8 bits).
        Sin el cast, el compilador generaria una advertencia de que podriamos
        perder informacion al meter un numero grande en una variable chica.
        El cast le dice al compilador "soy consciente de esto, esta bien".
    */
    sp_     = static_cast<uint8_t>(MEMORY_SIZE - 1);

    flags_  = 0;
    halted_ = false;
}

// =============================================================================
// Carga de programa
// =============================================================================

void CPU::load(const std::vector<uint8_t>& program) {
    /*
        Copiamos byte por byte desde el vector al array de memoria.

        La condicion "i < program.size() && i < MEMORY_SIZE" previene
        que un programa mas grande que la memoria cause un buffer overflow
        (escribir fuera de los limites del array, un bug grave en C++).

        std::size_t es el tipo correcto para indices y tamanos en C++.
        Es un entero sin signo cuyo tamano depende del sistema (32 o 64 bits).
        Usar int para indices puede causar advertencias del compilador porque
        int es con signo y .size() devuelve sin signo.
    */
    for (std::size_t i = 0; i < program.size() && i < MEMORY_SIZE; ++i) {
        memory_[i] = program[i];
    }
}

// =============================================================================
// Helpers internos
// =============================================================================

/*
    updateFlags() recibe el resultado como uint16_t (16 bits) aunque nuestra
    CPU trabaja con valores de 8 bits. Por que?

    Porque necesitamos detectar OVERFLOW: si sumas 200 + 100 = 300, ese
    valor no cabe en 8 bits (maximo 255). En un uint8_t el resultado seria
    300 - 256 = 44 (se "enrolla"), perdiendo informacion.

    Con uint16_t podemos ver el valor real (300) y compararlo con 0xFF (255)
    para detectar que hubo overflow ANTES de truncar el resultado a 8 bits.

    Luego con (result & 0xFF) tomamos solo los 8 bits bajos, que es lo que
    realmente quedo en el registro.
*/
void CPU::updateFlags(uint16_t result) {
    flags_ = 0;  // Limpiamos todos los flags antes de recalcular

    /*
        (result & 0xFF) == 0: verifica si los 8 bits bajos son todos cero.
        Usamos & 0xFF porque result es de 16 bits pero el registro guarda
        solo los 8 bits bajos. Un resultado de 256 daria 0 en el registro
        (overflow), y queremos activar ZERO en ese caso.
    */
    if ((result & 0xFF) == 0) flags_ |= Flags::ZERO;

    /*
        result > 0xFF: si el resultado supera 255, no cabia en 8 bits.
        Esto es overflow para numeros sin signo.
    */
    if (result > 0xFF)        flags_ |= Flags::OVERFLOW;

    /*
        result & 0x80: el bit 7 (el mas alto en 8 bits) indica signo
        en representacion de complemento a dos. Si esta encendido,
        el numero se interpreta como negativo.
        0x80 = 1000 0000 en binario.
    */
    if (result & 0x80)        flags_ |= Flags::NEGATIVE;
}

// =============================================================================
// Ciclo Fetch-Decode-Execute
// =============================================================================

/*
    fetch() es la operacion mas simple: lee el byte en la posicion pc_
    y luego avanza pc_ en 1 para la proxima instruccion.

    pc_++ es post-incremento: primero devuelve el valor actual de pc_,
    luego lo incrementa. Entonces memory_[pc_++] es equivalente a:
        uint8_t val = memory_[pc_];
        pc_ = pc_ + 1;
        return val;
*/
uint8_t CPU::fetch() {
    return memory_[pc_++];
}

void CPU::step() {
    if (halted_) return;  // Si ya paro, no hacemos nada

    /*
        Leemos el opcode y lo convertimos al enum OpCode.

        static_cast<OpCode>(...) convierte el uint8_t crudo a nuestro enum.
        Necesitamos el cast explicito porque enum class no se convierte
        automaticamente desde/hacia enteros (eso es justamente su ventaja).
        El compilador nos obliga a ser explicitos sobre la conversion.
    */
    auto opcode = static_cast<OpCode>(fetch());

    /*
        El switch actua como la "unidad de control" de la CPU.
        En hardware real esto es circuiteria. Aqui lo simulamos con codigo.

        Cada case corresponde a una instruccion. Dentro del case hacemos
        fetch() adicionales para leer los operandos (registros, valores, etc.)
        que siguen al opcode en memoria.
    */
    switch (opcode) {

        case OpCode::NOP:
            // No hace nada. El fetch() del opcode ya avanzo el PC.
            break;

        case OpCode::MOV_RR: {
            /*
                Formato en memoria: [0x01] [dst] [src]
                Leemos dos bytes mas: el registro destino y el registro fuente.
                auto deduce el tipo automaticamente (uint8_t en este caso).
            */
            auto dst = fetch();
            auto src = fetch();
            registers_[dst] = registers_[src];
            break;
        }

        case OpCode::MOV_RI: {
            // Formato en memoria: [0x02] [dst] [valor]
            auto dst = fetch();
            auto imm = fetch();  // imm de "immediate" (valor inmediato/literal)
            registers_[dst] = imm;
            break;
        }

        case OpCode::LOAD: {
            // Formato en memoria: [0x03] [dst] [direccion]
            auto dst  = fetch();
            auto addr = fetch();
            registers_[dst] = memory_[addr];
            break;
        }

        case OpCode::STORE: {
            // Formato en memoria: [0x04] [direccion] [src]
            auto addr = fetch();
            auto src  = fetch();
            memory_[addr] = registers_[src];
            break;
        }

        case OpCode::ADD: {
            auto dst = fetch();
            auto src = fetch();
            /*
                Usamos uint16_t para la suma para poder detectar overflow.
                Si sumaramos directamente en uint8_t, el resultado se
                truncaria automaticamente y perderiamos la informacion
                de que hubo overflow.
            */
            uint16_t result = registers_[dst] + registers_[src];
            /*
                static_cast<uint8_t>(result) trunca el resultado a 8 bits.
                Es lo mismo que result % 256, pero mas explicito y eficiente.
                El cast es necesario porque asignar uint16_t a uint8_t
                sin cast genera una advertencia del compilador.
            */
            registers_[dst] = static_cast<uint8_t>(result);
            updateFlags(result);  // Pasamos el resultado completo (16 bits) para detectar overflow
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
            pc_ = addr;  // Simplemente movemos el PC a la nueva direccion
            break;
        }

        case OpCode::JZ: {
            auto addr = fetch();
            /*
                (flags_ & Flags::ZERO) es una operacion AND bit a bit.
                Si el bit ZERO esta encendido en flags_, el resultado es
                distinto de 0 (verdadero). Si esta apagado, el resultado es 0.
                Es la forma estandar de consultar un bit especifico.
            */
            if (flags_ & Flags::ZERO) pc_ = addr;
            break;
        }

        case OpCode::JNZ: {
            auto addr = fetch();
            if (!(flags_ & Flags::ZERO)) pc_ = addr;  // ! invierte la condicion
            break;
        }

        case OpCode::HALT:
            halted_ = true;
            std::cout << "[CPU] HALT -> stopped at PC=0x"
                      << std::hex                           // imprime en hexadecimal
                      << std::setw(2)                       // ancho minimo de 2 caracteres
                      << std::setfill('0')                  // rellena con '0' a la izquierda
                      << static_cast<int>(pc_ - 1)          // convierte a int para imprimir bien
                      << "\n";
            break;

        default:
            /*
                Si llegamos aqui, la CPU encontro un byte que no reconoce
                como instruccion valida. Detenemos la CPU para no ejecutar
                basura o entrar en un estado impredecible.
            */
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
// Debug - dump()
// =============================================================================

/*
    dump() usa manipuladores de std::cout para formatear la salida.

    Los manipuladores cambian el "modo" del stream hasta que se cambie de nuevo.
    Son como configuraciones que afectan todo lo que se imprime despues.

    std::hex       -> imprime numeros en base 16 (hexadecimal)
    std::dec       -> imprime numeros en base 10 (decimal, el default)
    std::setw(n)   -> establece el ancho minimo del proximo valor a imprimir
                      si el numero tiene menos digitos, se rellena
    std::setfill(c) -> define el caracter de relleno (por defecto es espacio)
                       setfill('0') hace que se rellene con ceros: "0a" en vez de " a"

    Ejemplo de como funcionan juntos:
        std::cout << std::hex << std::setw(2) << std::setfill('0') << 10;
        Imprime: "0a"   (10 en hex es 'a', con ancho 2 y relleno '0')

        Sin setw y setfill:
        std::cout << std::hex << 10;
        Imprime: "a"    (sin padding)

    Por que static_cast<int>(valor_uint8)?
        std::cout trata a uint8_t (que internamente es "unsigned char") como
        un CARACTER, no como un numero. Si imprimes un uint8_t con valor 65,
        cout imprime 'A' (el caracter ASCII 65) en lugar de "65" o "41".
        Al convertirlo a int con static_cast<int>(), forzamos a cout a
        tratarlo como un numero y mostrar su valor numerico.
*/
void CPU::dump() const {
    std::cout << "\n===== CPU STATE =====\n";

    std::cout << "Registers:\n";
    for (std::size_t i = 0; i < NUM_REGISTERS; ++i) {
        std::cout << "  R" << i
                  << " = 0x"
                  << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(registers_[i])   // hex con padding
                  << "  ("
                  << std::dec << std::setw(3) << std::setfill(' ')
                  << static_cast<int>(registers_[i])   // decimal con padding
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

    /*
        El operador ternario: condicion ? valor_si_true : valor_si_false
        Es un if/else comprimido en una expresion.

        (flags_ & Flags::ZERO) ? "ZERO" : "    "
        significa: si el flag ZERO esta activo imprime "ZERO", si no imprime "    "
    */

    std::cout << "Memory (first 32 bytes):\n  ";
    for (int i = 0; i < 32; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(memory_[i]) << " ";
        if ((i + 1) % 8 == 0) std::cout << "\n  ";
    }
    std::cout << "\n=====================\n";
}