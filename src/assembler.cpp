/*
    assembler.cpp - Implementacion del assembler de texto

    Convierte archivos .asm en bytecode ejecutable por la CPU.
    Usa un algoritmo de dos pasadas para resolver etiquetas forward.
*/

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <cctype>
#include "assembler.hpp"
#include "instructions.hpp"

// =============================================================================
// Funciones libres estaticas (internas a este .cpp)
// =============================================================================

/*
    static aqui significa "visible solo dentro de este archivo de traduccion".
    No tienen nada que ver con la clase Assembler; son utilidades puras
    que no necesitan acceder a ningun estado de objeto.
*/

/*
    parseImm() convierte un string a uint8_t.
    Acepta decimal ("42") y hexadecimal ("0x2A").
    Lanza std::runtime_error si el string no es un numero valido.
*/
static uint8_t parseImm(const std::string& token, int lineNum) {
    try {
        /*
            std::stoul con base 0 detecta el formato automaticamente:
                "0x2A" -> hexadecimal
                "42"   -> decimal
        */
        unsigned long val = std::stoul(token, nullptr, 0);
        if (val > 255) {
            throw std::runtime_error("linea " + std::to_string(lineNum) +
                ": valor '" + token + "' fuera de rango (0-255)");
        }
        return static_cast<uint8_t>(val);
    } catch (const std::invalid_argument&) {
        throw std::runtime_error("linea " + std::to_string(lineNum) +
            ": se esperaba un numero, se encontro '" + token + "'");
    }
}

/*
    instrSize() devuelve cuantos bytes ocupa una instruccion en memoria.
    Necesario en la primera pasada para calcular las direcciones de etiquetas.
*/
static int instrSize(const std::string& mn) {
    if (mn == "NOP" || mn == "HALT")                              return 1;
    if (mn == "INC" || mn == "DEC" || mn == "NOT" ||
        mn == "JMP" || mn == "JZ"  || mn == "JNZ")               return 2;
    if (mn == "MOV_RR" || mn == "MOV_RI" || mn == "LOAD"  ||
        mn == "STORE"  || mn == "ADD"    || mn == "SUB"   ||
        mn == "AND"    || mn == "OR"     || mn == "XOR")          return 3;
    return 0;  // desconocido; el error se reporta en pasada 2
}

/*
    resolveOperand() resuelve un operando que puede ser numero o etiqueta.
*/
static uint8_t resolveOperand(const std::string& token, int lineNum,
                               const std::unordered_map<std::string, uint8_t>& labelMap) {
    if (std::isdigit((unsigned char)token[0])) {
        return parseImm(token, lineNum);
    }
    auto it = labelMap.find(token);
    if (it == labelMap.end()) {
        throw std::runtime_error("linea " + std::to_string(lineNum) +
            ": etiqueta no definida '" + token + "'");
    }
    return it->second;
}

// =============================================================================
// Pasada 1: construir tabla de etiquetas
// =============================================================================

/*
    Recorre las lineas sin emitir bytecode.
    Registra en labelMap la direccion de memoria de cada etiqueta.
*/
static std::unordered_map<std::string, uint8_t>
buildLabelMap(const std::vector<std::string>& lines) {
    std::unordered_map<std::string, uint8_t> labelMap;
    uint8_t addr = 0;

    for (int i = 0; i < (int)lines.size(); ++i) {
        std::istringstream ss(lines[i]);
        std::string token;
        if (!(ss >> token)) continue;  // linea vacia

        if (token.back() == ':') {
            // Etiqueta: registrar direccion actual, no genera bytes
            labelMap[token.substr(0, token.size() - 1)] = addr;
            continue;
        }

        int size = instrSize(token);
        if (size == 0) {
            throw std::runtime_error("linea " + std::to_string(i + 1) +
                ": mnemonic desconocido '" + token + "'");
        }
        addr += static_cast<uint8_t>(size);
    }

    return labelMap;
}

// =============================================================================
// Macros de emision de bytes
// =============================================================================

