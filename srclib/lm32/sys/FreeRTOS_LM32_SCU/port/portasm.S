/*!
 * @file portasm.S
 * @brief Assembler part of LM32 port for FreeRTOS.
 *
 * @date 14.01.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 */

#ifndef __lm32__
  #error This assembler module is for the target Latice Micro32 (LM32) only!
#endif
#ifndef CONFIG_RTOS
  #error Macro CONFIG_RTOS has to be defined in Makefile!
#endif

#include <lm32signal.h>
#include <lm32AssemblerMacros.h>

/*!
 * @brief Pointer to current Task Control Block.
 *
 * We require the address of the pxCurrentTCB variable, but don't want to know
 * any details of its type.
 * @see task.c
 */
#ifdef __DOXYGEN__
extern TCB_t* volatile pxCurrentTCB;
#else
.extern pxCurrentTCB
#endif

#ifdef __DOXYGEN__
/*!
 * The atomic section nesting counter is implemented in the startup module
 * crt0ScuLm32.S
 */
extern volatile uint32_t __atomic_section_nesting_count
#else
.extern __atomic_section_nesting_count
#endif



.extern _irq_entry

#ifdef __DOXYGEN__
void vStartFirstTask( void );
#else
.global vStartFirstTask
#endif

#ifdef __DOXYGEN__
void vPortYield( void );
#else
.global vPortYield
#endif

#ifdef __DOXYGEN__
void vPortInterruptHandler( void );
#else
.global vPortInterruptHandler
#endif

/*!
 * @brief Storage offset in 32-bit values.
 * @note In this case a variable for the context switch flag will used,
 *       therefore the storage offset for register saving will be one.
 * @see __cscf
 */
#define ST_OFS 1


/*! ---------------------------------------------------------------------------
 * @brief Defining of memory place for the context switch cause flag.
 * @note At stack offset (sp+4) will stored a flack which indicates the
 *       reason of context saving.\n
 *       * If its value equal one, than a real LM32 exception
 *         - eg: a interrupt - was happened.\n
 *       * If its value equal zero, so the context saving was caused by
 *         the FreeRTOS scheduler.
 */
#define __cscf (sp + (ALIGN * (1 + 0)))

#ifndef __DOXYGEN__
.section .text
.align ALIGN
#endif

#define MICO32_FULL_CONTEXT_SAVE_RESTORE

#ifdef MICO32_FULL_CONTEXT_SAVE_RESTORE
  #define TO_SAVE 30
  #define STK_RA  28
  #define STK_EA  29
  #define STK_BA   0
#else
  #define TO_SAVE 13
  #define STK_RA  11
  #define STK_EA  12
  #define STK_BA   0
#endif

/*!
 * @brief Stack size in bytes for saving all registers and system variables. 
 * @note ST_OFS is the place of system variable __cscf
 */
#define ST_REG_SIZE (ALIGN * (ST_OFS + TO_SAVE)) //29

/*! ---------------------------------------------------------------------------
 * @brief Macro calculates the absolute stack-pointer offset in bytes.
 * @param i Relative offset in 32-bit values.
 */
#define OS_OFS_R(n)   (sp + (ALIGN * (ST_OFS + TO_SAVE - n)))


#define OS_SAVE_R( n )    sw OS_OFS_R( n ), r##n
#define OS_RESTORE_R( n ) lw r##n, OS_OFS_R( n )

#define OS_SAVE_RA     sw OS_OFS_R(STK_RA), ra
#define OS_SAVE_EA     sw OS_OFS_R(STK_EA), ea
#define OS_SAVE_BA     sw OS_OFS_R(STK_BA), ba

#define OS_RESTORE_RA  lw ra, OS_OFS_R(STK_RA)
#define OS_RESTORE_EA  lw ea, OS_OFS_R(STK_EA)
#define OS_RESTORE_BA  lw ba, OS_OFS_R(STK_BA)

/*! ---------------------------------------------------------------------------
 * @brief Macro saves the content op all LM32 registers
 *        (except R0 and SP (R28)) on the stack.
 */
