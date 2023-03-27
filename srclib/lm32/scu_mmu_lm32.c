/*!
 * @file scu_mmu_lm32.c
 * @brief LM32 part of Memory Management Unit of SCU
 * 
 * Administration of the shared memory (for SCU3 using DDR3) between 
 * Linux host and LM32 application.
 * 
 * @note This source code is suitable for LM32 ony.
 * 
 * @see       scu_mmu_lm32.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      31.03.2022
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#include <scu_mmu_lm32.h>

#ifdef CONFIG_SCU_USE_DDR3 /***** DDR3-ACCESS *********/
#include <scu_ddr3_lm32.h>

/*! ---------------------------------------------------------------------------
 * @see scu_mmu_lm32.h
 */
MMU_STATUS_T mmuInit( void )
{
   return (ddr3init() == 0)? OK : MEM_NOT_PRESENT;
}

/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
void mmuRead( MMU_ADDR_T index, RAM_PAYLOAD_T* pItem, size_t len )
{
   for( size_t i = 0; i < len; i++, index++ )
   {
      ddr3read64( &pItem[i], index );
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
void mmuWrite( MMU_ADDR_T index, const RAM_PAYLOAD_T* pItem, size_t len )
{
   for( size_t i = 0; i < len; i++, index++ )
   {
      ddr3write64( index, &pItem[i] );
   }
}

#else /******* SRAM- ACCESS *********/
#include <scu_sram_lm32.h>
/*! ---------------------------------------------------------------------------
 * @see scu_mmu_lm32.h
 */
MMU_STATUS_T mmuInit( void )
{
   return (sramInit() == 0)? OK : MEM_NOT_PRESENT;
}

/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
void mmuRead( MMU_ADDR_T index, RAM_PAYLOAD_T* pItem, size_t len )
{
   for( size_t i = 0; i < len; i++, index++ )
   {
      sramRead64( &pItem[i], index );
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
void mmuWrite( MMU_ADDR_T index, const RAM_PAYLOAD_T* pItem, size_t len )
{
   for( size_t i = 0; i < len; i++, index++ )
   {
      sramWite64( index, &pItem[i] );
   }
}

#endif /* else ifdef CONFIG_SCU_USE_DDR3 */

/*================================== EOF ====================================*/
