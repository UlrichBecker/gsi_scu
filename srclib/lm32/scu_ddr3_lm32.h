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
#ifndef __lm32__
  #error Module is for target Lattice Micro 32 (LM32) only!
#endif

#include <scu_ddr3.h>


/*!
 * @brief Object type of SCU internal DDR3 RAM.
 */
typedef struct
{
   /*! @brief WB Base-address of transparent mode */
   DDR3_ADDR_T pTrModeBase;
#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
   /*! @brief WB Base-address of burst mode */
   DDR3_ADDR_T pBurstModeBase;
#endif
} DDR3_T;

/*! ---------------------------------------------------------------------------
 * @brief Initializing of DDR3 RAM
 * @retval 0 Initializing was successful
 * @retval <0 Error
 */
int ddr3init( void );

/*! ---------------------------------------------------------------------------
 * @brief Returns the pointer to the DDR3 object.
 * @note For debug purposes only. 
 */
DDR3_T* ddr3GetObj( void );

/*! ---------------------------------------------------------------------------
 * @brief Writes a 64-bit value in the DDR3 RAM
 * @see DDR3_PAYLOAD_T
 * @param index64 64 bit aligned index
 * @param pData Pointer to the 64 bit data to write.
 */
void ddr3write64( const unsigned int index64,
                  const DDR3_PAYLOAD_T* pData );

/*! ---------------------------------------------------------------------------
 * @brief Reads a 64-bit value
 * @see DDR3_PAYLOAD_T
 * @param index64 64 bit aligned index
 * @param pData Pointer to the 64-bit-target where the function should
 *              copy the data.
 */
void ddr3read64( DDR3_PAYLOAD_T* pData,
                 const unsigned int index64 );

#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
/*! ---------------------------------------------------------------------------
 * @brief Returns the DDR3-fofo -status;
 * @see DDR3_FIFO_STATUS_MASK_EMPTY
 * @see DDR3_FIFO_STATUS_MASK_INIT_DONE
 * @see DDR3_FIFO_STATUS_MASK_USED_WORDS
 * @return Currently fifo status;
 */
uint32_t ddr3GetFifoStatus( void );

/*! ---------------------------------------------------------------------------
 * @brief Rremoves a 64-bit data word from the button of the fifo..
 * @see DDR3_FIFO_LOW_WORD_OFFSET_ADDR
 * @see DDR3_FIFO_HIGH_WORD_OFFSET_ADDR
 */
void ddr3PopFifo( DDR3_PAYLOAD_T* pData );

/*! ---------------------------------------------------------------------------
 * @brief Starts the burst transfer.
 * @param pThis Pointer to the DDR3 object
 * @param burstStartAddr 64 bit oriented start address in fifo
 * @param burstLen 64 bit oriented data-length the value
 *                 has to be between [1..DDR3_XFER_FIFO_SIZE]
 * @see DDR3_XFER_FIFO_SIZE
 */
void ddr3StartBurstTransfer( const unsigned int burstStartAddr,
                             const unsigned int burstLen );

/*! ---------------------------------------------------------------------------
 * @brief Pointer type of the optional polling-function for
 *        the argument "poll" of the function ddr3FlushFiFo.
 *
 * This callback function can be used e.g.: to implement a timeout function
 * or in the case when using a OS to invoke a scheduling function.
 * @see ddr3FlushFiFo
 * @param count Number of subsequent calls of this function. E.g.: The
 *              condition (count == 0) can be used to initialize
 *              a timer.
 * @retval >0   Polling loop will terminated despite of the wrong FiFo-status.
 * @retval ==0  Polling will continue till the FiFo contains at least one
 *              data-word.
 * @retval <0   Function ddr3FlushFiFo will terminated immediately with the
 *              return-value of this function.
 */
typedef int (*DDR3_POLL_FT)( const DDR3_T* pThis UNUSED,
                             unsigned int count UNUSED );

/*! ---------------------------------------------------------------------------
 * @brief Flushes the DDR3 Fifo and writes it's content in argument pTarget
 * @param start Start-index (64-byte oriented) in fifo.
 * @param word64len Number of 64 bit words to read.
 * @param pTarget Target address. The memory-size where this address points
 *                has to be at least sizeof(uint64_t) resp. 8 bytes.
 * @param poll Optional pointer to a polling function. If not used then
 *             this parameter has to be set to NULL.
 *             @see DDR3_POLL_FT
 * @return Return status of poll-function if used.
 */
int ddr3FlushFiFo( unsigned int start, unsigned int word64len, 
                   DDR3_PAYLOAD_T* pTarget, DDR3_POLL_FT poll );

/*! --------------------------------------------------------------------------
 * @brief Callback function becomes invoked within function ddr3ReadBurst
 *        by every received 64-bit data word.
 * @param pPl Pointer to received 64 data word.
 * @param index Index of received data word.
 * @param pPrivate Pointer to optional private data.
 */
typedef void (*DDR3_ON_BURST_FT)( DDR3_PAYLOAD_T* pPl,
                                  unsigned int index,
                                  void* pPrivate
                                );

/*! --------------------------------------------------------------------------
 * @brief Reads the DDR3-memory in burst mode by using a callback function
 *        for each received 64-bit data word.
 * @param index64 Start offset in 64-bit words,
 * @param len64 Expected number of 64 -bit words to read.
 * @param onData Pointer to callback function which becomes invoked by each
 *               received 64-bit data word.
 * @param pPrivate Pointer for private data of callback function onData.
 * @retval <0  Error
 * @retval ==0 Ok
 */
int ddr3ReadBurst( unsigned int index64,
                   unsigned int len64,
                   DDR3_ON_BURST_FT onData,
                   void* pPrivate
                 );

#endif /* ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS */
#endif
/*================================== EOF ====================================*/
