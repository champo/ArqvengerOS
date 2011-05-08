GLOBAL  _read_msw,_lidt
GLOBAL  _int08Handler,_int09Handler
GLOBAL  _cli,_sti

EXTERN  int08, int09


SECTION .text


_cli:
    cli
    ret

_sti:
    sti
    ret


_read_msw:
    smsw ax
    retn

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

_eoi:
    mov al, 20h
    out 20h, al
    ret

_int08Handler:
    ; Save the current execution context
    push ds
    push es
    pusha

    ; Set up the interrupt handler execution context
    mov ax, 10h
    mov ds, ax
    mov es, ax

    ; Go go int 08h
    call int08

    ; Restore the execution context, tell the PIC we're done and exit
    popa
    pop es
    pop ds

    call _eoi

    iret

_int09Handler:
    ; Save the current execution context
    push ds
    push es
    pusha

    ; Set up the handler execution context
    mov ax, 10h
    mov ds, ax
    mov es, ax

    ; Call the handler
    call int09

    ; Set the context back
    popa
    pop es
    pop ds

    ; Tell the PIC we're done and exit
    call _eoi

    iret
