/*!
 * @file portasm.S
 * @brief Assembler part of LM32 port for FreeRTOS.
 * >>> PvdS <<<
 * @date 14.01.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 */

#if !defined(__lm32__) && !defined(__CPPCHECK__)
  #error This assembler module is for the target Latice Micro32 (LM32) only!
#endif
#ifndef CONFIG_RTOS
  #error Macro CONFIG_RTOS has to be defined in Makefile!
#endif

#include <lm32signal.h>
#include <lm32AssemblerMacros.h>
#include <lm32_port_common.h>

#ifndef __CPPCHECK__

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

.extern dbgPrintValue

#ifdef __DOXYGEN__
void vStartFirstTask( void );
#else
.global vStartFirstTask
#endif

#ifdef __DOXYGEN__
void vPortYieldLm32( void );
#else
.global vPortYieldLm32
#endif

#ifdef __DOXYGEN__
void vPortInterruptHandler( void );
#else
.global vPortInterruptHandler
#endif

#ifndef __DOXYGEN__
.section .text
.align ALIGN
#endif


/*! ---------------------------------------------------------------------------
 * @brief Defining of memory place for the context switch cause flag.
 * @note At stack offset (sp+4) will stored a flack which indicates the
 *       reason of context saving.\n
 *       * If its value equal one, than a real LM32 exception
 *         - eg: a interrupt - was happened.\n
 *       * If its value equal zero, so the context saving was caused by
 *         the FreeRTOS scheduler.
 */
#define __cscf (sp + (ALIGN * (1 + CSCF_POS)))

#ifdef CONFIG_SAFE_ASNC
/*! --------------------------------------------------------------------------
 * @brief Defining of memory place for saving of the atomic section nesting
 *        counter __atomic_section_nesting_count in each thread.
 * @see https://www.freertos.org/FreeRTOS_Support_Forum_Archive/November_2005/freertos_uxCriticalNesting_1386799.html
 */
#define __asnc (sp + (ALIGN * (1 + ASNC_POS)))

#endif /* ifdef CONFIG_SAFE_ASNC */

/*!
 * @brief Stack size in bytes for saving all registers and system variables. 
 * @note ST_OFS is the place of system variable __cscf
 */
#define OS_STACK_BYTE_SIZE (ALIGN * OS_STACK_DWORD_SIZE)

/*! ---------------------------------------------------------------------------
 * @brief Macro calculates the absolute stack-pointer offset in bytes.
 * @param n Relative offset in 32-bit values.
 */
#define OS_OFS_R(n)   (sp + (ALIGN * (OS_STACK_DWORD_SIZE - n)))


#define OS_SAFE_R( n )    sw OS_OFS_R( n ), r##n
#define OS_RESTORE_R( n ) lw r##n, OS_OFS_R( n )

#define OS_SAFE_RA     sw OS_OFS_R(STK_RA), ra
#define OS_SAFE_EA     sw OS_OFS_R(STK_EA), ea
#define OS_SAFE_BA     sw OS_OFS_R(STK_BA), ba

#define OS_RESTORE_RA  lw ra, OS_OFS_R(STK_RA)
#define OS_RESTORE_EA  lw ea, OS_OFS_R(STK_EA)
#define OS_RESTORE_BA  lw ba, OS_OFS_R(STK_BA)

#ifdef CONFIG_SAFE_ASNC
.macro OS_SAFE_ASNC
   LOAD_VAR   r1, __atomic_section_nesting_count
   sw __asnc, r1
.endm

.macro OS_RESTORE_ASNC
   lw         r2, __asnc
   LOAD_ADDR  r1, __atomic_section_nesting_count
   sw         (r1+0), r2
.endm
#else
  #define OS_SAFE_ASNC
  #define OS_RESTORE_ASNC
#endif /* /ifdef CONFIG_SAFE_ASNC */

/*! ---------------------------------------------------------------------------
 * @brief Function saves all registers and the atomic nesting counter
 *        except SP and RA which needs a special handling.
 */
