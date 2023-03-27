/*!
 * @file scu_sram.h
 * @brief Common interface for handling SRAM for LM32 and Linux
 * @note Header only
 * @note This module is suitable for Linux and LM32.
 *
 * @date 27.03.2023
 * @copyright (C) 2023 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
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
#ifndef _SCU_SRAM_H
#define _SCU_SRAM_H
#include <stdint.h>

#ifdef CONFIG_SRAM_PEDANTIC_CHECK
   /* CAUTION:
    * Assert-macros could be expensive in memory consuming and the
    * latency time can increase as well!
    * Especially in embedded systems with small resources.
    * Therefore use them for bug-fixing or developing purposes only!
    */
   #include <scu_assert.h>
   #define SRAM_ASSERT SCU_ASSERT
#else
   #define SRAM_ASSERT(__e) ((void)0)
#endif


#ifdef __cplusplus
extern "C" {
namespace Scu
{
#endif

const unsigned int _32MB_IN_BYTE = 33554432;
const unsigned int SRAM_MAX_INDEX64 = _32MB_IN_BYTE / sizeof(uint64_t) - 1;


#ifdef __cplusplus
} /* namespace Scu */
} /* extern "C" */
#endif

#endif /* ifndef _SCU_SRAM_H */
/*================================== EOF ====================================*/
