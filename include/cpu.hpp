#pragma once

/*
    cpu.hpp - Definicion de la clase CPU

    Este archivo define como "se ve" nuestra CPU por dentro.
    En C++ separamos la DEFINICION (hpp) de la IMPLEMENTACION (cpp).
    Esto es importante porque:
    - Otros archivos solo necesitan saber QUE hace la CPU, no COMO lo hace
    - Si cambias la implementacion, no necesitas recompilar todo el proyecto
*/

#include <array>     // std::array: arreglo con tamano fijo conocido en compilacion
#include <cstdint>   // uint8_t, uint16_t: enteros de tamano exacto (8 bits, 16 bits)
#include <vector>    // std::vector: arreglo dinamico, usado para cargar programas
#include "instructions.hpp"

/*
    Por que static constexpr y no #define?

    La forma antigua en C era:
        #define MEMORY_SIZE 256

    El problema es que #define es un reemplazo de texto puro hecho por el
    preprocesador antes de compilar. No tiene tipo, no tiene scope, puede
    causar bugs raros. En C++ moderno usamos constexpr:

        static constexpr std::size_t MEMORY_SIZE = 256;

    Ventajas:
    - Tiene tipo (std::size_t), el compilador puede detectar errores
    - Vive dentro del scope donde se declara
    - El compilador la reemplaza en tiempo de compilacion igual que #define,
      sin costo en tiempo de ejecucion

    Por que "static"?
    Porque este archivo se incluye en multiples .cpp. Sin static, cada
    archivo que incluya este header tendria su propia copia de la variable
    y el linker se quejaria de definiciones duplicadas.
    static le dice: "esta definicion es local a este archivo de traduccion".
*/
static constexpr std::size_t MEMORY_SIZE   = 256;
static constexpr std::size_t NUM_REGISTERS = 8;

/*
    Flags del procesador

    Usamos un namespace para agrupar las constantes relacionadas.
    Esto evita colisiones de nombres (si tuvieras otro ZERO en otro lado).

    Los flags son bits individuales dentro de un solo byte (uint8_t).
    Usamos potencias de 2 para que cada flag ocupe exactamente un bit:

        ZERO     = 0x01 = 0000 0001  (bit 0)
        NEGATIVE = 0x02 = 0000 0010  (bit 1)
        OVERFLOW = 0x04 = 0000 0100  (bit 2)

    Para ACTIVAR un flag usamos OR:     flags |= Flags::ZERO
    Para DESACTIVAR usamos AND+NOT:     flags &= ~Flags::ZERO
    Para CONSULTAR usamos AND:          if (flags & Flags::ZERO)

    Este patron se llama "bitmask" y es muy comun en sistemas embebidos
    porque permite guardar multiples estados en un solo byte de memoria.
*/
namespace Flags {
    static constexpr uint8_t ZERO     = 0x01;
    static constexpr uint8_t NEGATIVE = 0x02;
    static constexpr uint8_t OVERFLOW = 0x04;
}

/*
    La clase CPU

    Modela una CPU de 8 bits muy simple. Los valores de 8 bits significan:
    - Los registros guardan numeros de 0 a 255
    - La memoria tiene 256 posiciones (direcciones de 0x00 a 0xFF)
    - El Program Counter puede apuntar a cualquier posicion de memoria

    Por que una clase y no un struct?
    - El struct expone todos sus datos directamente (todo es publico)
    - La clase nos permite esconder el estado interno (private) y
      exponer solo lo necesario (public)
    - Esto se llama ENCAPSULAMIENTO y es uno de los pilares de OOP
*/
class CPU {
public:
    /*
        Constructor y reset

        El constructor se llama automaticamente cuando creas un objeto:
            CPU cpu;   <-- aqui se llama CPU()

        reset() permite volver al estado inicial sin destruir el objeto.
        Util si queres correr multiples programas en la misma CPU.
    */
    CPU();
    void reset();

