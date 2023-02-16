/*!
 *  @file scu_ddr3_lm32.h
 *  @brief Interface routines for Double Data Rate (DDR3) RAM in SCU3
 *
 *  @see scu_ddr3_lm32.c
 *  @see scu_ddr3.h
 *  @see
 *  <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/MacroF%C3%BCr1GbitDDR3MT41J64M16LADesSCUCarrierboards">
 *     DDR3 VHDL Macro der SCU3 </a>
 *  @date 16.02.2023
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
#ifndef _SCU_DDR3_LM32_H
#define _SCU_DDR3_LM32_H
#include <scu_ddr3.h>

#ifdef CONFIG_RTOS
  #include <lm32Interrupts.h>
  #define ddr3Lock()   criticalSectionEnter()
  #define ddr3Unlock() criticalSectionExit()
#else
 /*
  * Dummy functions when FreeRTOS will not used.
  */
 #define ddr3Lock()
 #define ddr3Unlock()
#endif


/*! ---------------------------------------------------------------------------
 * @brief Initializing of DDR3 RAM
 * @param pThis Pointer to the DDR3 object
 * @retval 0 Initializing was successful
 * @retval <0 Error
 */
int ddr3init( register DDR3_T* pThis );

/*! ---------------------------------------------------------------------------
 * @brief Writes a 64-bit value in the DDR3 RAM
 * @see DDR3_PAYLOAD_T
 * @param pThis Pointer to the DDR3 object
 * @param index64 64 bit aligned index
 * @param pData Pointer to the 64 bit data to write.
 */
STATIC inline
void ddr3write64( register const  DDR3_T* pThis,
                           const unsigned int index64,
                           const DDR3_PAYLOAD_T* pData )
{
   DDR_ASSERT( pThis != NULL );
   DDR_ASSERT( pThis->pTrModeBase != DDR3_INVALID );
   DDR_ASSERT( index64 <= DDR3_MAX_INDEX64 );

   register const unsigned int index32 =
                  index64 * (sizeof(DDR3_PAYLOAD_T)/sizeof(uint32_t));
   ddr3Lock();
   /*
    * CAUTION: Don't change the order of the following both
    * code lines!
    */
   pThis->pTrModeBase[index32+1] = pData->ad32[1]; // DDR3 high word
   pThis->pTrModeBase[index32+0] = pData->ad32[0]; // DDR3 low word

   ddr3Unlock();
}

/*! ---------------------------------------------------------------------------
 * @brief Reads a 64-bit value
 * @see DDR3_PAYLOAD_T
 * @param pThis Pointer to the DDR3 object
 * @param index64 64 bit aligned index
 * @param pData Pointer to the 64-bit-target where the function should
 *              copy the data.
 */
STATIC inline
void ddr3read64( register const DDR3_T* pThis, DDR3_PAYLOAD_T* pData,
                          const unsigned int index64 )
{
   DDR_ASSERT( pThis != NULL );
   DDR_ASSERT( pThis->pTrModeBase != DDR3_INVALID );
   DDR_ASSERT( index64 <= DDR3_MAX_INDEX64 );

   register const unsigned int index32 =
                  index64 * (sizeof(DDR3_PAYLOAD_T)/sizeof(uint32_t));
   ddr3Lock();
   /*
    * CAUTION: Don't change the order of the following both
    * code lines!
    */
   pData->ad32[0] = pThis->pTrModeBase[index32+0]; // DDR3 low word
   pData->ad32[1] = pThis->pTrModeBase[index32+1]; // DDR3 high word

   ddr3Unlock();
}

#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
/*! ---------------------------------------------------------------------------
 * @brief Returns the DDR3-fofo -status;
 * @see DDR3_FIFO_STATUS_MASK_EMPTY
 * @see DDR3_FIFO_STATUS_MASK_INIT_DONE
 * @see DDR3_FIFO_STATUS_MASK_USED_WORDS
 * @param pThis Pointer to the DDR3 object
 * @return Currently fifo status;
 */
STATIC inline volatile
uint32_t ddr3GetFifoStatus( register const DDR3_T* pThis )
{
   DDR_ASSERT( pThis != NULL );
   DDR_ASSERT( pThis->pBurstModeBase != DDR3_INVALID );

   ddr3Lock();
   const uint32_t ret = pThis->pBurstModeBase[DDR3_FIFO_STATUS_OFFSET_ADDR];
   ddr3Unlock();

   return ret;
}

