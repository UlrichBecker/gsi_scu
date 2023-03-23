/*!
 *  @file daq_ramBuffer_lm32.h
 *  @brief Abstraction layer for handling RAM buffer for DAQ data blocks.
 *
 *  @see scu_ramBuffer.h
 *
 *  @see scu_ddr3.h
 *  @see scu_ddr3.c
 *  @date 07.02.2019
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
#ifndef _DAQ_RAMBUFFER_LM32_H
#define _DAQ_RAMBUFFER_LM32_H
#ifndef __lm32__
  #error Module is for target Lattice Micro 32 (LM32) only!
#endif

#include <daq_ring_admin.h>

#ifdef CONFIG_SCU_USE_DDR3
#include <scu_ddr3_lm32.h>
#else
#error Unknown memory type!
#endif

#include <daq_ramBuffer.h>
#include <daq.h>

/*!
 * @defgroup DAQ_RAM_BUFFER
 * @brief Abstraction layer for handling SCU RAM-Buffer
 *
 * @note At the moment its DDR3 only.
 * @see SCU_DDR3
 * @{
 */

#ifdef __cplusplus
extern "C" {
namespace Scu
{
namespace daq
{
#endif

#if defined( CONFIG_SCU_USE_DDR3 ) || defined(__DOXYGEN__)

#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
/*!
 * @see DDR3_POLL_FT
 */
typedef DDR3_POLL_FT   RAM_DAQ_POLL_FT;
#endif
#endif /* ifdef CONFIG_SCU_USE_DDR3 */

/*! ---------------------------------------------------------------------------
 * @brief Generalized object type for SCU RAM buffer
 */
typedef struct
{
#ifdef CONFIG_SCU_USE_DDR3
   /*!
    * @brief SCU DDR3 administration object.
    */
   DDR3_T   ram;
#else
   #error Unknown RAM-object!
   //TODO maybe in the future will use a other memory type
#endif
   /*!
    * @brief Administration of fifo- indexes.
    * @note The memory space of this object has to be within the shared
    *       memory. \n
    *       Therefore its a pointer in this object.
    */
#ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
   RAM_RING_SHARED_INDEXES_T* volatile pSharedObj;
#else
   RAM_RING_SHARED_OBJECT_T* volatile pSharedObj;
#endif
} RAM_SCU_T;


/*! ---------------------------------------------------------------------------
 * @brief Initializing SCU RAM buffer ready to use.
 * @param pThis Pointer to the RAM object.
 * @param pSharedObj Pointer to the fifo administration in shared memory.
 * @retval 0 Initializing was successful
 * @retval <0 Error
 */
#ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
int ramInit( register RAM_SCU_T* pThis, RAM_RING_SHARED_INDEXES_T* pSharedObj );
#else
int ramInit( register RAM_SCU_T* pThis, RAM_RING_SHARED_OBJECT_T* pSharedObj );
#endif

/*! ----------------------------------------------------------------------------
 * @brief Exchanges the order of devicedeskriptor and payload so that the
 *        devicedescriptor appears at first of the given DAQ channel during
 *        writing in the ring buffer. \n
 *        If not enough free space in the ring buffer, so the oldest
 *        DAQ data blocks becomes deleted until its enough space.
 * @param pThis Pointer to the RAM object object.
 * @param pDaqChannel Pointer of the concerning DAQ-channel-object.
 * @param isShort Decides between long and short DAQ-block.
 *                If true it trades is a short block (DAQ continuous)
 *                else (DAQ HiRes or PostMortem)
 * @return Number of deleted old data blocks.
 */
int ramPushDaqDataBlock( register RAM_SCU_T* pThis,
                         DAQ_CANNEL_T* pDaqChannel,
                         const bool isShort
                       );

/*! ---------------------------------------------------------------------------
 */
STATIC inline ALWAYS_INLINE
void ramWriteItem( register RAM_SCU_T* pThis, const RAM_RING_INDEX_T index,
                   RAM_DAQ_PAYLOAD_T* pItem )
{
#if defined( CONFIG_SCU_USE_DDR3 ) || defined(__DOXYGEN__)
   ddr3write64( &pThis->ram, index, pItem );
#else
   #error Nothing implemented in function ramWriteItem()!
#endif
}

/*! @} */ //End of group DAQ_RAM_BUFFER
#ifdef __cplusplus
} /* namespace daq */
} /* namespace Scu */
} /* extern "C"    */
#endif
#endif /* ifndef _DAQ_RAMBUFFER_H */
/* ================================= EOF ====================================*/
