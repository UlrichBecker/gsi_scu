/*!
 *  @file lm32memtest.c
 *  @brief Performance-test of DDR3-memory for LM32.
 *
 *  @see scu_ddr3_lm32.c
 *  @see scu_ddr3.h
 *  @see
 *  <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/MacroF%C3%BCr1GbitDDR3MT41J64M16LADesSCUCarrierboards">
 *     DDR3 VHDL Macro der SCU3 </a>
 *  @date 28.02.2023
 *  @copyright (C) 2023 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
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
 * License along with this library. If not, @see <http://www.gnu.org/licenses/>
 ******************************************************************************
 */
#include <mprintf.h>
#include <eb_console_helper.h>
#include <stdbool.h>
#include <scu_wr_time.h>
#include <scu_ddr3_lm32.h>

extern uint32_t __reset_count;


DDR3_T g_ddr3;

/*!----------------------------------------------------------------------------
 */
void wrTest( const unsigned int index, const unsigned int n )
{
   mprintf( "Writing %u 64-bit-words at index: %u to index: %u\n", n, index, index+n );
   volatile uint64_t startTime = getWrSysTime();
   for( unsigned int i = 0; i < n; i++ )
   {
      DDR3_PAYLOAD_T sendPl;
      for( unsigned int j = 0; j < ARRAY_SIZE( sendPl.ad16 ); j++ )
         sendPl.ad16[j] = i;
      ddr3write64( &g_ddr3, index + i, &sendPl );
   }
   volatile uint64_t stopTime = getWrSysTime() - startTime;
   mprintf( "Time: %u\n", ((uint32_t*)&stopTime)[1] );


   mprintf( "Reading %u 64-bit-words at index: %u to index: %u\n", n, index, index+n );
   startTime = getWrSysTime();
   for( unsigned int i = 0; i < n; i++ )
   {
      DDR3_PAYLOAD_T recPl;
      ddr3read64( &g_ddr3, &recPl, index + i );
      for( unsigned int j = 0; j < ARRAY_SIZE( recPl.ad16 ); j++ )
      {
         if( recPl.ad16[j] != i )
         {
            mprintf( ESC_ERROR "Error in: recPl.ad16[%u], index: %u, value: 0x%04X\n" ESC_NORMAL,
                     j, i, recPl.ad16[j] );
            return;
         }
      }
   }
   stopTime = getWrSysTime() - startTime;
   mprintf( "Time: %u\n", ((uint32_t*)&stopTime)[1] );

   mprintf( ESC_BOLD ESC_FG_GREEN "PASS\n\n" ESC_NORMAL );
}

/*!----------------------------------------------------------------------------
 */
void wrBurstTest( const unsigned int index, const unsigned int n )
{
   mprintf( "Writing %u 64-bit-words at index: %u to index: %u\n", n, index, index+n );
   volatile uint64_t startTime = getWrSysTime();
   for( unsigned int i = 0; i < n; i++ )
   {
      DDR3_PAYLOAD_T sendPl;
      for( unsigned int j = 0; j < ARRAY_SIZE( sendPl.ad16 ); j++ )
         sendPl.ad16[j] = i;
      ddr3write64( &g_ddr3, index + i, &sendPl );
   }
   volatile uint64_t stopTime = getWrSysTime() - startTime;
   mprintf( "Time: %u\n", ((uint32_t*)&stopTime)[1] );

   mprintf( "Burst-reading %u 64-bit-words at index: %u to index: %u\n", n, index, index+n );
   unsigned int l = n;
   unsigned int i = 0;
   unsigned int maxPollCount = 0;
   unsigned int parts = 0;
   startTime = getWrSysTime();
   while( l > 0 )
   {
      parts++;
      unsigned int partLen = min( l, (unsigned int)(DDR3_XFER_FIFO_SIZE-1) );
      l -= partLen;
      ddr3StartBurstTransfer( &g_ddr3, index + i, partLen );
      unsigned int pollCount = 0;
      while( (ddr3GetFifoStatus( &g_ddr3 ) & DDR3_FIFO_STATUS_MASK_EMPTY) != 0 )
      {
         pollCount++;
         if( pollCount >= 1000 )
         {
            mprintf( ESC_ERROR "Error FiFo timeout!\n" ESC_NORMAL );
            return;
         }
      }
      maxPollCount = max( maxPollCount, pollCount );
      for( unsigned int j = 0; j < partLen; j++, i++ )
      {
         DDR3_PAYLOAD_T recPl;
         ddr3PopFifo( &g_ddr3, &recPl );
         for( unsigned int k = 0; k < ARRAY_SIZE( recPl.ad16 ); k++ )
         {
            if( recPl.ad16[k] != i )
            {
               mprintf( ESC_ERROR "Error in: recPl.ad16[%u], index: %u, value: 0x%04X\n" ESC_NORMAL,
                        k, i, recPl.ad16[k] );
               return;
            }
         }
      }
   }
   stopTime = getWrSysTime() - startTime;
   mprintf( "Time: %u\n", ((uint32_t*)&stopTime)[1] );

   mprintf( ESC_BOLD ESC_FG_GREEN "PASS  " ESC_NORMAL
            "Poll-counter: %u; Parts: %u\n\n", maxPollCount, parts );
}

/*!----------------------------------------------------------------------------
 */
void main( void )
{
   mprintf( ESC_XY("1","1") ESC_CLR_SCR "DDR3 performance test: %u\n", __reset_count );
   if( ddr3init( &g_ddr3 ) < 0 )
   {
      mprintf( ESC_ERROR "Error by initializing DDR3 object!\n" ESC_NORMAL );
      while( true );
   }
   mprintf( "DDR3 IF1: 0x%08X\nDDR3 IF2: 0x%08X\n",
            g_ddr3.pTrModeBase, g_ddr3.pBurstModeBase );

   wrTest( 100, 10000 );
   wrBurstTest( 200, 10000 );
   mprintf( "End...\n" );
   while( true );
}
/*================================== EOF ====================================*/
