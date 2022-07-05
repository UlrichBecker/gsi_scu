/*!
 * @defgroup GENERATED_SOURCE
 * @{
 * @file time_measurement.ld
 * @brief lm32-Linkerscript for target: time_measurement
 * @note This file has been created automatically, do not modify it by hand!
 * @date Di 5. Jul 11:56:28 CEST 2022
 * @author makefile.scu
 */

OUTPUT_FORMAT("elf32-lm32")
GROUP( -lc -lgcc )
ENTRY(_reset_handler)

MEMORY
{
   ram(rwx):
      ORIGIN = 0x10000000,
      LENGTH = 147456 - 10240

   stack(rw):
      ORIGIN = 0x10000000 + 147456 - 10240,
      LENGTH = 10240
} /* End of MEMORY */

SECTIONS
{
   . = ORIGIN(ram);

   /* First location in stack is highest address in RAM */
   PROVIDE(_fstack = ORIGIN(stack) + LENGTH(stack) - 4);
   PROVIDE(_endram = ORIGIN(stack));

   .boot :
   {
      _fboot = .;
      *(.boot);
      _eboot = .;
   } > ram

   /* Begin of build-id array defined in "./generated/time_measurement_buildid.c" */
   .buildid ADDR(.boot) + 0x100 :
   {/*
     * The build-id array has to be preserved in any cases,
     * doesn't matter whether the compiler option -fdata-sections and
     * linker option --gc-sections are set or not.
     */
      KEEP(*(.buildid .buildid.*))
      _fbuildid = .;
      *(.buildid .buildid.*) _ebuildid = .;
   } > ram

   /* Begin of shared memory */
   .shared ADDR(.buildid) + 0x400 :
   {
      _fshared = .;
      PROVIDE(_startshared = .);
      *(.shared .shared.*) _eshared = .;
   } > ram

   /* Begin of code */
   .text  ADDR(.shared) + 0 :
   {
     . = ALIGN(4);
     _ftext = .;
     _ftext_rom = LOADADDR(.text);
     *(.text .stub .text.* .gnu.linkonce.t.*)
     *(.gnu.warning)
     KEEP(*(.init))
     KEEP(*(.fini))
     /* Constructors and destructors */
     KEEP(*crtbegin*.o(.ctors))
     KEEP(*(EXCLUDE_FILE (*crtend*.o ) .ctors))
     KEEP(*(SORT(.ctors.*)))
     KEEP(*(.ctors))
     KEEP(*crtbegin*.o(.dtors))
     KEEP(*(EXCLUDE_FILE (*crtend*.o ) .dtors))
     KEEP(*(SORT(.dtors.*)))
     KEEP(*(.dtors))
     KEEP(*(.jcr))
     _etext = .;
   } > ram

   /* Exception handlers */
   .eh_frame_hdr :
   {
      *(.eh_frame_hdr)
   } > ram
   .eh_frame :
   {
      KEEP (*(.eh_frame))
   } > ram
   .gcc_except_table :
   {
      *(.gcc_except_table)
      *(.gcc_except_table.*)
   } > ram

   /* Read-only data */
   .rodata :
   {
     . = ALIGN(4);
     _frodata = .;
     _frodata_rom = LOADADDR(.rodata);
     *(.rodata .rodata.* .gnu.linkonce.r.*)
     *(.rodata1)
     _erodata = .;
   } > ram

   .data :
   {
      . = ALIGN(4);
      _fdata = .;
      _fdata_rom = LOADADDR(.data);
      *(.data .data.* .gnu.linkonce.d.*)
      *(.data1)
      SORT(CONSTRUCTORS)
      _gp = ALIGN(16) + 0x7ff0;
      *(.sdata .sdata.* .gnu.linkonce.s.*)
      _edata = .;
   } > ram

   .bss :
   {
     . = ALIGN(4);
     _fbss = .;
     *(.dynsbss)
     *(.sbss .sbss.* .gnu.linkonce.sb.*)
     *(.scommon)
     *(.dynbss)
     *(.bss .bss.* .gnu.linkonce.b.*)
     *(COMMON)
     . = ALIGN(4);
     _ebss = .;
     _end = .;
     PROVIDE(end = .);
   } > ram = 0

} /* End of SECTIONS */
/*!@}*/