#ifdef __DOXYGEN__
#define SAVE_REGISTERS
#else
.macro SAVE_REGISTERS
   OS_SAVE_R(1)
   OS_SAVE_R(2)
   OS_SAVE_R(3)
   OS_SAVE_R(4)
   OS_SAVE_R(5)
   OS_SAVE_R(6)
   OS_SAVE_R(7)
   OS_SAVE_R(8)
   OS_SAVE_R(9)
   OS_SAVE_R(10)
 #ifdef MICO32_FULL_CONTEXT_SAVE_RESTORE
   OS_SAVE_R(11)
   OS_SAVE_R(12)
   OS_SAVE_R(13)
   OS_SAVE_R(14)
   OS_SAVE_R(15)
   OS_SAVE_R(16)
   OS_SAVE_R(17)
   OS_SAVE_R(18)
   OS_SAVE_R(19)
   OS_SAVE_R(20)
   OS_SAVE_R(21)
   OS_SAVE_R(22)
   OS_SAVE_R(23)
   OS_SAVE_R(24)
   OS_SAVE_R(25)
   OS_SAVE_R(26)
   OS_SAVE_R(27)
 #endif
   OS_SAVE_RA
   OS_SAVE_EA
   OS_SAVE_BA
.endm
#endif

/*! ---------------------------------------------------------------------------
 * @brief Macro restores all LM32 registers from stack
 *        except R0, R1, R2 and SP (R28)
 */
#ifdef __DOXYGEN__
#define RESTORE_REGISTERS
#else
.macro RESTORE_REGISTERS
   OS_RESTORE_R(3)
   OS_RESTORE_R(4)
   OS_RESTORE_R(5)
   OS_RESTORE_R(6)
   OS_RESTORE_R(7)
   OS_RESTORE_R(8)
   OS_RESTORE_R(9)
   OS_RESTORE_R(10)
 #ifdef MICO32_FULL_CONTEXT_SAVE_RESTORE
   OS_RESTORE_R(11)
   OS_RESTORE_R(12)
   OS_RESTORE_R(13)
   OS_RESTORE_R(14)
   OS_RESTORE_R(15)
   OS_RESTORE_R(16)
   OS_RESTORE_R(17)
   OS_RESTORE_R(18)
   OS_RESTORE_R(19)
   OS_RESTORE_R(20)
   OS_RESTORE_R(21)
   OS_RESTORE_R(22)
   OS_RESTORE_R(23)
   OS_RESTORE_R(24)
   OS_RESTORE_R(25)
   OS_RESTORE_R(26)
   OS_RESTORE_R(27)
 #endif
   OS_RESTORE_RA
   OS_RESTORE_EA
   OS_RESTORE_BA 
.endm
#endif

/*! ---------------------------------------------------------------------------
 * @brief Macro restores the remaining registers which are still used after
 *        restoring of the context and executes the exception-return "eret". 
 */
#ifdef __DOXYGEN__
#define RESTORE_REMANING_REGISTERS_AND_ERET
#else
.macro RESTORE_REMANING_REGISTERS_AND_ERET
   OS_RESTORE_R(1)
   OS_RESTORE_R(2)
   addi sp, sp, ST_REG_SIZE
   eret
.endm
#endif

/*! ---------------------------------------------------------------------------
 * @brief Macro disables the interrupts.
 */
#ifdef __DOXYGEN__
#define DISABLE_INTERRUPTS
#else
.macro DISABLE_INTERRUPTS
   wcsr ie, r0 
.endm
#endif

/*! ---------------------------------------------------------------------------
 * @brief Macro sets the EIE-bit, so the interrupts becomes enabled by
 *        exception-return "eret".
 */
#ifdef __DOXYGEN__
#define ENABLE_INTERRUPTS_BY_ERET
#else
.macro ENABLE_INTERRUPTS_BY_ERET
   mvi  r1, IRQ_EIE     /* Setting EIE bit enabling the IRQs by "eret" */
   wcsr ie, r1          /* Setting EIE bit enabling the IRQs by "eret" */
.endm
#endif

/*! ---------------------------------------------------------------------------
 * @brief Macro to save registers on the stack.
 */
#ifdef __DOXYGEN__
#define SAVE_CONTEXT
#else
.macro SAVE_CONTEXT
   /*
    * Making space for the context on the stack.
    */
   addi      sp,  sp, -ST_REG_SIZE   /* r28 */

   /*
    * Saving all register values of the current respectively old context.
    * Register R0 is always 0, therefore start with R1.
    */
   SAVE_REGISTERS      /* Save all registers except r0 and r1 */

   /*
    * Save the top of stack value to the Task Control Block (TCB).
    */
   LOAD_VAR  r1, pxCurrentTCB
   sw        (r1+0), sp
.endm
#endif

/*! ---------------------------------------------------------------------------
 * @brief Macro to restore registers from the stack.
 */
