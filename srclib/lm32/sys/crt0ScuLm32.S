/*!
 * @file crt0ScuLm32.S
 * @brief Startup code for Lattice Micro 32 LM32 applications in the SCU 
 *        with and without FreeRTOS
 *
 * @copyright 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @date 20.05.2020
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @note When the compiler switch CONFIG_RTOS is defined then this module
 *       is specialized for FreeRTOS.
 */
#include <lm32signal.h>

#ifndef MICO32_FULL_CONTEXT_SAVE_RESTORE
 #define MICO32_FULL_CONTEXT_SAVE_RESTORE
#endif

#ifdef CONFIG_RTOS
   .extern vPortInterruptHandler
#else
   /*
    * This function is implemented in module lm32Interrupts.c 
    */
   .extern _irq_entry
#endif
.extern _onException
.extern _onSysCall
.extern __init
.extern main
.extern __exit
#ifndef CONFIG_NO_INTERRUPTS
   /*
    * Nesting counter for atomic sections.
    * This global variable will used in module lm32Interrupts.c and
    * in the case of using FreeRTOS in module portasm.S.
    */
   .global   __atomic_section_nesting_count
   .section  .data
   .align    4
   .type     __atomic_section_nesting_count, @object
   .size     __atomic_section_nesting_count, 4
__atomic_section_nesting_count:
   .long     1
#endif

#ifndef CONFIG_NO_RESET_COUNTER
   .global  __reset_count
   .section .data
   .align    4
   .type     __reset_count, @object
   .size     __reset_count, 4
__reset_count:
   .long -1    /* Pre-initializing __reset_count with -1 by assembler */
#endif

/* ----------------------------------------------------------------------------
 * Exception handlers - Must be 32 bytes long respectively 8 instructions.
 * @see LatticeMico32ProcessorReferenceManual39.pdf page 22
 */
.section    .boot, "ax", @progbits

.global _reset_handler
.type   _reset_handler, @function
_reset_handler:
/*
 * init r0 from CSR to make sure it's got a valid state before XOR (simulation)
 */
    rcsr    r0, CFG 
    xor     r0, r0, r0
    wcsr    IE, r0             /* Disable the global interrupts */
    wcsr    IM, r0             /* Crear interrupt mask register */ 
    mvhi    r1,     hi(_reset_handler)
    ori     r1, r1, lo(_reset_handler)
    wcsr    EBA, r1
    calli   _crt0
.size	_reset_handler, .-_reset_handler

.global _breakpoint_handler
.type   _breakpoint_handler, @function
_breakpoint_handler:
    sw      (sp+0), ra
    calli   _save_all
    mvi     r1,  SIGTRAP
    calli   _onException
    bi      _restore_all_and_bret
    nop
    nop
    nop
.size _breakpoint_handler, .-_breakpoint_handler

.global _instruction_bus_error_handler
.type   _instruction_bus_error_handler, @function
_instruction_bus_error_handler:
    sw      (sp+0), ra
    calli   _save_all
    mvi     r1,  SIGSEGV
    calli   _onException
    bi      _restore_all_and_eret
    nop
    nop
    nop
.size _instruction_bus_error_handler, .-_instruction_bus_error_handler

.global _watchpoint_handler
.type   _watchpoint_handler, @function
_watchpoint_handler:
    sw      (sp+0), ra
    calli   _save_all
    mvi     r1,  SIGTRAP
    calli   _onException
    bi      _restore_all_and_bret
    nop
    nop
    nop
.size _watchpoint_handler, .-_watchpoint_handler

.global _data_bus_error_handler
.type   _data_bus_error_handler, @function
_data_bus_error_handler:
    sw      (sp+0), ra
    calli   _save_all
    mvi     r1, SIGSEGV
    calli  _onException
    bi     _restore_all_and_eret
    nop
    nop
    nop
.size _data_bus_error_handler, .-_data_bus_error_handler

.global _divide_by_zero_handler
.type   _divide_by_zero_handler, @function
_divide_by_zero_handler:
    sw      (sp+0), ra
    calli   _save_all
    mvi     r1,  SIGFPE
    calli   _onException
    bi      _restore_all_and_eret
    nop
    nop
    nop
.size _divide_by_zero_handler, .-_divide_by_zero_handler

.global _interrupt_handler
.type   _interrupt_handler, @function
_interrupt_handler:
#ifdef CONFIG_RTOS
    /*
     * In the case of using FreeRTOS the interrupt handling will made in module portasm.S.
     * The in portasm.S implemented function vPortInterruptHandler will invoke the
     * interrupt main function _irq_entry.
     */
    bi      vPortInterruptHandler    /* 1 */
    nop                              /* 2 */
    nop                              /* 3 */
    nop                              /* 4 */
    nop                              /* 5 */
