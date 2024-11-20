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
#include <sdb_lm32.h>
#include <dbg.h>
#include <scu_lm32_macros.h>
#include <lm32Interrupts.h>

#ifndef CONFIG_SCU_USE_DDR3
   #error CONFIG_SCU_USE_DDR3 has to be defined!
#endif

/*!
 * @brief Module internal handle for DDR3 accesses.
 */
DDR3_T mg_oDdr3 =
{
   .pTrModeBase     = DDR3_INVALID
#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
   ,.pBurstModeBase = DDR3_INVALID
#endif
};

#if defined( CONFIG_RTOS ) && defined( CONFIG_DDR3_MULTIPLE_USE )
 #if 1
  #define ddr3Lock()   wbZycleEnter()
  #define ddr3Unlock() wbZycleExit()
 #else
  STATIC inline ALWAYS_INLINE void ddr3Lock( void )
  {
     osMutexLock( &mg_oDdr3.oMutex );
    //!! wbZycleEnterBase();
  }

  STATIC inline ALWAYS_INLINE void ddr3Unlock( void )
  {
    //!! wbZycleExitBase();
     osMutexUnlock( &mg_oDdr3.oMutex );
  }
 #endif
#else
 /*
  * Dummy functions when FreeRTOS will not used.
  */
 #define ddr3Lock()    wbZycleEnter()
 #define ddr3Unlock()  wbZycleExit()
#endif


/*! ---------------------------------------------------------------------------
 * @see scu_ddr3_lm32.h
 */
int ddr3init( void )
{
   DDR_ASSERT( mg_oDdr3.pTrModeBase == DDR3_INVALID );

#if defined( CONFIG_RTOS ) && defined( CONFIG_DDR3_MULTIPLE_USE )
   osMutexInit( &mg_oDdr3.oMutex );
#endif

   mg_oDdr3.pTrModeBase = (DDR3_ADDR_T)find_device_adr( GSI, WB_DDR3_if1 );
   if( mg_oDdr3.pTrModeBase == (uint32_t*)ERROR_NOT_FOUND )
   {
      mg_oDdr3.pTrModeBase = DDR3_INVALID;
      DBPRINT1( "DBG: ERROR: DDR3: Can't find address of WB_DDR3_if1 !\n" );
      return -1;
   }
#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
   mg_oDdr3.pBurstModeBase = (DDR3_ADDR_T)find_device_adr( GSI, WB_DDR3_if2 );
   if( mg_oDdr3.pBurstModeBase == (uint32_t*)ERROR_NOT_FOUND )
   {
      mg_oDdr3.pBurstModeBase = DDR3_INVALID;
      mg_oDdr3.pTrModeBase    = DDR3_INVALID;
      DBPRINT1( "DBG: ERROR: DDR3: Can't find address of WB_DDR3_if2 !\n" );
      return -1;
   }

   /*
    * Making the FiFo empty if not empty;
    */
   unsigned int size = ddr3GetFifoStatus() & DDR3_FIFO_STATUS_MASK_USED_WORDS;
   for( unsigned int i = 0; i < size; i++ )
   {
      uint32_t dummy = mg_oDdr3.pBurstModeBase[DDR3_FIFO_LOW_WORD_OFFSET_ADDR];
      dummy = mg_oDdr3.pBurstModeBase[DDR3_FIFO_HIGH_WORD_OFFSET_ADDR];
      /*
       * Suppresses the not-used warning.
       */
      (void) dummy;
   }
#endif
   return 0;
}

/*! ---------------------------------------------------------------------------
 * @see scu_ddr3_lm32.h
 */
DDR3_T* ddr3GetObj( void )
{
   return &mg_oDdr3;
}

/*! ---------------------------------------------------------------------------
 * @see scu_ddr3_lm32.h
 */
void ddr3write64( const unsigned int index64, const DDR3_PAYLOAD_T* pData )
{
   DDR_ASSERT( mg_oDdr3.pTrModeBase != DDR3_INVALID );
   DDR_ASSERT( index64 <= DDR3_MAX_INDEX64 );

   const unsigned int index32 =
                  index64 * (sizeof(DDR3_PAYLOAD_T)/sizeof(uint32_t));
   ddr3Lock();
   /*
    * CAUTION: Don't change the order of the following
    * code lines!
    */
   mg_oDdr3.pTrModeBase[index32+1] = pData->ad32[1]; // DDR3 high word first!
   BARRIER();
   mg_oDdr3.pTrModeBase[index32+0] = pData->ad32[0]; // DDR3 low word second!
   BARRIER();

   ddr3Unlock();
}

/*! ---------------------------------------------------------------------------
 * @see scu_ddr3_lm32.h
 */
