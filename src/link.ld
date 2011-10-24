ENTRY (_loader)

SECTIONS{
    . = 0x00100000;

    .text :{
        *(.text)
    }

    .bss ALIGN (0x1000) : {
        _sbss = .;
        *(COMMON)
        *(.bss)
        _ebss = .;
    }

    .data ALIGN (0x1000) : {
        *(.data)
    }


    .rodata ALIGN (0x1000) : {
        *(.rodata)
        *(.comment)
    }

}
