/*
 * Linker script for STM32H723VGTx (1 MB flash) on the WeAct H723 core board.
 *
 * RAM use (default option bytes: 64 KB ITCM, 320 KB AXI SRAM):
 *   DTCM   128 KB  0x20000000  stack, heap, .data, .bss (fastest, CPU only)
 *   AXI    320 KB  0x24000000  camera frame + model tensor arenas (.axi_sram)
 *   SRAM1/2 32 KB  0x30000000  spare DMA-capable RAM (.sram_d2)
 *   SRAM4   16 KB  0x38000000  spare (.sram_d3)
 * DMA1/2 (used by DCMI) cannot reach DTCM, so DMA buffers go in AXI or D2.
 */
ENTRY(Reset_Handler)

_estack = ORIGIN(DTCMRAM) + LENGTH(DTCMRAM);
_Min_Heap_Size  = 0x2000;
_Min_Stack_Size = 0x4000;

MEMORY
{
  ITCMRAM (xrw) : ORIGIN = 0x00000000, LENGTH = 64K
  DTCMRAM (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
  FLASH   (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
  RAM_D1  (xrw) : ORIGIN = 0x24000000, LENGTH = 320K
  RAM_D2  (xrw) : ORIGIN = 0x30000000, LENGTH = 32K
  RAM_D3  (xrw) : ORIGIN = 0x38000000, LENGTH = 16K
}

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)
    KEEP (*(.init))
    KEEP (*(.fini))
    . = ALIGN(4);
    _etext = .;
  } >FLASH

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  .ARM.extab (READONLY) : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM (READONLY) :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array (READONLY) :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array (READONLY) :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array (READONLY) :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    _sdata = .;
    *(.data)
    *(.data*)
    *(.RamFunc)
    *(.RamFunc*)
    . = ALIGN(4);
    _edata = .;
  } >DTCMRAM AT> FLASH

  .bss :
  {
    . = ALIGN(4);
    _sbss = .;
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    __bss_end__ = _ebss;
  } >DTCMRAM

  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >DTCMRAM

  /* Large uninitialised buffers: not zeroed at boot, not stored in flash. */
  .axi_sram (NOLOAD) :
  {
    . = ALIGN(32);
    *(.axi_sram)
    *(.axi_sram*)
    . = ALIGN(32);
  } >RAM_D1

  .sram_d2 (NOLOAD) :
  {
    . = ALIGN(32);
    *(.sram_d2)
    *(.sram_d2*)
  } >RAM_D2

  .sram_d3 (NOLOAD) :
  {
    . = ALIGN(32);
    *(.sram_d3)
    *(.sram_d3*)
  } >RAM_D3

  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