#ifdef __DOXYGEN__
STATIC void _safeRegisters( void ) {}
#else
.type   _safeRegisters, @function
_safeRegisters:
   OS_SAFE_R(1)
   OS_SAFE_R(2)
   OS_SAFE_R(3)
   OS_SAFE_R(4)
   OS_SAFE_R(5)
   OS_SAFE_R(6)
   OS_SAFE_R(7)
   OS_SAFE_R(8)
   OS_SAFE_R(9)
   OS_SAFE_R(10)
   OS_SAFE_R(11)
   OS_SAFE_R(12)
   OS_SAFE_R(13)
   OS_SAFE_R(14)
   OS_SAFE_R(15)
   OS_SAFE_R(16)
   OS_SAFE_R(17)
   OS_SAFE_R(18)
   OS_SAFE_R(19)
   OS_SAFE_R(20)
   OS_SAFE_R(21)
   OS_SAFE_R(22)
   OS_SAFE_R(23)
   OS_SAFE_R(24)
   OS_SAFE_R(25)
   OS_SAFE_R(26)
   OS_SAFE_R(27)
   OS_SAFE_EA
   OS_SAFE_BA
   OS_SAFE_ASNC
   ret
.size _safeRegisters, .-_safeRegisters
#endif

/*! ---------------------------------------------------------------------------
 * @brief function restores all registers and the atomic nesting counter 
 *        except R1, R2, RA and SP.
 * @note  R1, R2 and SP will restored in function _restoreRemainingRegistersAndERET.
 *        Return-address RA needs a special handling.
 * @see _restoreRemainingRegistersAndERET
 */
#ifdef __DOXYGEN__
STATIC void _restoreRegisters( void );
#else
.type _restoreRegisters, @function
_restoreRegisters:
   OS_RESTORE_R(3)
   OS_RESTORE_R(4)
   OS_RESTORE_R(5)
   OS_RESTORE_R(6)
   OS_RESTORE_R(7)
   OS_RESTORE_R(8)
   OS_RESTORE_R(9)
   OS_RESTORE_R(10)
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
   OS_RESTORE_EA
   OS_RESTORE_BA
   OS_RESTORE_ASNC
   ret
.size _restoreRegisters, .-_restoreRegisters
#endif

/*! ---------------------------------------------------------------------------
 * @brief Function restores the remaining registers which are still used after
 *        restoring of the context and executes the exception-return "eret".
 * @note CAUTION: Invoke this function by "bi" rather than "calli"!
 * @see _restoreRegisters
 */
#ifdef __DOXYGEN__
STATIC void _restoreRemainingRegistersAndERET( void )
#else
.type _restoreRemainingRegistersAndERET, @function
_restoreRemainingRegistersAndERET:
   lw  r1, __cscf         /* Load context change cause flag in r1 */
   bne r0, r1, L_INT_SAVE /* Jump if context switch was caused by LM32 exception  */
   mv  ea, ra             /* Converts "eret" into "ret". Return-Address is stored in RA */
L_INT_SAVE:
   OS_RESTORE_R(1)
   OS_RESTORE_R(2)
   addi sp, sp, OS_STACK_BYTE_SIZE
   eret
.size _restoreRemainingRegistersAndERET, .-_restoreRemainingRegistersAndERET
#endif

/*! ---------------------------------------------------------------------------
 * @brief Macro disables the interrupts.
 */
#ifdef __DOXYGEN__
#define DISABLE_INTERRUPTS
#else
.macro DISABLE_INTERRUPTS
   wcsr ie, r0
 #ifdef CONFIG_PATCH_LM32_BUG
   wcsr ie, r0
   wcsr ie, r0
 #endif
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
#define SAFE_CONTEXT
#else
.macro SAFE_CONTEXT
   /*
    * Making space for the context on the stack.
    */
   addi      sp,  sp, -OS_STACK_BYTE_SIZE   /* r28 */

   OS_SAFE_RA
   calli _safeRegisters

   /*
    * Save the top of stack value to the Task Control Block (TCB).
    */
   LOAD_VAR  r1, pxCurrentTCB
   sw        (r1+0), sp
.endm
#endif


/*! ---------------------------------------------------------------------------
 * @brief Function initialized the stack pointer with the actual pointer of
 *        the task control block (TCB), and adds to the pointer of TCB
 *        the value sp + OS_STACK_BYTE_SIZE.
 */
#ifdef __DOXYGEN__
STATIC void _initStackWithTCB( void ) {}
#else
.type _initStackWithTCB, @function
_initStackWithTCB:
   LOAD_VAR  r1, pxCurrentTCB    /* Load the pointer of the task control block in register R1. */
   mv        r2, r1              /* Copy R1 to R2 */
   lw        sp, (r2+0)          /* After access, SP (r28) hold stack pointer from actual TCB */
   addi      r1, sp, OS_STACK_BYTE_SIZE /* Calculate new stack pointer */
   sw        (r2+0), r1          /* Store new Stack pointer in actual TCB */
   ret