#define EMIT1(op)        out.push_back(static_cast<uint8_t>(OpCode::op))
#define EMIT2(op, a)     EMIT1(op); out.push_back(a)
#define EMIT3(op, a, b)  EMIT2(op, a); out.push_back(b)

// =============================================================================
// Miembros de Assembler
// =============================================================================

std::string Assembler::stripComment(const std::string& line) {
    auto pos = line.find(';');
    if (pos == std::string::npos) return line;
    return line.substr(0, pos);
}

std::vector<std::string> Assembler::tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream ss(line);
    std::string token;
    while (ss >> token) tokens.push_back(token);
    return tokens;
}

std::vector<uint8_t> Assembler::assemble(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("no se pudo abrir el archivo: " + filepath);
    }

    // Leer todas las lineas limpiando comentarios
    std::vector<std::string> lines;
    std::string raw;
    while (std::getline(file, raw)) {
        lines.push_back(stripComment(raw));
    }

    // --- Pasada 1 ---
    auto labelMap = buildLabelMap(lines);

    // --- Pasada 2 ---
    std::vector<uint8_t> out;
    out.reserve(256);

    for (int i = 0; i < (int)lines.size(); ++i) {
        auto tokens = tokenize(lines[i]);
        if (tokens.empty()) continue;

        const std::string& mn = tokens[0];
        int lineNum = i + 1;

        if (mn.back() == ':') continue;  // etiqueta, ya procesada

        auto need = [&](int n) {
            if ((int)tokens.size() < n + 1)
                throw std::runtime_error("linea " + std::to_string(lineNum) +
                    ": '" + mn + "' requiere " + std::to_string(n) + " operando(s)");
        };

        auto reg = [&](int idx) -> uint8_t {
            uint8_t r = parseImm(tokens[idx], lineNum);
            if (r >= 8) throw std::runtime_error("linea " + std::to_string(lineNum) +
                ": registro R" + std::to_string(r) + " no existe (R0-R7)");
            return r;
        };

        auto imm = [&](int idx) -> uint8_t {
            return resolveOperand(tokens[idx], lineNum, labelMap);
        };

        // ----- Datos -----
        if      (mn == "NOP")    { EMIT1(NOP); }
        else if (mn == "MOV_RR") { need(2); EMIT3(MOV_RR, reg(1), reg(2)); }
        else if (mn == "MOV_RI") { need(2); EMIT3(MOV_RI, reg(1), imm(2)); }
        else if (mn == "LOAD")   { need(2); EMIT3(LOAD,   reg(1), imm(2)); }
        else if (mn == "STORE")  { need(2); EMIT3(STORE,  imm(1), reg(2)); }
        // ----- Aritmetica -----
        else if (mn == "ADD")    { need(2); EMIT3(ADD, reg(1), reg(2)); }
        else if (mn == "SUB")    { need(2); EMIT3(SUB, reg(1), reg(2)); }
        else if (mn == "INC")    { need(1); EMIT2(INC, reg(1)); }
        else if (mn == "DEC")    { need(1); EMIT2(DEC, reg(1)); }
        // ----- Logica -----
        else if (mn == "AND")    { need(2); EMIT3(AND, reg(1), reg(2)); }
        else if (mn == "OR")     { need(2); EMIT3(OR,  reg(1), reg(2)); }
        else if (mn == "XOR")    { need(2); EMIT3(XOR, reg(1), reg(2)); }
        else if (mn == "NOT")    { need(1); EMIT2(NOT, reg(1)); }
        // ----- Control de flujo -----
        else if (mn == "JMP")    { need(1); EMIT2(JMP, imm(1)); }
        else if (mn == "JZ")     { need(1); EMIT2(JZ,  imm(1)); }
        else if (mn == "JNZ")    { need(1); EMIT2(JNZ, imm(1)); }
        else if (mn == "HALT")   { EMIT1(HALT); }
        else {
            throw std::runtime_error("linea " + std::to_string(lineNum) +
                ": mnemonic desconocido '" + mn + "'");
        }
    }

    return out;
}