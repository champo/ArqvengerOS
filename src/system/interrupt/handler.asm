GLOBAL  _int08Handler,_int09Handler, 
EXTERN  int08, int09, int80, interruptDispatcher

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

    ; Tell the PIC we're done and exit
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

%macro CALLER 1
    ; Save the current execution context
    pusha   
    push ds    
    push es
    push fs
    push gs

    ; Set up the handler execution context
    mov ax, 0x10    
    mov ds, ax 
    mov es, ax
    mov fs, ax
    mov gs, ax

    call %1

    pop gs
    pop fs
    pop es
    pop ds 
    popa

    ; Move the sp to where it when the interrupt was triggered
    add esp, 8  

    sti
    iret
%endmacro


%macro ISR_NOERRCODE 2  ; Defines a macro that takes as an argument the
GLOBAL _int%1Handler    ; interrupt number
  _int%1Handler:
    cli
    push byte 0
    push byte %1h
    CALLER %2
%endmacro

%macro ISR_ERRCODE 2    ; Idem, but without adding the cero error code
GLOBAL _int%1Handler
  _int%1Handler:
    cli
    push byte %1h
    CALLER %2
%endmacro


ISR_ERRCODE 80, interruptDispatcher
