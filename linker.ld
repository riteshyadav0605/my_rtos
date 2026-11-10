MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
    RAM   (rwx): ORIGIN = 0x20000000, LENGTH = 64K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .data :
    {
        *(.data*)
        . = ALIGN(4);
    } > RAM AT > FLASH

    .bss :
    {
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
    } > RAM
}