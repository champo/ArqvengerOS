GLOBAL _isIF

_isIF:
    push ebp
    mov ebp, esp

    pushf
    pop ax
    and eax, 512 

    mov esp, ebp
    pop ebp
