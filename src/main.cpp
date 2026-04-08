/*
    main.cpp - Etapa 4: carga programas desde archivos .asm
 
    En lugar de hardcodear bytecode en el fuente C++, ahora el programa
    se escribe en un archivo .asm legible y el Assembler lo traduce a bytes.
*/
 
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include "cpu.hpp"
#include "assembler.hpp"
 
int main(int argc, char* argv[]) {
    /*
        Aceptamos el path del .asm como argumento opcional.
        Si no se pasa ninguno, usamos "programs/loop.asm" por defecto.
    */
    std::string path = (argc > 1) ? argv[1] : "programs/loop.asm";
 
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
        cpu.run();
        cpu.dump();
 
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << "\n";
        return 1;
    }
 
    return 0;
}