/*!
 *  @file access64_type.h
 *  @brief Access data type for 64-bit write and read accesses for DDR3 of
 *         SCU3 and SRAM of SCU4
 *  @note Header only
 *  @note This module is suitable for Linux and LM32.
 *
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
#ifndef _ACCESS64_TYPE_H
#define _ACCESS64_TYPE_H
#include <helper_macros.h>

#ifdef __cplusplus
extern "C" {
namespace Scu
{
#endif

/*!
 * @brief 64-bit payload base type of the smallest storing unit
 *        of the SCU-DDR3-RAM and therefore of the SCU4-SRAM as well.
 */
typedef union
{  /*!
    * @brief Full 64-bit access.
    */
   uint64_t  d64;

   /*!
    * @brief Indexed 64- bit access.
    */
   uint64_t  ad64[sizeof(uint64_t)/sizeof(uint64_t)];

   /*!
    * @brief Indexed 64-bit access in two 32-bit steps.
    */
   uint32_t  ad32[sizeof(uint64_t)/sizeof(uint32_t)];

   /*!
    * @brief Indexed 64-bit access in four 16-bit steps.
    */
   uint16_t  ad16[sizeof(uint64_t)/sizeof(uint16_t)];

   /*!
    * @brief Indexed 64-bit access in eight 8-bit steps.
    */
   uint8_t   ad8[sizeof(uint64_t)/sizeof(uint8_t)];
} ACCESS64_T;

STATIC_ASSERT( sizeof(ACCESS64_T) == sizeof(uint64_t) );

/*! ---------------------------------------------------------------------------
 * @brief Helper function accomplishes a pre-swapping or post-swapping for
 *        preparing or follow up a byte-swapping of the etherbone-library.
 */
STATIC inline unsigned int _swapIndex( const unsigned int i )
{
   return ((i % 2) == 0)? (i+1) : (i-1);
}

/*! ---------------------------------------------------------------------------
 * @brief Fills a 64-bit value by 16-bit values and makes a pre-swapping
 *        if this function is in a big-endian environment.
 * @param pPayload Pointer to the 64-bit payload object to fill.
 * @param value16 16-bit value.
 * @param i Index.
 */
STATIC inline ALWAYS_INLINE
void setPayload16( ACCESS64_T* pPayload, const uint16_t value16, const unsigned int i )
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
   pPayload->ad16[_swapIndex(i)] = value16;
#else
   pPayload->ad16[i] = value16;
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief Reads a 16-bit value from a 64-bit object and makes a follow up
 *        swapping if this function is in a big-endian environment.
 * @param pPayload Pointer to the 64-bit payload object to read.
 * @param i Index.
 * @return 16-bit value.
 */
STATIC inline ALWAYS_INLINE
uint16_t getPayload16( const ACCESS64_T* pPayload, const unsigned int i )
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
   return pPayload->ad16[_swapIndex(i)];
#else
   return pPayload->ad16[i];
#endif
}

#ifdef __cplusplus
} /* namespace Scu */
} /* extern "C" */
#endif

#endif /* ifndef _ACCESS64_TYPE_H */
/*================================== EOF ====================================*/