#else
    sw      (sp+0), ra               /* 1 */
    calli   _save_all                /* 2 */
    mvi     r1, SIGINT               /* 3 */
 #ifndef CONFIG_NO_INTERRUPTS
    calli   _irq_entry               /* 4 */
 #else
    wcsr    IE, r0                   /* 4 */
 #endif
    bi      _restore_all_and_eret    /* 5 */
#endif
    nop                              /* 6 */
    nop                              /* 7 */
    nop                              /* 8 */
.size     _interrupt_handler, .-_interrupt_handler
.global _system_call_handler
.type _system_call_handler, @function
_system_call_handler:
    sw      (sp+0), ra
    calli   _save_all
    mv      r1, sp
    calli   _onSysCall
    bi      _restore_all_and_eret
    nop
    nop
    nop
.size _system_call_handler, .-_system_call_handler

/* ----------------------------------------------------------------------------
 * Function _crt0
 */
.section    .text, "ax", @progbits
.global _crt0
.type   _crt0, @function
_crt0:
    /* Clear r0 */
    xor     r0, r0, r0

    /* Disable interrupt */ 
    wcsr    ie, r0

    /* Setup stack and global pointer */
    mvhi    sp,     hi(_fstack)
    ori     sp, sp, lo(_fstack)

    /* 
     * Initializing the stack overflow indicator variable
     */
    mvhi    r1,     hi(_endram)
    ori     r1, r1, lo(_endram)
    mvhi    r2,     hi(STACK_MAGIC)
    ori     r2, r2, lo(STACK_MAGIC)
    sw      (r1+0), r2 /* _endram = STACK_MAGIC; */

    /*
     * Initializing .bss by zero.
     */
    mvhi    r1,     hi(_fbss)
    ori     r1, r1, lo(_fbss)
    mvi     r2, 0
    mvhi    r3,     hi(_ebss)
    ori     r3, r3, lo(_ebss)
    sub     r3, r3, r1
    calli   memset      /* memset( _fbss, 0, _ebss - _fbss ); */

#ifndef CONFIG_NO_INTERRUPTS
    /*
     * In the case of a unexpected reset event within a atomic section the
     * atomic nesting counter has to be pre initialized with 1 here!
     * This makes it possible using atomic sections before the 
     * interrupts are initialized and enabled.
     */
    mvhi    r1,     hi(__atomic_section_nesting_count)
    ori     r1, r1, lo(__atomic_section_nesting_count)
    mvi     r2, 1       /* __atomic_section_nesting_count = 1; */
    sw      (r1+0), r2
#endif
#ifndef CONFIG_NO_RESET_COUNTER
    /*
     * Incrementing the reset counter: __reset_count
     */
    mvhi    r1,     hi(__reset_count)
    ori     r1, r1, lo(__reset_count)
    lw      r1, (r1+0)
    addi    r2, r1, 1
    mvhi    r1,     hi(__reset_count)
    ori     r1, r1, lo(__reset_count)
    sw      (r1+0), r2
#endif
    /*
     * The C-function __init() is optional and is implemented in module stubs.c by default. 
     * It is called BEFORE the main function. 
     */
    calli   __init

#ifdef CONFIG_CPLUSPLUS_OBSOLETE
    /*
     * @see https://tldp.org/HOWTO/Program-Library-HOWTO/miscellaneous.html
     */
.weak _init
    /*
     * Call C++ constructors
     */
    calli   _init

.weak _fini
    /*
     * Call C++ destructors on exit
     */
    mvhi    r1, hi(_fini)
    ori     r1, r1, lo(_fini)
    calli   atexit
#endif

    /*
     * Calling the C-function main( 0, 0, 0 )
     */
    mvi     r1, 0
    mvi     r2, 0
    mvi     r3, 0
    calli   main
    
    /*
     * The C-function __exit() is optional and is implemented in module stubs.c by default.
     * It is called AFTER the main function.
     */
    calli   __exit

#ifdef CONFIG_CPLUSPLUS_OBSOLETE
    /*
     * Call exit, which doesn't return, to perform any clean up
     */
    calli   exit    
#else
    /*
     * Endless loop!
     */
loopf:
    bi loopf
#endif

/*---------------------------------------------------------------------------*/
#define ST_OFS 0

#define OFS_R( n )     (ALIGN * (n + ST_OFS))
#define SAVE_R( n )    sw (sp + OFS_R(n)), r##n
#define RESTORE_R( n ) lw r##n, (sp + OFS_R(n))

