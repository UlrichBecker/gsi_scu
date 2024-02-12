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
#include <event_measurement.h>

#define configCPU_CLOCK_HZ   (USRCPUCLK * 10)

#define FREQU 10

TIME_MEASUREMENT_T g_evTime = TIME_MEASUREMENT_INITIALIZER;

volatile static unsigned int g_count = 0;

static void onTimerInterrupt( const unsigned int intNum, const void* pContext )
{
   static unsigned int irqCount = 0;

   if( irqCount == 0 )
      g_count++;
   
   irqCount++;
   irqCount %= FREQU;
}


void main( void )
{
   mprintf( "Timer IRQ test\nCompiler: " COMPILER_VERSION_STRING "\n" );
   mprintf( "CPU frequency: %d Hz\n", configCPU_CLOCK_HZ );
   mprintf( "Interrupt frequency: " TO_STRING( FREQU ) " HZ\n" );
   unsigned int oldCount = g_count - 1;

   SCU_LM32_TIMER_T* pTimer = lm32TimerGetWbAddress();
   if( (unsigned int)pTimer == ERROR_NOT_FOUND )
   {
      mprintf( ESC_ERROR "ERROR: Timer not found!\n" ESC_NORMAL );
      while( true );
   }

   mprintf( "Timer found at wishbone base address 0x%p\n", pTimer );

   lm32TimerSetPeriod( pTimer, configCPU_CLOCK_HZ / FREQU );
   lm32TimerEnable( pTimer );
   irqRegisterISR( TIMER_IRQ, (void*)pTimer, onTimerInterrupt );
   irqEnable();

   unsigned int i = 0;
   static const char fan[] = { '|', '/', '-', '\\' };
   mprintf( "Entering polling loop...\n" );
   while( true )
   {
      volatile unsigned int currentCount;
      ATOMIC_SECTION() currentCount = g_count;
      if( oldCount == currentCount )
         continue;
      oldCount = currentCount;

      mprintf( ESC_BOLD "\r%c" ESC_NORMAL, fan[i++] );
      i %= ARRAY_SIZE( fan );
   }
}

/*================================== EOF ====================================*/