.size _initStackWithTCB, .-_initStackWithTCB
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
   calli _initStackWithTCB

   /*
    * Restoring all register values (except R0, R1 and R28 (SP))
    * for the new context.
    */
   calli _restoreRegisters
   OS_RESTORE_RA
.endm
#endif

/*===========================================================================*/
/*=                            Public functions                             =*/
/*===========================================================================*/

/*! ---------------------------------------------------------------------------
 * @brief Start first task, resets the global variable
 *        __atomic_section_nesting_count and enable global interrupt
 */
#ifdef __DOXYGEN__
void vStartFirstTask( void ) {}
#else
vStartFirstTask:
 #ifndef CONFIG_SAFE_ASNC
   LOAD_ADDR r1, __atomic_section_nesting_count
   sw        (r1+0), r0          /* __atomic_section_nesting_count = 0; */
 #endif
  /*
   * The critical section nesting counter __atomic_section_nesting_count
   * becomes zero by the C-function pxPortInitialiseStack()
   * implemented in port.c.
   */
   ENABLE_INTERRUPTS_BY_ERET
   RESTORE_CONTEXT
   bi _restoreRemainingRegistersAndERET
.size vStartFirstTask, .-vStartFirstTask
#endif

#ifdef CONFIG_PATCH_LM32_BUG
#endif

/*! ---------------------------------------------------------------------------
 * @brief Function for context switching it enables the IRQs as well.
 */
#ifdef __DOXYGEN__
void vPortYieldLm32( void ) { vTaskSwitchContext(); }
#else
vPortYieldLm32:
   DISABLE_INTERRUPTS
   SAFE_CONTEXT                 /* Save all registers */

   /*
    * __atomic_section_nesting_count++;
    */
   LOAD_ADDR r1, __atomic_section_nesting_count
   lw        r2, (r1+0)
   addi      r2, r2, 1
   sw        (r1+0), r2

   sw        __cscf, r0         /* Set the context switch cause flag to zero */
   calli     vTaskSwitchContext /* Call this C-function implemented in task.c */

#ifndef CONFIG_SAFE_ASNC
   /*
    * __atomic_section_nesting_count--;
    * When the nesting counter becomes restored (CONFIG_SAFE_ASNC is defined)
    * then it's not necessary to decrement it here.
    */
   LOAD_ADDR r1, __atomic_section_nesting_count
   lw        r2, (r1+0)
   addi      r2, r2, -1
   sw        (r1+0), r2
#endif
   RESTORE_CONTEXT              /* Restore all registers except R1, R2 and SP. */
   LOAD_VAR  r1, __atomic_section_nesting_count
   bne       r1, r0, L_NOT_ENABLE /* Jump to L_NOT_ENABLE if nesting count not zero */
   ENABLE_INTERRUPTS_BY_ERET
L_NOT_ENABLE:
   bi _restoreRemainingRegistersAndERET
.size vPortYieldLm32, .-vPortYieldLm32
#endif

/*! ---------------------------------------------------------------------------
 * @brief Interrupt Handler. Jump target from _interrupt_handler
 *        implemented in crt0FreeRTOS.S
 * @see crt0FreeRTOS.S
 */
#ifdef __DOXYGEN__
void vPortInterruptHandler( void ) { _irq_entry(); }
#else
vPortInterruptHandler:
 #if defined( CONFIG_PATCH_LM32_BUG ) && !defined( CONFIG_DEBUG_BY_LOGIK_ANALYSATOR )
   wcsr      ie, r0
 #endif
   SAFE_CONTEXT                 /* Save all registers */
   mvi       r1, 1              /* Set the context switch cause flag to one */
   sw        __cscf, r1         /* and store it in stack. */
   mvi       r1, SIGINT         /* Argument for C-function _irq_entry (will ignored)*/
   calli     _irq_entry         /* Call this C- callback function */
   RESTORE_CONTEXT              /* Restore all registers except R1, R2 and SP. */
   ENABLE_INTERRUPTS_BY_ERET
   bi _restoreRemainingRegistersAndERET
.size vPortInterruptHandler, .-vPortInterruptHandler
#endif

#endif /* ifndef __CPPCHECK__ */
/*================================== EOF ====================================*/
