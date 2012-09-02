GLOBAL  getCPUSpeedHandler

getCPUSpeedHandler:
    ;wait until the timer interrupt has been called.
    mov eax, 191    ; Calling the ticks syscall 
    int 80h 
	
    mov  ebx, eax

waitIrq0:
    mov eax, 191    ; Calling the ticks syscall 
    int 80h 
	
    cmp  ebx, eax
	jz   waitIrq0

	; Getting clocks cycles since the processor started
	rdtsc         
	mov  [timeStampCounterLow], eax
	mov  [timeStampCounterHigh], edx
    
    ; Setting number of ticks to wait
	add  ebx, 2

waitForElapsedTicks:
    mov eax, 191    ;Calling the ticks syscall 
    int 80h 
    
    ; Have we reached the number of ticks we previously set? 
    cmp  ebx, eax 
	jnz  waitForElapsedTicks

	rdtsc
    ; Calculating clock cycles elapsed
    sub eax, [timeStampCounterLow] 
   	sbb edx, [timeStampCounterHigh]

   	; microSeconds  =  ( numberOfTicks / totalTicksPerSecond) * 1,000,000
	; so for this: microSeconds = (1/18.2) * 1,000,000 = 54945
	mov ebx, 54945
	div ebx

	; eax contains measured speed in MHz
    ret 

[SECTION .data]
timeStampCounterLow: DB 0, 0, 0, 0
timeStampCounterHigh: DB 0, 0, 0, 0
