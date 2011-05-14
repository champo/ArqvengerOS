GLOBAL systemCall
EXTERN int80

SECTION .text


systemCall:
    push ebp 
    mov ebp,esp

    mov eax,[esp+8]
    mov ebx,[esp+12]
    mov ecx,[esp+16]
    mov edx,[esp+20]

    int 80h 
        
    mov esp,ebp
    pop ebp

    ret
