GLOBAL  _int08Handler,_int09Handler
EXTERN  int08, int09

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

    mov al, 20h
    out 20h, al

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
    mov al, 20h
    out 20h, al

    iret
