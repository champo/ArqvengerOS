GLOBAL _isIF

_isIF:
    push ebp
    mov ebp, esp

    ; Push the flags, then pop'em and check the IF flag
    pushf
    pop ax
    and eax, 512 

    mov esp, ebp
    pop ebp