#ifdef MICO32_FULL_CONTEXT_SAVE_RESTORE
   #define OFS_RA    OFS_R( 28 )
   #define OFS_EA    OFS_R( 29 )
   #define OFS_BA    OFS_R( 30 )
   #define STACK_TOP OFS_R( 32 )
#else
   #define OFS_RA    OFS_R( 11 )
   #define OFS_EA    OFS_R( 12 )
   #define OFS_BA    OFS_R( 13 )
   #define STACK_TOP OFS_R( 15 )
#endif

/* ----------------------------------------------------------------------------
 * Function _save_all
 */
.global _save_all
.type   _save_all, @function
_save_all:
 #if defined( CONFIG_PATCH_LM32_BUG ) && !defined( CONFIG_DEBUG_BY_LOGIK_ANALYSATOR )
    wcsr ie, r0
 #endif
    addi sp, sp, -STACK_TOP
    SAVE_R(1)
    SAVE_R(2)
    SAVE_R(3)
    SAVE_R(4)
    SAVE_R(5)
    SAVE_R(6)
    SAVE_R(7)
    SAVE_R(8)
    SAVE_R(9)
    SAVE_R(10)
#ifdef MICO32_FULL_CONTEXT_SAVE_RESTORE
    SAVE_R(11)
    SAVE_R(12)
    SAVE_R(13)
    SAVE_R(14)
    SAVE_R(15)
    SAVE_R(16)
    SAVE_R(17)
    SAVE_R(18)
    SAVE_R(19)
    SAVE_R(20)
    SAVE_R(21)
    SAVE_R(22)
    SAVE_R(23)
    SAVE_R(24)
    SAVE_R(25)
    SAVE_R(26)
    SAVE_R(27)
#endif
    /* ra and sp need special handling, as they have been modified */
    lw  r1, (sp + STACK_TOP)
    sw  (sp + OFS_RA), r1
    sw  (sp + OFS_EA), ea
    sw  (sp + OFS_BA), ba
#ifdef CONFIG_PATCH_LM32_BUG
   //!! wcsr ie, r0
#endif
    ret
.size   _save_all, .-_save_all

/*! ----------------------------------------------------------------------------
 * @brief Macro restores all registers which has been saved by function _save_all.
 */
.macro RESTORE_ALL
    RESTORE_R(1)
    RESTORE_R(2)
    RESTORE_R(3)
    RESTORE_R(4)
    RESTORE_R(5)
    RESTORE_R(6)
    RESTORE_R(7)
    RESTORE_R(8)
    RESTORE_R(9)
    RESTORE_R(10)
#ifdef MICO32_FULL_CONTEXT_SAVE_RESTORE
    RESTORE_R(11)
    RESTORE_R(12)
    RESTORE_R(13)
    RESTORE_R(14)
    RESTORE_R(15)
    RESTORE_R(16)
    RESTORE_R(17)
    RESTORE_R(18)
    RESTORE_R(19)
    RESTORE_R(20)
    RESTORE_R(21)
    RESTORE_R(22)
    RESTORE_R(23)
    RESTORE_R(24)
    RESTORE_R(25)
    RESTORE_R(26)
    RESTORE_R(27)
#endif
    lw   ra, (sp + OFS_RA)
    lw   ea, (sp + OFS_EA)
    lw   ba, (sp + OFS_BA)
    /* Stack pointer must be restored last, in case it has been updated */
    addi sp, sp, STACK_TOP
.endm

/* ----------------------------------------------------------------------------
 * Function _restore_all_and_eret
 * Restore all registers and return from exception
 */
.global _restore_all_and_eret
.type   _restore_all_and_eret, @function
_restore_all_and_eret:
    /*
     * Sometimes the EIE flag is zero perhaps caused by using of atomic sections,
     * therefore it will set here in any cases.
     */
    mvi     r1, IRQ_EIE /* Enabling interrupt by "eret" (IE = EIE). */
    wcsr    ie, r1
    RESTORE_ALL
    eret
.size   _restore_all_and_eret, .-_restore_all_and_eret

/* ----------------------------------------------------------------------------
 * Function _restore_all_and_bret
 * Restore all registers and return from watchpoint 
 */
.global _restore_all_and_bret
.type   _restore_all_and_bret, @function
_restore_all_and_bret:
    mvi     r1, IRQ_BIE /* Enabling interrupt by "bret" (IE = BIE). */
    wcsr    ie, r1
    RESTORE_ALL
    bret
.size   _restore_all_and_bret, .-_restore_all_and_bret

/*================================== EOF ====================================*/
