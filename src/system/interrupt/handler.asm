EXTERN  int08, int09, interruptDispatcher

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


; Defines a macro that takes as an argument the
; interrupt number
%macro ISR_NOERRCODE 2 
GLOBAL _int%1Handler   
  _int%1Handler:
    cli

    ; Save the error code (errCode)
    push 0         
    ; Save the interrupt number (intNum)
    push %1h      
    CALLER %2
%endmacro

; Idem, but without adding the cero error code
%macro ISR_ERRCODE 2   
GLOBAL _int%1Handler
  _int%1Handler:
    cli

    push 0
    ; Save the interrupt number (intNum)
    push %1h       
    CALLER %2
%endmacro


ISR_ERRCODE 80, interruptDispatcher
ISR_ERRCODE 20, interruptDispatcher
ISR_ERRCODE 21, interruptDispatcher