#ifdef __DOXYGEN__
#define RESTORE_CONTEXT
#else
.macro RESTORE_CONTEXT
   /*
    * Loading last stack value from Task Control Block (TCB).
    */
   LOAD_VAR  r1, pxCurrentTCB    /* Load the pointer of the task control block in register R1. */
   mv        r2, r1              /* Copy R1 to R2 */
   lw        sp, (r1+0)          /* After access, SP (r28) hold stack pointer from actual TCB */
   addi      r1, sp, ST_REG_SIZE /* Calculate new stack pointer */
   sw        (r2+0), r1          /* Store new Stack pointer in actual TCB */

   /*
    * Restoring all register values (except R0, R1 and R28 (SP))
    * for the new context.
    */
   RESTORE_REGISTERS
.endm
#endif

#ifdef CONFIG_RTOS
/*! ---------------------------------------------------------------------------
 * @brief Start first task, resets the global variable
 *        __atomic_section_nesting_count and enable global interrupt
 */
#ifdef __DOXYGEN__
void vStartFirstTask( void ) {}
#else
vStartFirstTask:
   LOAD_ADDR r1, __atomic_section_nesting_count
   sw        (r1+0), r0          /* __atomic_section_nesting_count = 0; */
   ENABLE_INTERRUPTS_BY_ERET
   RESTORE_CONTEXT
   RESTORE_REMANING_REGISTERS_AND_ERET
#endif

#ifdef CONFIG_PATCH_LM32_BUG
#endif

/*! ---------------------------------------------------------------------------
 * @brief Function for context switching it enables the IRQs as well.
 */
#ifdef __DOXYGEN__
void vPortYield( void ) { vTaskSwitchContext(); }
#else
vPortYield:
   DISABLE_INTERRUPTS
   SAVE_CONTEXT                 /* Save all registers */

   /*
    * __atomic_section_nesting_count++;
    */
   LOAD_ADDR r1, __atomic_section_nesting_count
   lw        r2, (r1+0)
   addi      r2, r2, 1
   sw        (r1+0), r2

   sw        __cscf, r0         /* Set the context switch cause flag to zero */
   calli     vTaskSwitchContext /* Call this C-function implemented in task.c */

   /*
    * __atomic_section_nesting_count--;
    */
   LOAD_ADDR r1, __atomic_section_nesting_count
   lw        r2, (r1+0)
   addi      r2, r2, -1
   sw        (r1+0), r2

   RESTORE_CONTEXT              /* Restore all registers except R1. */
   lw        r1, __cscf         /* Load context change cause flag in r1 */
   bne       r0, r1, L_INT_SAVE /* Jump if context switch was caused by LM32 exception  */
   mv        ea, ra             /* Return-Address is stored in RA */
L_INT_SAVE:
#if 1
   LOAD_VAR  r1, __atomic_section_nesting_count
   bne       r1, r0, L_NOT_ENABLE /* Jump to L_NOT_ENABLE if nesting count not zero */
#endif
   ENABLE_INTERRUPTS_BY_ERET
L_NOT_ENABLE:
   RESTORE_REMANING_REGISTERS_AND_ERET
#endif

#endif /* ifdef CONFIG_RTOS */

/*! ---------------------------------------------------------------------------
 * @brief Interrupt Handler. Jump target from _interrupt_handler
 *        implemented in crt0FreeRTOS.S
 * @see crt0FreeRTOS.S
 */
#ifdef __DOXYGEN__
void vPortInterruptHandler( void ) { _irq_entry(); }
#else
vPortInterruptHandler:
   SAVE_CONTEXT                 /* Save all registers */
   mvi       r1, 1              /* Set the context switch cause flag to one */
   sw        __cscf, r1         /* and store it in stack. */
   mvi       r1, SIGINT         /* Argument for C-function _irq_entry (will ignored)*/
   calli     _irq_entry         /* Call this C- callback function */
   RESTORE_CONTEXT              /* Restore all registers except R1. */
   lw        r1, __cscf         /* Load context switch cause flag in r1 */
   bne       r0, r1, L_RETURN   /* Jump if context change was caused from LM32 exception  */
   mv        ea, ra             /* Return-Address is stored in RA */
L_RETURN:
   sw        __cscf, r0 //!!
   ENABLE_INTERRUPTS_BY_ERET
   RESTORE_REMANING_REGISTERS_AND_ERET
#endif

/*================================== EOF ====================================*/
