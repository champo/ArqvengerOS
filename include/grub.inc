;****************************************************************
; grub.inc
;
; Constantes utilizadas en el header.
;***************************************************************


MULTIBOOT_PAGE_ALIGN	equ 1<<0	; Alineado a 4KB ( pagina )
MULTIBOOT_MEMORY_INFO	equ 1<<1        
MULTIBOOT_AOUT_KLUDGE	equ 1<<16	; Indica que los offset del 8 al 24 en el header son 
					; validos para extraer información.
					
MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_AOUT_KLUDGE
MULTIBOOT_CHECKSUM	equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)