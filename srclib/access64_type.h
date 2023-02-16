/*!
 *  @file access64_type.h
 *  @brief SCU3
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

/*!
 * @brief 64-bit payload base type of the smallest storing unit
 *        of the SCU- DDR3- RAM.
 */
typedef union
{
   uint64_t  d64;
   uint32_t  ad32[sizeof(uint64_t)/sizeof(uint32_t)];
   uint16_t  ad16[sizeof(uint64_t)/sizeof(uint16_t)];
   uint8_t   ad8[sizeof(uint64_t)/sizeof(uint8_t)];
} DDR3_PAYLOAD_T;

STATIC_ASSERT( sizeof(DDR3_PAYLOAD_T) == sizeof(uint64_t) );


#endif /* ifndef _ACCESS64_TYPE_H */
/*================================== EOF ====================================*/