    /*
        load() - Carga un programa en memoria

        Recibe un vector de bytes y los copia en memory_ desde la posicion 0.
        El programa es simplemente una secuencia de bytes que la CPU
        interpretara como instrucciones cuando empiece a ejecutar.

        Por que const std::vector<uint8_t>& y no solo std::vector<uint8_t>?
        - Sin el &: C++ copiaria todo el vector al llamar la funcion (lento)
        - Con el &: pasamos una referencia (como un puntero), sin copiar
        - El const garantiza que la funcion no va a modificar el vector original
    */
    void load(const std::vector<uint8_t>& program);

    /*
        Ciclo Fetch-Decode-Execute

        Este es el ciclo fundamental de cualquier CPU. En cada ciclo:

        1. FETCH:   Lee el byte en memory[PC] y avanza PC en 1
        2. DECODE:  Interpreta ese byte como una instruccion (opcode)
        3. EXECUTE: Realiza la accion que corresponde a esa instruccion

        fetch() hace solo el paso 1.
        step() hace los tres pasos juntos (un ciclo completo).
        run()  repite step() hasta que la CPU reciba HALT.
    */
    uint8_t fetch();
    void    step();
    void    run();

    /*
        dump() - Imprime el estado completo de la CPU

        Util para debuggear. Muestra todos los registros, flags y
        los primeros bytes de memoria en formato hexadecimal.

        Por que "const" al final?
        Significa que este metodo NO modifica el objeto. El compilador
        verifica esto y te avisa si accidentalmente intentas modificar algo.
        Es una buena practica marcar como const todo metodo que solo lee.
    */
    void dump() const;

    /*
        Getters - Acceso de solo lectura al estado interno

        Como el estado (memory_, registers_, etc.) es privado, necesitamos
        metodos publicos para que codigo externo pueda leerlo sin modificarlo.

        La implementacion va directamente aqui (inline) porque son tan simples
        que no vale la pena ponerlas en el .cpp. El compilador las expande
        en el lugar donde se usan, sin el costo de llamar una funcion.
    */
    bool    halted()        const { return halted_; }
    uint8_t pc()            const { return pc_; }
    uint8_t sp()            const { return sp_; }
    uint8_t flags()         const { return flags_; }
    uint8_t reg(int i)      const { return registers_[i]; }
    uint8_t memAt(int addr) const { return memory_[addr]; }

private:
    /*
        Estado interno de la CPU

        Todo esto es PRIVADO: solo los metodos de esta clase pueden tocarlo.
        El codigo externo no puede hacer cpu.pc_ = 42 directamente.

        Por que el guion bajo al final (_)?
        Es una convencion muy comun en C++ para distinguir variables miembro
        de variables locales. Cuando ves pc_ sabes que es del objeto,
        cuando ves pc sabes que es un parametro o variable local.

        std::array vs arreglo C clasico:
        - uint8_t memory[256]       <- arreglo C, no sabe su tamano, no tiene metodos
        - std::array<uint8_t, 256>  <- sabe su tamano (.size()), tiene .fill(), mas seguro
        El tamano de std::array debe conocerse en tiempo de compilacion (por eso usamos
        la constexpr MEMORY_SIZE y no una variable normal).
    */
    std::array<uint8_t, MEMORY_SIZE>   memory_;
    std::array<uint8_t, NUM_REGISTERS> registers_;

    uint8_t pc_;      // Program Counter: apunta a la proxima instruccion
    uint8_t sp_;      // Stack Pointer: apunta al tope del stack (crece hacia abajo)
    uint8_t flags_;   // Byte de flags: cada bit es un flag (ZERO, NEGATIVE, OVERFLOW)
    bool    halted_;  // true cuando la CPU ejecuta HALT y deja de correr

    /*
        updateFlags() es privada porque es un detalle de implementacion.
        Solo los metodos internos (ADD, SUB, etc.) la necesitan.
        El codigo externo no necesita saber que existe.
    */
    void updateFlags(uint16_t result);
};