GLOBAL _isIF, getFlags

; Checks whether the Interrupt Flag is set or not.
_isIF:
    push ebp
    mov ebp, esp

    ; Push the flags, then pop'em and check the IF flag
    pushf
    pop ax
    and eax, 512

    mov esp, ebp
    pop ebp

getFlags:
    push ebp
    mov ebp, esp

    ; Push the flags, then pop'em and check the IF flag
    pushfd
    pop eax

    mov esp, ebp
    pop ebp
