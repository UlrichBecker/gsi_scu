/*!
 * @file   lm32Interrupts.c
 * @brief  General administration of the interrupt handling and
 *         critical resp. atomic sections for LM32
 *
 * @note This module is suitable for FreeRTOS-port.
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      21.01.2020
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#include "lm32Interrupts.h"
#ifdef CONFIG_USE_INTERRUPT_TIMESTAMP
 #include <scu_wr_time.h>
#endif

/*!
 * @ingroup INTERRUPT
 * @brief Nesting counter for critical sections.
 * @note The nesting counter is implemented in the startup module
 *       crt0ScuLm32.S and becomes pre-initialized with 1 so it becomes
 *       possible to use critical sections before the global interrupt
 *       is enabled. \n
 *       The function irqEnable() for non FreeRTOS applications or
 *       the FreeRTOS function vTaskStartScheduler() respectively
 *       the port-function xPortStartScheduler() will reset this counter
 *       to zero.
 * @see crt0ScuLm32.S
 */
extern volatile uint32_t __atomic_section_nesting_count;

#ifdef CONFIG_USE_INTERRUPT_TIMESTAMP
/*!
 * @ingroup INTERRUPT
 * @brief White rabbit time stamp of the last occurred interrupt.
 */
volatile uint64_t mg_interruptTimestamp = 0LL;
#endif

/*!
 * @ingroup INTERRUPT
 * @brief This flag becomes "true" f the program flow is in the
 *        interrupt routine, else it is "false".
 */
volatile bool mg_isInContext = false;

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief ISR entry type
 */
typedef struct
{
   /*!
    * @brief Function pointer of the interrupt service routine of
    *        the concerning interrupt number.
    */
   ISRCallback pfCallback;

   /*!
    * @brief User tunnel: second parameter of the interrupt service routine.
    */
   void*       pContext;
} ISR_ENTRY_T;

/*!
 * @ingroup INTERRUPT
 * @brief  ISREntry table
 *
 * The maximum number of items of the table depends on the macro
 * MAX_LM32_INTERRUPTS which can be overwritten by the makefile.
 * The default and maximum value of MAX_LM32_INTERRUPTS is 32.
 *
 * @see MAX_LM32_INTERRUPTS
 */
