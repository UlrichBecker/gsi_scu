/*!
 * @file   irqTimerTest.c
 * @brief  Test of LM32 timer-interrupt necessary for FreeRTOS.
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      12.10.2022
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
#include <stdbool.h>
#include <lm32signal.h>
#include <eb_console_helper.h>
#include <scu_lm32Timer.h>
#include <lm32Interrupts.h>

#define configCPU_CLOCK_HZ   (USRCPUCLK * 1000)

//#define FREQU 100000
#define FREQU 4


volatile unsigned int g_count = 0;
const char g_fan[] = { '|', '/', '-', '\\' };


static void onTimerInterrupt( const unsigned int intNum, const void* pContext )
{
   static unsigned int irqCount = 0;

   if( irqCount == 0 )
      g_count++;

   irqCount++;
   irqCount %= (FREQU / ARRAY_SIZE( g_fan ));
}


void irqInfo( void )
{
   mprintf( "Nesting count: %d; IE: 0x%X\n", irqGetAtomicNestingCount(), irqGetEnableRegister() );
}

void main( void )
{
   mprintf( "Timer IRQ test\nCompiler: " COMPILER_VERSION_STRING "\n" );
   mprintf( "CPU frequency: %d Hz\n", configCPU_CLOCK_HZ );
   mprintf( "IRQ frequency: " TO_STRING( FREQU ) " Hz\n" );
   unsigned int oldCount = g_count - 1;

   SCU_LM32_TIMER_T* pTimer = lm32TimerGetWbAddress();
   if( (unsigned int)pTimer == ERROR_NOT_FOUND )
   {
      mprintf( ESC_ERROR "ERROR: Timer not found!\n" ESC_NORMAL );
      while( true );
   }

   mprintf( "Timer found at wishbone base address 0x%p\n", pTimer );
   irqInfo();
   lm32TimerSetPeriod( pTimer, configCPU_CLOCK_HZ / FREQU );
   lm32TimerEnable( pTimer );
   irqRegisterISR( TIMER_IRQ, NULL, onTimerInterrupt );
   irqInfo();
   //irqEnable();
   criticalSectionExit();
  // criticalSectionExit();
   irqInfo();
   
   
   criticalSectionEnter();
   criticalSectionEnter();
   criticalSectionEnter();
   irqInfo();
   criticalSectionExit();
   criticalSectionExit();
   criticalSectionExit();
   irqInfo();
   
   unsigned int i = 0;
   while( true )
   {
      volatile unsigned int currentCount;

      //criticalSectionEnter();
     // asm volatile ( "mvi r1, 2\n\twcsr ie, r1\n\t" :::"r1", "memory" );
      asm volatile ( "wcsr ie, r0\n\t" ::: "memory" );
         currentCount = g_count;
      asm volatile ( "mvi r1, 1\n\twcsr ie, r1\n\t" :::"r1", "memory" );
     // criticalSectionExit();

      if( oldCount != currentCount )
      {
         oldCount = currentCount;

         mprintf( ESC_BOLD "\r%c" ESC_NORMAL, g_fan[i++] );
         i %= ARRAY_SIZE( g_fan );
      }
   }
}

/*================================== EOF ====================================*/
