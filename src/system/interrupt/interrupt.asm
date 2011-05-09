GLOBAL  _lidt
GLOBAL  _cli,_sti

SECTION .text

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
