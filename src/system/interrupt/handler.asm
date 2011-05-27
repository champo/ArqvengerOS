EXTERN  int08, int09, interruptDispatcher

; Defines a macro that takes as an argument the interrupt number.
; Calls that interrupt.
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


; Defines a macro that takes as an argument the interrupt number.
; Uses the CALLER macro to call the corresponding interrupt.
; Pushes a 0 as the error code.
%macro ISR_NOERRCODE 2 
GLOBAL _int%1Handler   
  _int%1Handler:
    cli

    ; Save the error code (errCode).
    push 0         
    ; Save the interrupt number (intNum).
    push %1h      
    CALLER %2
%endmacro

; Idem, but pushing a 1 as the error code.
%macro ISR_ERRCODE 2   
GLOBAL _int%1Handler
  _int%1Handler:
    cli
    
    ; Save the error code (errCode).
     push 1
    ; Save the interrupt number (intNum).
    push %1h       
    CALLER %2
%endmacro

; Definition of the interrupt handlers

ISR_ERRCODE 80, interruptDispatcher

; IRQs Handlers

ISR_ERRCODE 20, interruptDispatcher
ISR_ERRCODE 21, interruptDispatcher

; Definition of exceptions Handlers
ISR_ERRCODE 00, interruptDispatcher
ISR_ERRCODE 01, interruptDispatcher
ISR_ERRCODE 02, interruptDispatcher
ISR_ERRCODE 03, interruptDispatcher
ISR_ERRCODE 04, interruptDispatcher
ISR_ERRCODE 05, interruptDispatcher
ISR_ERRCODE 06, interruptDispatcher
ISR_ERRCODE 07, interruptDispatcher
ISR_ERRCODE 08, interruptDispatcher
ISR_ERRCODE 09, interruptDispatcher
ISR_ERRCODE 0A, interruptDispatcher
ISR_ERRCODE 0B, interruptDispatcher
ISR_ERRCODE 0C, interruptDispatcher
ISR_ERRCODE 0D, interruptDispatcher
ISR_ERRCODE 0E, interruptDispatcher
ISR_ERRCODE 0F, interruptDispatcher
ISR_ERRCODE 10, interruptDispatcher
ISR_ERRCODE 11, interruptDispatcher
ISR_ERRCODE 12, interruptDispatcher
ISR_ERRCODE 13, interruptDispatcher
ISR_ERRCODE 14, interruptDispatcher
ISR_ERRCODE 15, interruptDispatcher
ISR_ERRCODE 16, interruptDispatcher
ISR_ERRCODE 17, interruptDispatcher
ISR_ERRCODE 18, interruptDispatcher
ISR_ERRCODE 19, interruptDispatcher
ISR_ERRCODE 1A, interruptDispatcher
ISR_ERRCODE 1B, interruptDispatcher
ISR_ERRCODE 1C, interruptDispatcher
ISR_ERRCODE 1D, interruptDispatcher
ISR_ERRCODE 1E, interruptDispatcher
ISR_ERRCODE 1F, interruptDispatcher
