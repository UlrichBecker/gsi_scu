/*!
 *  @file scu_ddr3_lm32.c
 *  @brief Interface routines for Double Data Rate (DDR3) RAM in SCU3
 *
 *  @see scu_ddr3_lm32.h
 *  @see scu_ddr3.h
 *  @see
 *  <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/MacroF%C3%BCr1GbitDDR3MT41J64M16LADesSCUCarrierboards">
 *     DDR3 VHDL Macro der SCU3 </a>
 *  @date 01.02.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */
#include <scu_ddr3_lm32.h>
#include <mini_sdb.h>
#include <dbg.h>

/*! ---------------------------------------------------------------------------
 * @see scu_ddr3_lm32.h
 */
int ddr3init( register DDR3_T* pThis  )
{
   DDR_ASSERT( pThis != NULL );
#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
   pThis->pBurstModeBase = DDR3_INVALID;
#endif
   pThis->pTrModeBase = find_device_adr( GSI, WB_DDR3_if1 );
   if( pThis->pTrModeBase == (uint32_t*)ERROR_NOT_FOUND )
   {
      pThis->pTrModeBase = DDR3_INVALID;
      DBPRINT1( "DBG: ERROR: DDR3: Can't find address of WB_DDR3_if1 !\n" );
      return -1;
   }
#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
   pThis->pBurstModeBase = find_device_adr( GSI, WB_DDR3_if2 );
   if( pThis->pBurstModeBase == (uint32_t*)ERROR_NOT_FOUND )
   {
      pThis->pBurstModeBase = DDR3_INVALID;
      pThis->pTrModeBase    = DDR3_INVALID;
      DBPRINT1( "DBG: ERROR: DDR3: Can't find address of WB_DDR3_if2 !\n" );
      return -1;
   }

   /*
    * Making the FiFo empty if not empty;
    */
   unsigned int size = ddr3GetFifoStatus( pThis ) & DDR3_FIFO_STATUS_MASK_USED_WORDS;
   for( unsigned int i = 0; i < size; i++ )
   {
      uint32_t dummy = pThis->pBurstModeBase[DDR3_FIFO_LOW_WORD_OFFSET_ADDR];
      dummy = pThis->pBurstModeBase[DDR3_FIFO_HIGH_WORD_OFFSET_ADDR];
      /*
       * Suppresses the not-used warning.
       */
      (void) dummy;
   }
#endif
   return 0;
}

#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS

static inline
DDR3_RETURN_T _ddr3PopFifo( register const DDR3_T* pThis,
                           DDR3_PAYLOAD_T* pData )
{
   DDR_ASSERT( pThis != NULL );
   DDR_ASSERT( pThis->pBurstModeBase != DDR3_INVALID );

//TODO

}

/*! ---------------------------------------------------------------------------
 * @see scu_ddr3_lm32.h
 */
#define CONFIG_EB_BLOCK_READING
int ddr3FlushFiFo( register const DDR3_T* pThis, unsigned int start,
                   unsigned int word64len, DDR3_PAYLOAD_T* pTarget,
                   DDR3_POLL_FT poll )
{
   int pollRet = 0;
   unsigned int targetIndex = 0;
   DDR_ASSERT( pTarget != NULL );
   DDR_ASSERT( (word64len + start) <= DDR3_MAX_INDEX64 );
   while( word64len > 0 )
   {
      unsigned int blkLen = min( word64len, (unsigned int)(DDR3_XFER_FIFO_SIZE * sizeof(uint32_t)/sizeof(uint64_t)-1) );
      DBPRINT2( "DBG: blkLen: %d\n", blkLen );
      ddr3StartBurstTransfer( pThis, start, blkLen );

      unsigned int pollCount = 0;
      while( (ddr3GetFifoStatus( pThis ) & DDR3_FIFO_STATUS_MASK_EMPTY) != 0 )
      {
         if( poll == NULL )
            continue;
         pollRet = poll( pThis, pollCount );
         if( pollRet < 0 )
            return pollRet;
         if( pollRet > 0 )
            break;
         pollCount++;
      }

      for( unsigned int i = 0; i < blkLen; i++ )
      {
         ddr3PopFifo( pThis, &pTarget[targetIndex++] );
      }

      start     += blkLen;
      word64len -= blkLen;
   }
   DBPRINT2( "DBG: FiFo-status final: 0x%08x\n", ddr3GetFifoStatus( pThis ) );
   return pollRet;
}

/*! ---------------------------------------------------------------------------
 * @see scu_ddr3_lm32.h
 */
int ddr3ReadBurst( DDR3_T* pThis, unsigned int index64,
                                  unsigned int len64,
                                  DDR3_ON_BURST_FT onData,
                                  void* pPrivate
                 )
{
   DDR_ASSERT( onData != NULL );
   unsigned int pollCount = 0;
   unsigned int i = 0;
   int status = 0;
   while( len64 > 0 )
   {
      unsigned int partLen = min( len64, (unsigned int)(DDR3_XFER_FIFO_SIZE * sizeof(uint32_t)/sizeof(uint64_t)-1) );
      len64 -= partLen;
      ddr3StartBurstTransfer( pThis, index64 + i, partLen );
      while( ((status = ddr3GetFifoStatus( pThis )) & DDR3_FIFO_STATUS_MASK_EMPTY) != 0 )
      {
         pollCount++;
         if( pollCount > 1000 )
            return -1;
      }
      for( unsigned int j = 0; j < partLen; j++, i++ )
      {
         DDR3_PAYLOAD_T recPl;
         ddr3PopFifo( pThis, &recPl );
         onData( &recPl, i, pPrivate );
      }
   }
   return status & DDR3_FIFO_STATUS_MASK_USED_WORDS;
}

#endif /* ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS */

/*================================== EOF ====================================*/