/*! ---------------------------------------------------------------------------
 * @brief Rremoves a 64-bit data word from the button of the fifo..
 * @see DDR3_FIFO_LOW_WORD_OFFSET_ADDR
 * @see DDR3_FIFO_HIGH_WORD_OFFSET_ADDR
 * @param pThis Pointer to the DDR3 object
 */
STATIC inline
void ddr3PopFifo( register const DDR3_T* pThis,
                           DDR3_PAYLOAD_T* pData )
{
   DDR_ASSERT( pThis != NULL );
   DDR_ASSERT( pThis->pBurstModeBase != DDR3_INVALID );

   ddr3Lock();
   /*
    * CAUTION: Don't change the order of the following both
    * code lines!
    */
   pData->ad32[0] = pThis->pBurstModeBase[DDR3_FIFO_LOW_WORD_OFFSET_ADDR];
   pData->ad32[1] = pThis->pBurstModeBase[DDR3_FIFO_HIGH_WORD_OFFSET_ADDR];

   ddr3Unlock();
}

/*! ---------------------------------------------------------------------------
 * @brief Starts the burst transfer.
 * @param pThis Pointer to the DDR3 object
 * @param burstStartAddr 64 bit oriented start address in fifo
 * @param burstLen 64 bit oriented data-length the value
 *                 has to be between [1..DDR3_XFER_FIFO_SIZE]
 * @see DDR3_XFER_FIFO_SIZE
 */
STATIC inline
void ddr3StartBurstTransfer( register const DDR3_T* pThis,
                                      const unsigned int burstStartAddr,
                                      const unsigned int burstLen )
{
   DDR_ASSERT( pThis != NULL );
   DDR_ASSERT( pThis->pTrModeBase != DDR3_INVALID );
   DDR_ASSERT( burstLen <= DDR3_XFER_FIFO_SIZE );

   ddr3Lock();
   /*
    * CAUTION: Don't change the order of the following both
    * code lines!
    */
   pThis->pTrModeBase[DDR3_BURST_START_ADDR_REG_OFFSET] = burstStartAddr;
   pThis->pTrModeBase[DDR3_BURST_XFER_CNT_REG_OFFSET]   = burstLen;

   ddr3Unlock();
}

/*! ---------------------------------------------------------------------------
 * @brief Pointer type of the optional polling-function for
 *        the argument "poll" of the function ddr3FlushFiFo.
 *
 * This callback function can be used e.g.: to implement a timeout function
 * or in the case when using a OS to invoke a scheduling function.
 * @see ddr3FlushFiFo
 * @param pThis Pointer to the DDR3 object
 * @param count Number of subsequent calls of this function. E.g.: The
 *              condition (count == 0) can be used to initialize
 *              a timer.
 * @retval >0   Polling loop will terminated despite of the wrong FiFo-status.
 * @retval ==0  Polling will continue till the FiFo contains at least one
 *              data-word.
 * @retval <0   Function ddr3FlushFiFo will terminated immediately with the
 *              return-value of this function.
 */
typedef int (*DDR3_POLL_FT)( register const DDR3_T* pThis UNUSED,
                             unsigned int count UNUSED );

/*! ---------------------------------------------------------------------------
 * @brief Flushes the DDR3 Fifo and writes it's content in argument pTarget
 * @param pThis Pointer to the DDR3 object
 * @param start Start-index (64-byte oriented) in fifo.
 * @param word64len Number of 64 bit words to read.
 * @param pTarget Target address. The memory-size where this address points
 *                has to be at least sizeof(uint64_t) resp. 8 bytes.
 * @param poll Optional pointer to a polling function. If not used then
 *             this parameter has to be set to NULL.
 *             @see DDR3_POLL_FT
 * @return Return status of poll-function if used.
 */
int ddr3FlushFiFo( register const DDR3_T* pThis, unsigned int start,
                   unsigned int word64len, DDR3_PAYLOAD_T* pTarget,
                   DDR3_POLL_FT poll
                 );

#endif /* ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS */
#endif
/*================================== EOF ====================================*/
