/*!
 *
 * @brief     Some additional helpful macro definitions specific for SCU LM32
 *
 *            Extension of file helper_macros.h
 *
 * @note Header only!
 *
 * @file      scu_lm32_macros.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      04.01.2019
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
#ifndef _SCU_LM32_MACROS_H
#define _SCU_LM32_MACROS_H
#if !defined(__lm32__) && !defined(__CPPCHECK__)
   #error This header file is specific for LM32 targets in GSI-SCU only!
#endif

#include <helper_macros.h>

/*! ---------------------------------------------------------------------------
 * @ingroup HELPER_MACROS
 * @brief Modifier macro forces data variables declared by this macro in
 *        to the shared memory area.
 *
 * @note For a clean design this macro should only appear once in the project!
 */
#define SHARED __attribute__((section(".shared")))

/*! ---------------------------------------------------------------------------
 * @ingroup HELPER_MACROS
 * @brief Macro makes a memory mapped access on a hardware register.
 * @param T Data-type of the concerning register (alignment-type)
 * @param p Memory base address of register
 * @param n Address-offset in alignment-units
 */
#define __REGX_ACCESS( T, p, n )    ((T volatile *)(p))[n]

/*! ---------------------------------------------------------------------------
 * @ingroup HELPER_MACROS
 * @ingroup PATCH
 * @brief Base macro for accessing to wishbone devices via members of device
 *        objects.
 * @note This is a patch! For still unknown reasons it's not possible making a
 *       direct access via object member.\n
 *       Doesn't matter which compiler version will used. (4.5.3 or 9.2.0)
 * @todo Find the cause why this patch is necessary and remove it
 *       if possible.
 * @param TO Object type.
 * @param TA Alignment type.
 * @param p Pointer to the concerning object.
 * @param m Name of member variable.
 */
#define __WB_ACCESS( TO, TA, p, m ) \
    __REGX_ACCESS( TA, p, (offsetof( TO, m ) / sizeof(TA)) )

/*! ---------------------------------------------------------------------------
 * @ingroup HELPER_MACROS
 * @brief Writes a 32-bit value in a memory mapped hardware register.
 * @param p Memory base address of register
 * @param n Address-offset in 32-bit-units (index)
 * @param v 32-bit value to write
 */
#define SET_REG32( p, n, v ) __REGX_ACCESS( uint32_t, p, n ) = (v)

/*! ---------------------------------------------------------------------------
 * @ingroup HELPER_MACROS
 * @brief Writes a 16-bit value in a memory mapped hardware register.
 * @param p Memory base address of register
 * @param n Address-offset in 32-bit-units (index)
 * @param v 16-bit value to write
 */
#define SET_REG16( p, n, v ) __REGX_ACCESS( uint16_t, p, n ) = (v)

/*! ---------------------------------------------------------------------------
 * @ingroup HELPER_MACROS
 * @brief Writes a 8-bit value in a memory mapped hardware register.
 * @param p Memory base address of register
 * @param n Address-offset in 8-bit-units (index)
 * @param v 8-bit value to write
 */
#define SET_REG8( p, n, v ) __REGX_ACCESS( uint8_t, p, n ) = (v)


/*! ---------------------------------------------------------------------------
 * @brief Performs no operation! Wasting of one clock cycle.
 */
#define NOP() asm volatile ( "nop" )

/*! ---------------------------------------------------------------------------
 * @brief Prevents a memory reordering depending at the adjusted optimization
 *        level.
 * 
 * That means that any reads or writes of memory will not be moved across this
 * memory barrier, nor will the results of such reads or writes be cached
 * across the barrier.  
 */
#define BARRIER() asm volatile ( "" ::: "memory" )

#endif /* ifndef _SCU_LM32_MACROS_H */
/*================================== EOF ====================================*/
