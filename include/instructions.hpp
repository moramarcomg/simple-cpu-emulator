#pragma once

/*
    instructions.hpp - Set de instrucciones de la CPU

    Define todos los "comandos" que nuestra CPU puede ejecutar.
    Cada instruccion es un numero de 8 bits llamado "opcode" (operation code).

    Cuando la CPU lee un byte de memoria durante el FETCH, ese byte
    es el opcode que le dice QUE hacer a continuacion.

    Por que enum class y no enum normal o #define?

    Opcion 1 - #define (C clasico, evitar):
        #define OP_NOP 0x00
        #define OP_ADD 0x10
        Problema: son reemplazos de texto sin tipo. El compilador no puede
        avisarte si mezclas opcodes con otros numeros por accidente.

    Opcion 2 - enum normal (C++, mejor pero no ideal):
        enum OpCode { NOP = 0x00, ADD = 0x10 };
        Problema: los valores se "escapan" al scope exterior. Podrias
        escribir solo NOP sin decir OpCode::NOP, lo que puede causar
        colisiones de nombres con otras partes del codigo.

    Opcion 3 - enum class (C++11 en adelante, la mejor):
        enum class OpCode : uint8_t { NOP = 0x00, ADD = 0x10 };
        Ventajas:
        - Los valores viven en su propio namespace: debes escribir OpCode::NOP
        - Tiene tipo explicito (uint8_t), el compilador verifica que los
          valores caben en 8 bits
        - No se convierte automaticamente a int, evitando bugs sutiles

    El ": uint8_t" al final indica que cada valor del enum ocupa exactamente
    1 byte. Sin eso, el compilador elegiria el tamano (generalmente int = 4 bytes).
    Como los opcodes son bytes que viviran en memoria, tiene sentido que
    ocupen exactamente 1 byte.
*/
enum class OpCode : uint8_t {

    // -------------------------------------------------------------------------
    // Instrucciones de movimiento de datos
    // Estas instrucciones mueven datos entre registros y memoria
    // -------------------------------------------------------------------------

    NOP    = 0x00,  // No Operation: no hace nada, consume un ciclo
                    // Util para insertar delays o como placeholder

    MOV_RR = 0x01,  // MOV registro_destino registro_fuente
                    // Copia el valor de un registro a otro
                    // Ejemplo: MOV_RR 0 1  ->  R0 = R1

    MOV_RI = 0x02,  // MOV registro_destino valor_inmediato
                    // Carga un valor literal (constante) en un registro
                    // Ejemplo: MOV_RI 0 42  ->  R0 = 42
                    // "I" de Immediate (inmediato): el valor va directo en el codigo

    LOAD   = 0x03,  // LOAD registro_destino direccion_memoria
                    // Lee un byte de la RAM y lo pone en un registro
                    // Ejemplo: LOAD 0 200  ->  R0 = memory[200]

    STORE  = 0x04,  // STORE direccion_memoria registro_fuente
                    // Escribe un registro en la RAM
                    // Ejemplo: STORE 200 0  ->  memory[200] = R0

    // -------------------------------------------------------------------------
    // Instrucciones aritmeticas
    // Todas actualizan los flags segun el resultado
    // -------------------------------------------------------------------------

    ADD    = 0x10,  // ADD registro_destino registro_fuente
                    // Suma: Rdst = Rdst + Rsrc
                    // Activa OVERFLOW si el resultado supera 255

    SUB    = 0x11,  // SUB registro_destino registro_fuente
                    // Resta: Rdst = Rdst - Rsrc
                    // Activa ZERO si el resultado es 0
                    // Activa NEGATIVE si el resultado es negativo

    INC    = 0x12,  // INC registro
                    // Incrementa en 1: Rdst = Rdst + 1
                    // Equivalente a ADD Rdst, 1 pero en una sola instruccion

    DEC    = 0x13,  // DEC registro
                    // Decrementa en 1: Rdst = Rdst - 1

    // -------------------------------------------------------------------------
    // Instrucciones logicas (operaciones bit a bit)
    // Trabajan sobre los bits individuales de los valores
    // -------------------------------------------------------------------------

    AND    = 0x20,  // AND registro_destino registro_fuente
                    // AND bit a bit: un bit es 1 solo si AMBOS son 1
                    // Ejemplo: 1100 AND 1010 = 1000
                    // Uso comun: apagar bits especificos (mascaras)

    OR     = 0x21,  // OR bit a bit: un bit es 1 si AL MENOS UNO es 1
                    // Ejemplo: 1100 OR 1010 = 1110
                    // Uso comun: encender bits especificos

    XOR    = 0x22,  // XOR bit a bit: un bit es 1 si son DIFERENTES
                    // Ejemplo: 1100 XOR 1010 = 0110
                    // Truco clasico: X XOR X = 0 (poner registro en 0 rapidamente)

    NOT    = 0x23,  // NOT registro: invierte todos los bits
                    // Ejemplo: 0000 1111 -> 1111 0000

    // -------------------------------------------------------------------------
    // Instrucciones de control de flujo
    // Modifican el Program Counter para saltar a otra parte del programa
    // -------------------------------------------------------------------------

    JMP    = 0x30,  // JMP direccion
                    // Salto incondicional: siempre salta a esa direccion
                    // Equivalente al "goto" o un loop infinito en alto nivel

    JZ     = 0x31,  // Jump if Zero: salta SOLO si el flag ZERO esta activo
                    // El flag ZERO se activa cuando una operacion da 0
                    // Uso tipico: if (resultado == 0) { ir a otra parte }

    JNZ    = 0x32,  // Jump if Not Zero: salta SOLO si ZERO NO esta activo
                    // Uso tipico para loops: repetir mientras contador != 0

    HALT   = 0xFF   // Detiene la CPU completamente
                    // 0xFF como convencion: es el ultimo valor posible en 8 bits
                    // Si la CPU encuentra un opcode desconocido, tambien se detiene
};