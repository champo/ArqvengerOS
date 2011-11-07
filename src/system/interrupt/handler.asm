EXTERN  interruptDispatcher, mm_set_kernel_context, mm_set_process_context, mm_pagination_set_kernel_directory, mm_pagination_set_process_directory
GLOBAL _interruptEnd

; Defines a macro that takes as an argument the interrupt number.
; Calls that interrupt.
%macro CALLER 2
    ; Save the current execution context
    pushad
    push gs
    push fs
    push es
    push ds

    ; Set up the handler execution context
    mov ax, 0x10    
    mov ds, ax 
    mov es, ax
    mov fs, ax
    mov gs, ax

    call mm_pagination_set_kernel_directory
    call mm_set_kernel_context

    call %2

    call mm_set_process_context
    call mm_pagination_set_process_directory
    
    pop ds
    pop es
    pop fs
    pop gs
    popad

    ; Move the sp to where it when the interrupt was triggered
    add esp, 8

    iretd
%endmacro

_interruptEnd:

    call mm_set_process_context
    call mm_pagination_set_process_directory

    pop ds
    pop es
    pop fs
    pop gs

    iretd


; Defines a macro that takes as an argument the interrupt number.
; Uses the CALLER macro to call the corresponding interrupt.
%macro ISR 2 
GLOBAL _int%1Handler   
  _int%1Handler:
    cli

    ; Save the interrupt number (intNum).
    push 0
    push %1h      
    CALLER %1, %2
%endmacro

%macro ERR_ISR 2 
GLOBAL _int%1Handler   
  _int%1Handler:

    ; Save the interrupt number (intNum).
    push %1h      
    CALLER %1,%2
%endmacro

; Definition of the interrupt handlers

ISR 80, interruptDispatcher

; IRQs Handlers

ISR 20, interruptDispatcher
ISR 21, interruptDispatcher

; Definition of exceptions Handlers
ERR_ISR 00, interruptDispatcher
ERR_ISR 01, interruptDispatcher
ERR_ISR 02, interruptDispatcher
ERR_ISR 03, interruptDispatcher
ERR_ISR 04, interruptDispatcher
ERR_ISR 05, interruptDispatcher
ERR_ISR 06, interruptDispatcher
ERR_ISR 07, interruptDispatcher
ERR_ISR 08, interruptDispatcher
ERR_ISR 09, interruptDispatcher
ERR_ISR 0A, interruptDispatcher
ERR_ISR 0B, interruptDispatcher
ERR_ISR 0C, interruptDispatcher
ERR_ISR 0D, interruptDispatcher
ERR_ISR 0E, interruptDispatcher
ERR_ISR 0F, interruptDispatcher
ERR_ISR 10, interruptDispatcher
ERR_ISR 11, interruptDispatcher
ERR_ISR 12, interruptDispatcher
ERR_ISR 13, interruptDispatcher
ERR_ISR 14, interruptDispatcher
ERR_ISR 15, interruptDispatcher
ERR_ISR 16, interruptDispatcher
ERR_ISR 17, interruptDispatcher
ERR_ISR 18, interruptDispatcher
ERR_ISR 19, interruptDispatcher
ERR_ISR 1A, interruptDispatcher
ERR_ISR 1B, interruptDispatcher
ERR_ISR 1C, interruptDispatcher
ERR_ISR 1D, interruptDispatcher
ERR_ISR 1E, interruptDispatcher
ERR_ISR 1F, interruptDispatcher
