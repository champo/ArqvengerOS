GLOBAL  _lidt
GLOBAL  _cli,_sti

SECTION .text

; Definition of "wrapper" functions of assembler instructions, to make possible to call
; them from a C code.

_cli:
    cli
    ret

_sti:
    sti
    ret

_lidt:
    push ebp
    mov ebp, esp
    push ebx
    mov ebx, [ss:ebp + 6] ; ds:bx = puntero a IDTR
    rol ebx, 16
    lidt [ds:ebx] ; carga IDTR
    pop ebx
    pop ebp
    retn