void ddr3read64( DDR3_PAYLOAD_T* pData, const unsigned int index64 )
{
   DDR_ASSERT( mg_oDdr3.pTrModeBase != DDR3_INVALID );
   DDR_ASSERT( index64 <= DDR3_MAX_INDEX64 );

   const unsigned int index32 =
                  index64 * (sizeof(DDR3_PAYLOAD_T)/sizeof(uint32_t));
   ddr3Lock();
   /*
    * CAUTION: Don't change the order of the following
    * code lines!
    */
   pData->ad32[0] = mg_oDdr3.pTrModeBase[index32+0]; // DDR3 low word first!
   BARRIER();
   pData->ad32[1] = mg_oDdr3.pTrModeBase[index32+1]; // DDR3 high word second!
   BARRIER();

   ddr3Unlock();
}

#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS

/*! ---------------------------------------------------------------------------
 * @see scu_ddr3_lm32.h
 */
uint32_t ddr3GetFifoStatus( void )
{
   DDR_ASSERT( mg_oDdr3.pBurstModeBase != DDR3_INVALID );

   ddr3Lock();
   const uint32_t ret = mg_oDdr3.pBurstModeBase[DDR3_FIFO_STATUS_OFFSET_ADDR];
   ddr3Unlock();

   return ret;
}

/*! ---------------------------------------------------------------------------
 * @see scu_ddr3_lm32.h
 */
void ddr3PopFifo( DDR3_PAYLOAD_T* pData )
{
   DDR_ASSERT( mg_oDdr3.pBurstModeBase != DDR3_INVALID );

   ddr3Lock();
   /*
    * CAUTION: Don't change the order of the following
    * code lines!
    */
   pData->ad32[0] = mg_oDdr3.pBurstModeBase[DDR3_FIFO_LOW_WORD_OFFSET_ADDR];
   BARRIER();
   pData->ad32[1] = mg_oDdr3.pBurstModeBase[DDR3_FIFO_HIGH_WORD_OFFSET_ADDR];
   BARRIER();

   ddr3Unlock();
}

/*! ---------------------------------------------------------------------------
 * @see scu_ddr3_lm32.h
 */
void ddr3StartBurstTransfer( const unsigned int burstStartAddr,
                             const unsigned int burstLen )
{
   DDR_ASSERT( mg_oDdr3.pTrModeBase != DDR3_INVALID );
   DDR_ASSERT( burstLen <= DDR3_XFER_FIFO_SIZE );

   ddr3Lock();
   /*
    * CAUTION: Don't change the order of the following
    * code lines!
    */
   mg_oDdr3.pTrModeBase[DDR3_BURST_START_ADDR_REG_OFFSET] = burstStartAddr;
   BARRIER();
   mg_oDdr3.pTrModeBase[DDR3_BURST_XFER_CNT_REG_OFFSET]   = burstLen;
   BARRIER();

   ddr3Unlock();
}

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
int ddr3FlushFiFo( unsigned int start, unsigned int word64len,
                   DDR3_PAYLOAD_T* pTarget, DDR3_POLL_FT poll )
{
   int pollRet = 0;
   unsigned int targetIndex = 0;
   DDR_ASSERT( pTarget != NULL );
   DDR_ASSERT( (word64len + start) <= DDR3_MAX_INDEX64 );
   while( word64len > 0 )
   {
      unsigned int blkLen = min( word64len, (unsigned int)(DDR3_XFER_FIFO_SIZE * sizeof(uint32_t)/sizeof(uint64_t)-1) );
      DBPRINT2( "DBG: blkLen: %d\n", blkLen );
      ddr3StartBurstTransfer( start, blkLen );

      unsigned int pollCount = 0;
      while( (ddr3GetFifoStatus() & DDR3_FIFO_STATUS_MASK_EMPTY) != 0 )
      {
         if( poll == NULL )
            continue;
         pollRet = poll( &mg_oDdr3, pollCount );
         if( pollRet < 0 )
            return pollRet;
         if( pollRet > 0 )
            break;
         pollCount++;
      }

      for( unsigned int i = 0; i < blkLen; i++ )
      {
         ddr3PopFifo( &pTarget[targetIndex++] );
      }

      start     += blkLen;
      word64len -= blkLen;
   }
   DBPRINT2( "DBG: FiFo-status final: 0x%08x\n", ddr3GetFifoStatus() );
   return pollRet;
}

/*! ---------------------------------------------------------------------------
 * @see scu_ddr3_lm32.h
 */
int ddr3ReadBurst( unsigned int index64,
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
      ddr3StartBurstTransfer( index64 + i, partLen );
      while( ((status = ddr3GetFifoStatus()) & DDR3_FIFO_STATUS_MASK_EMPTY) != 0 )
      {
         pollCount++;
         if( pollCount > 1000 )
            return -1;
      }
      for( unsigned int j = 0; j < partLen; j++, i++ )
      {
         DDR3_PAYLOAD_T recPl;
         ddr3PopFifo( &recPl );
         onData( &recPl, i, pPrivate );
      }
   }
   return status & DDR3_FIFO_STATUS_MASK_USED_WORDS;
}

#endif /* ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS */

/*================================== EOF ====================================*/
