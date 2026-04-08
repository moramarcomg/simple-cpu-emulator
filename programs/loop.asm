; loop.asm - Programa de prueba para el assembler
;
; Mismo programa de la etapa 3, ahora escrito en assembly legible
; en vez de bytecode hardcodeado en main.cpp.
;
; Pseudocodigo:
;   R0 = 3          ; contador
;   loop:
;       R0 = R0 - 1
;       si R0 != 0: ir a loop
;   R2 = 42         ; post-loop
;   HALT

MOV_RI 0 3          ; R0 = 3 (contador)
MOV_RI 1 1          ; R1 = 1 (ilustrativo)

loop:
    DEC 0           ; R0--
    JNZ loop        ; si R0 != 0, volver

MOV_RI 2 42         ; R2 = 42 (solo se ejecuta al salir del loop)
HALT
