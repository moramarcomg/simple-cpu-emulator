/*
    main.cpp - Etapas 4 + 5: assembler de texto + modo debug

    Uso:
        ./emulator.exe                        -> corre programs/loop.asm
        ./emulator.exe programs/loop.asm      -> corre el .asm indicado
        ./emulator.exe programs/loop.asm --debug  -> modo step-by-step
*/

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include "cpu.hpp"
#include "assembler.hpp"

int main(int argc, char* argv[]) {
    std::string path  = "programs/loop.asm";
    bool debugMode    = false;

    /*
        Parseamos los argumentos manualmente.
        argc es la cantidad de argumentos (incluyendo el nombre del programa).
        argv[0] = nombre del ejecutable
        argv[1] = primer argumento (path del .asm si se pasa)
        argv[2] = segundo argumento (--debug si se pasa)

        Buscamos "--debug" en cualquier posicion para no forzar un orden fijo.
    */
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--debug") debugMode = true;
        else                  path = arg;
    }

    CPU cpu;
    Assembler assembler;

    try {
        std::cout << "=== Ensamblando: " << path << " ===\n";
        auto bytecode = assembler.assemble(path);

        std::cout << "Bytecode generado (" << bytecode.size() << " bytes): ";
        for (auto b : bytecode) {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(b) << " ";
        }
        std::cout << "\n\n";

        cpu.load(bytecode);

        if (debugMode) {
            cpu.runDebug();
        } else {
            cpu.run();
            cpu.dump();
        }

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << "\n";
        return 1;
    }

    return 0;
}