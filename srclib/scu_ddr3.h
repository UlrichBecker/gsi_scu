/*!
 *  @file scu_ddr3.h
 *  @brief Interface routines for Double Data Rate (DDR3) RAM in SCU3
 *  @note Header only
 *  @note This module is suitable for Linux and LM32.
 *
 *  @see scu_ddr3.c
 *  @see
 *  <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/MacroF%C3%BCr1GbitDDR3MT41J64M16LADesSCUCarrierboards">
 *     DDR3 VHDL Macro der SCU3 </a>
 *  @date 01.02.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#ifndef _SCU_DDR3_H
#define _SCU_DDR3_H
#include <stdint.h>
#include <stdbool.h>
#include <helper_macros.h>

#include <access64_type.h>

#ifdef __cplusplus
extern "C" {
namespace Scu
{
#endif
/*!
 * @defgroup SCU_DDR3
 * @ingroup SCU_RAM_BUFFER
 * @brief DDR3 RAM of SCU3
 * @{
 */

#ifdef CONFIG_DDR_PEDANTIC_CHECK
   /* CAUTION:
    * Assert-macros could be expensive in memory consuming and the
    * latency time can increase as well!
    * Especially in embedded systems with small resources.
    * Therefore use them for bug-fixing or developing purposes only!
    */
   #include <scu_assert.h>
   #define DDR_ASSERT SCU_ASSERT
#else
   #define DDR_ASSERT(__e) ((void)0)
#endif

/*!
 * @brief Maximum size of DDR3 RAM in bytes (1GiBit = GB/8) (134 MB)
 *        (134217728 B)
 */
#define DDR3_MAX_SIZE 0x8000000UL

/*!
 * @brief Maximum usable DDR3 address.
 */
#define DDR3_MAX_ADDR 0x7FFFFECUL

/*!
 * @brief Maximum of 64 bit oriented access-index.
 * @see ddr3write64 ddr3read64
 */
#define DDR3_MAX_INDEX64 (DDR3_MAX_ADDR / sizeof(DDR3_PAYLOAD_T))

/*!
 * @brief 32 bit oriented offset address of burst mode start-address register
 * @see ddr3StartBurstTransfer
 */
#define DDR3_BURST_START_ADDR_REG_OFFSET 0x1FFFFFD

/*!
 * @brief 32 bit oriented offset address of Xfer_Cnt register.
 * Maximum value is 512 64-bit words.
 * @see ddr3StartBurstTransfer
 */
#define DDR3_BURST_XFER_CNT_REG_OFFSET   0x1FFFFFE

/*!
 * @brief Maximum size of DDR3 Xfer Fifo in 64-bit words
 */
#define DDR3_XFER_FIFO_SIZE  256

/*!
 * @brief 32 bit oriented offset address of fifo status
 */
#define DDR3_FIFO_STATUS_OFFSET_ADDR 0x0E

/*!
 * @see ddr3GetFifoStatus
 */
#define DDR3_FIFO_STATUS_MASK_EMPTY       (1 << 31)

/*!
 * @see ddr3GetFifoStatus
 */
#define DDR3_FIFO_STATUS_MASK_INIT_DONE   (1 << 30)

/*!
 * @see ddr3GetFifoStatus
 */
#define DDR3_FIFO_STATUS_MASK_USED_WORDS  0xFF

STATIC_ASSERT( DDR3_FIFO_STATUS_MASK_USED_WORDS == DDR3_XFER_FIFO_SIZE -1 );

/*!
 * @brief 32 bit oriented offset address of the low data fifo-register
 * @see ddr3PopFifo
 */
#define DDR3_FIFO_LOW_WORD_OFFSET_ADDR  0x0C

/*!
 * @brief 32 bit oriented offset address of the high data fifo-register.
 * @see ddr3PopFifo
 */
#define DDR3_FIFO_HIGH_WORD_OFFSET_ADDR 0x0D

#ifdef __lm32__
   typedef uint32_t* volatile DDR3_ADDR_T;
   typedef void               DDR3_RETURN_T;
   #define DDR3_INVALID       NULL
   #define LM32_VOLATILE      volatile
#else
   typedef uint32_t           DDR3_ADDR_T;
   typedef void               DDR3_RETURN_T;
   #define DDR3_INVALID       0
   #define LM32_VOLATILE
#endif


/*!
 * @brief Payload type for DDR3-RAM.
 */
typedef ACCESS64_T DDR3_PAYLOAD_T;


/*! --------------------------------------------------------------------------
 */
STATIC inline
void ddr3SetPayload16( DDR3_PAYLOAD_T* pPl, const uint16_t d,
                       const unsigned int i )
{
   DDR_ASSERT( i <= ARRAY_SIZE( pPl->ad16 ) );
   setPayload16( pPl, d, i );
}

/*! --------------------------------------------------------------------------
 */
STATIC inline
uint16_t ddr3GetPayload16( DDR3_PAYLOAD_T* pPl, const unsigned int i )
{
   DDR_ASSERT( i <= ARRAY_SIZE( pPl->ad16 ) );
   return getPayload16( pPl, i );
}

/*! @} */ //End of group  SCU_DDR3
#ifdef __cplusplus
} /* namespace Scu */
} /* extern "C" */
#endif

#endif /* ifndef _SCU_DDR3_H */
/*================================== EOF ====================================*/