STATIC ISR_ENTRY_T ISREntryTable[MAX_LM32_INTERRUPTS] = {{NULL, NULL}};

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
void irqClearEntryTab( void )
{
   criticalSectionEnter();
   for( unsigned int i = 0; i < ARRAY_SIZE( ISREntryTable ); i++ )
   {
      ISREntryTable[i].pfCallback = NULL;
      ISREntryTable[i].pContext   = NULL;
   }
   criticalSectionExit();
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
inline bool irqIsInContext( void )
{
   return mg_isInContext;
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
inline unsigned int irqGetAtomicNestingCount( void )
{
   return __atomic_section_nesting_count;
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
inline uint32_t* irqGetNestingCountPointer( void )
{
   return (uint32_t*) &__atomic_section_nesting_count;
}

inline void irqPresetAtomicNestingCount( void )
{
   irqSetEnableRegister( 0 );
   __atomic_section_nesting_count = 1;
}

#ifndef CONFIG_RTOS
/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
inline void irqEnable( void )
{
   __atomic_section_nesting_count = 0;
   _irqEnable();
}
#endif

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
OVERRIDE uint32_t _irqGetPendingMask( const unsigned int intNum )
{
   return (1 << intNum);
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
OVERRIDE unsigned int _irqReorderPriority( const unsigned int prio )
{
   return prio;
}

#ifdef CONFIG_USE_INTERRUPT_TIMESTAMP
/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
uint64_t irqGetTimestamp( void )
{
   criticalSectionEnter();
   const uint64_t timestamp = mg_interruptTimestamp;
   criticalSectionExit();
   return timestamp;
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
uint64_t irqGetTimeSinceLastInterrupt( void )
{
   criticalSectionEnter();
   const uint64_t ret = getWrSysTime() - mg_interruptTimestamp; 
   criticalSectionExit();
   return ret;
}

#endif /* ifdef CONFIG_USE_INTERRUPT_TIMESTAMP */

#ifdef CONFIG_INTERRUPT_PEDANTIC_CHECK
/*! --------------------------------------------------------------------------
 * @brief Function is for debug purposes only.
 */
void irqPrintInfo( uint32_t ie,  uint32_t nc )
{
   mprintf( "\nAtomic nesting count: %d\n"
            "Interrupt-enable:     0b%02b\n"
            "Interrupt-pending:    0b%02b\n",
            nc, ie,
            irqGetPendingRegister() );
}

void irqPrintNestingCounter( void )
{
   mprintf( "\nAtomic nesting count: 0x%08X, %d\n",
            __atomic_section_nesting_count, __atomic_section_nesting_count );
}
#endif

#if defined( CONFIG_RTOS ) && !defined( CONFIG_IRQ_ENABLING_IN_ATOMIC_SECTIONS )
  //#define CONFIG_IRQ_ENABLING_IN_ATOMIC_SECTIONS
#endif

#ifdef CONFIG_DEBUG_BY_LOGIK_ANALYSATOR
  #warning CAUTION! SCU-BUS becomes misused for logik-analysator by writing 0x47110815 on it for triggering the logc-analysator.
extern volatile uint16_t* g_pScuBusBase;
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief General Interrupt Handler (invoked by low-level routine in portasm.S)
 *
 * If an interrupt-handler exists for the relevant interrupt (as detected
 * from "ip" and "im" cpu registers), then invoke the handler else disable the
 * interrupt in the register "im".
 *
 * @todo Enhance via compiler-switch to the ability of nested interrupt
 *       handling. E.g.: CONFIG_NESTED_IRQS
 */
void _irq_entry( void )
{
   mg_isInContext = true;
#ifdef CONFIG_DEBUG_BY_LOGIK_ANALYSATOR
   if( (irqGetEnableRegister() & IRQ_EIE) == 0 )
   {
      //mprintf( "*\n" );
      *g_pScuBusBase = 0x4711;
   }
#endif
   IRQ_ASSERT( (irqGetEnableRegister() & IRQ_IE) == 0 );
 //  mprintf( " %X\n", irqGetEnableRegister() );
#ifndef CONFIG_PATCH_LM32_BUG
   IRQ_ASSERT( (irqGetEnableRegister() & IRQ_EIE) != 0 );
#endif
   IRQ_ASSERT( irqGetPendingRegister() != 0 );

   /*
    * Allows using of atomic sections within interrupt context.
    */
#ifdef CONFIG_IRQ_ENABLING_IN_ATOMIC_SECTIONS
   __atomic_section_nesting_count++;
 #ifdef CONFIG_INTERRUPT_PEDANTIC_CHECK
   const volatile uint32_t tempNestingCount = __atomic_section_nesting_count;
 #endif
#else
   IRQ_ASSERT( __atomic_section_nesting_count == 0 );
   __atomic_section_nesting_count = 1;
#endif
   /*!
    * @brief Copy of the interrupt pending register before reset.
    */
   uint32_t ip;

   /*
    * As long as there is an interrupt pending...
    */
#ifdef CONFIG_IRQ_RESET_IP_AFTER
   #warning CONFIG_IRQ_RESET_IP_AFTER active!
   while( (ip = irqGetPendingRegister()) != 0 )
#else
   while( (ip = irqGetAndResetPendingRegister()) != 0 )
   //while( (ip = irqGetAndResetPendingRegister() & irqGetMaskRegister()) != 0 )
#endif
   {
    //  mprintf( "ip: 0x%X\n", ip );
   #ifdef CONFIG_USE_INTERRUPT_TIMESTAMP
      mg_interruptTimestamp = getWrSysTime();
   #endif
     /*
      * Zero has the highest priority.
      */
      for( unsigned int prio = 0; prio < ARRAY_SIZE( ISREntryTable ); prio++ )
      {
         const unsigned int intNum = _irqReorderPriority( prio );
         IRQ_ASSERT( intNum < ARRAY_SIZE( ISREntryTable ) );
         const uint32_t mask = _irqGetPendingMask( intNum );

          /*
           * Is this interrupt pending?
           */
         if( (mask & ip) == 0 )
         { /*
            * No, go to next possible interrupt.
            */
            continue;
         }

         /*
          * Handling of detected pending interrupt.
          */
         const ISR_ENTRY_T* pCurrentInt = &ISREntryTable[intNum];
         if( pCurrentInt->pfCallback != NULL )
         { /*
            * Invoking the callback function of the current handled
            * interrupt.
            */
            pCurrentInt->pfCallback( intNum, pCurrentInt->pContext );
         }
         else
         { /*
            * Ridding of unregistered interrupt so it doesn't bothering
            * any more...
            */
            irqSetMaskRegister( irqGetMaskRegister() & ~mask );
         }
      #ifdef CONFIG_IRQ_RESET_IP_AFTER
         irqResetPendingRegister( mask );
      #endif
      }
   }

   mg_isInContext = false;
   /*
    * Allows using of atomic sections within interrupt context.
    */
#ifdef CONFIG_IRQ_ENABLING_IN_ATOMIC_SECTIONS
   IRQ_ASSERT( __atomic_section_nesting_count == tempNestingCount );
   __atomic_section_nesting_count--;
#else
   IRQ_ASSERT( __atomic_section_nesting_count == 1 );
   __atomic_section_nesting_count = 0;
#endif
 //  IRQ_ASSERT( irqGetEnableRegister() == 0 ); //IRQ_EIE );
 //  IRQ_ASSERT( irqGetEnableRegister() == IRQ_EIE );
}

//#define CONFIG_SAVE_BIE_AND_EIE
//#define CONFIG_RESTORE_BIE_AND_EIE

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
OPTIMIZE( "O1" ) /* O1 prevents a function epilogue and prologue. */
void criticalSectionEnterBase( void )
{
   asm volatile
   (
   #ifndef CONFIG_DISABLE_CRITICAL_SECTION
    #ifdef  CONFIG_SAVE_BIE_AND_EIE
      "rcsr   r1, ie                                                  \n\t"
      "andi   r1, r1, " TO_STRING( (~IRQ_IE) & (IRQ_BIE | IRQ_EIE) ) "\n\t"
      "wcsr   ie, r1                                                  \n\t"
    #else
      "wcsr   ie, r0                                                  \n\t"
     #ifdef CONFIG_PATCH_LM32_BUG
      "wcsr   ie, r0                                                  \n\t"
      "wcsr   ie, r0                                                  \n\t"
     #endif
    #endif
   #endif
      "orhi   r1, r0, hi(__atomic_section_nesting_count)              \n\t"
      "ori    r1, r1, lo(__atomic_section_nesting_count)              \n\t"
      "lw     r2, (r1+0)                                              \n\t"
      "addi   r2, r2, 1                                               \n\t"
      "sw     (r1+0), r2                                              \n\t"
      :
      :
      : "r1", "r2", "memory"
   );
}

#define CONFIG_IRQ_ALSO_ENABLE_IF_COUNTER_ALREADY_ZERO

//#define IRQ_ENABLE (IRQ_EIE | IRQ_IE)
#define IRQ_ENABLE IRQ_IE

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
OPTIMIZE( "O1" ) /* O1 prevents a function epilogue and prologue. */
void criticalSectionExitBase( void )
{
   asm volatile
   (
      "orhi   r1, r0, hi(__atomic_section_nesting_count)              \n\t"
      "ori    r1, r1, lo(__atomic_section_nesting_count)              \n\t"
      "lw     r2, (r1+0)                                              \n\t"
   #ifdef CONFIG_IRQ_ALSO_ENABLE_IF_COUNTER_ALREADY_ZERO
      "be     r2, r0, L_ENABLE                                        \n\t"
   #else
      "be     r2, r0, L_NO_ENABLE                                     \n\t"
   #endif
      "addi   r2, r2, -1                                              \n\t"
      "sw     (r1+0), r2                                              \n\t"
      "bne    r2, r0, L_NO_ENABLE                                     \n"
   "L_ENABLE:                                                         \n\t"
   #ifdef CONFIG_RESTORE_BIE_AND_EIE
      "rcsr   r1, ie                                                  \n\t"
      "ori    r1, r1, " TO_STRING( IRQ_IE ) "                         \n\t"
   #else
      "mvi    r1, " TO_STRING( IRQ_ENABLE ) "                         \n\t"
   #endif
      "wcsr   ie, r1                                                  \n"
   "L_NO_ENABLE:                                                      \n\t"
      :
      :
      : "r1", "r2", "memory"
   );
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
void irqRegisterISR( const unsigned int intNum, void* pContext,
                     ISRCallback pfCallback )
{
   IRQ_ASSERT( intNum < ARRAY_SIZE( ISREntryTable ) );

   criticalSectionEnter();

   ISREntryTable[intNum].pfCallback = pfCallback;
   ISREntryTable[intNum].pContext   = pContext;

   const uint32_t mask = _irqGetPendingMask( intNum );
   const uint32_t im = irqGetMaskRegister();
   irqSetMaskRegister( (pfCallback == NULL)? (im & ~mask) : (im | mask) );

   criticalSectionExit();
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
void irqDisableSpecific( const unsigned int intNum )
{
   IRQ_ASSERT( intNum < ARRAY_SIZE( ISREntryTable ) );

   criticalSectionEnter();
   irqSetMaskRegister( irqGetMaskRegister() & ~_irqGetPendingMask( intNum ) );
   criticalSectionExit();
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
bool irqIsSpecificEnabled( const unsigned int intNum )
{
   return (irqGetMaskRegister() & _irqGetPendingMask( intNum )) != 0;
}

/*! ---------------------------------------------------------------------------
 * @see lm32Interrupts.h
 */
void irqEnableSpecific( const unsigned int intNum )
{
   IRQ_ASSERT( intNum < ARRAY_SIZE( ISREntryTable ) );

   criticalSectionEnter();
   irqSetMaskRegister( irqGetMaskRegister() | _irqGetPendingMask( intNum ) );
   criticalSectionExit();
}

/*================================== EOF ====================================*/
