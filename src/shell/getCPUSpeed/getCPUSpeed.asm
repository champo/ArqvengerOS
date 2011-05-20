EXTERN getTicksSinceStart
GLOBAL  getCPUSpeedHandler

getCPUSpeedHandler:
	;wait until the timer interrupt has been called.
	call getTicksSinceStart 
	mov  ebx, eax

wait_irq0:
    call getTicksSinceStart
	cmp  ebx, eax
	jz   wait_irq0
	;read time stamp counter
	rdtsc         
	mov  [timeStampCounterLow], eax
	mov  [timeStampCounterHigh], edx
    ; Set time delay value ticks.
	add  ebx, 2            
    ; remember: so far ebx = ~[irq0]-1, so the next tick is
	; two steps ahead of the current ebx ;)

wait_for_elapsed_ticks:
	call getTicksSinceStart
    ; Have we hit the delay?
    cmp  ebx, eax 
	jnz  wait_for_elapsed_ticks
	rdtsc
    ; Calculate TSC
    sub eax, [timeStampCounterLow] 
   	sbb edx, [timeStampCounterLow]
   	; f(total_ticks_per_Second) =  (1 / total_ticks_per_Second) * 1,000,000
   	; This adjusts for MHz.
	; so for this: f(100) = (1/100) * 1,000,000 = 10000
	mov ebx, 10000
	div ebx
	; ax contains measured speed in MHz
	;mov ~[mhz], ax
    ret 

[SECTION .data]
timeStampCounterLow: resb 4 
timeStampCounterHigh: resb 4
