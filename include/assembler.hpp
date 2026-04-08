#pragma once

/*
    assembler.hpp - Interfaz del assembler de texto

    El assembler convierte un archivo .asm legible por humanos en un vector
    de bytes que la CPU puede ejecutar directamente.

    Flujo completo:
        archivo .asm  ->  [Assembler]  ->  vector<uint8_t>  ->  [CPU]

    El formato del .asm es una instruccion por linea:
        - Los tokens se separan por espacios
        - Los comentarios empiezan con ';' y se ignoran hasta el fin de linea
        - Las lineas vacias se ignoran
        - Las etiquetas terminan con ':' y definen una direccion con nombre

    Ejemplo de archivo .asm valido:
        ; Esto es un comentario
        MOV_RI 0 10     ; R0 = 10
        loop:
            DEC 0       ; R0--
            JNZ loop    ; si R0 != 0, volver a "loop"
        HALT
*/

#include <string>
#include <vector>
#include <cstdint>

class Assembler {
public:
    /*
        assemble() - Punto de entrada principal

        Lee el archivo en la ruta dada, lo parsea y devuelve el bytecode.
        Lanza std::runtime_error si hay errores de sintaxis o archivo no encontrado.

        El proceso interno tiene dos pasadas (two-pass assembler):
            Pasada 1: recorre el archivo, registra la direccion de cada etiqueta
            Pasada 2: genera el bytecode, resuelve referencias a etiquetas

        Por que dos pasadas?
        Porque una etiqueta puede usarse ANTES de ser definida:
            JNZ fin     <- "fin" todavia no existe en este punto
            DEC 0
            fin:
            HALT
        En la primera pasada calculamos donde queda "fin".
        En la segunda ya podemos escribir la direccion correcta.
    */
    std::vector<uint8_t> assemble(const std::string& filepath);

private:
    /*
        stripComment() elimina todo lo que va desde ';' hasta el fin de linea.
    */
    std::string stripComment(const std::string& line);

    /*
        tokenize() parte una linea en palabras separadas por espacios/tabs.
    */
    std::vector<std::string> tokenize(const std::string& line);
};