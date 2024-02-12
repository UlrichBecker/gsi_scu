/*!
 * @file sdb_lm32.h
 * @brief Finding of Self Described Bus (SDB) device addresses.
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Updated by Ulrich Becker <u.becker@gsi.de>
 * @date 28.03.2023
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
#ifndef _SDB_LM32_H
#define _SDB_LM32_H
#if !defined(__lm32__) && !defined(__CPPCHECK__)
 #error This module os for Lettice Micro 32 (LM32) only!
#endif

#include <helper_macros.h>
#include <inttypes.h>
#include <stdint.h>
#include <sdb_ids.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @todo Remove all this following pointers!
 */
extern volatile uint32_t* g_pCpuIrqSlave;
extern volatile uint32_t* g_pCpuSysTime;
extern volatile uint32_t* g_pCpuMsiBox;
extern volatile uint32_t* g_pMyMsi;
extern volatile uint32_t* g_pCpuId;

STATIC_ASSERT( sizeof(WB_VENDOR_ID_T) == sizeof(uint32_t) );
STATIC_ASSERT( sizeof(WB_DEVICE_ID_T) == sizeof(uint32_t) );

/*!
 * @ingroup SDB
 */
typedef struct
{ /*!
   * @brief Opaque pointer to the actual SDB record.
   */
   void*    pSdb;

   /*!
    * @brief Base address of SDB device.
    */
   uint32_t adr;

   uint32_t msiFirst;
   uint32_t msiLast;
} SDB_LOCATION_T;

STATIC_ASSERT( sizeof(SDB_LOCATION_T) == 16 );

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
SDB_LOCATION_T* find_device_multi( SDB_LOCATION_T* pFound_sdb,
                                   uint32_t* pIdx,
                                   const uint32_t qty,
                                   const WB_VENDOR_ID_T venId,
                                   const WB_DEVICE_ID_T devId );

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
uint32_t* find_device_adr( const WB_VENDOR_ID_T venId, const WB_DEVICE_ID_T devId );

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
SDB_LOCATION_T* find_device_multi_in_subtree( SDB_LOCATION_T* pLoc,
                                              SDB_LOCATION_T* pFound_sdb,
                                              uint32_t* pIdx,
                                              const uint32_t qty,
                                              const WB_VENDOR_ID_T venId,
                                              const WB_DEVICE_ID_T devId );

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
uint32_t* find_device_adr_in_subtree( SDB_LOCATION_T* pLoc,
                                      const WB_VENDOR_ID_T venId,
                                      const WB_DEVICE_ID_T devId );

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
uint32_t getSdbAdr( SDB_LOCATION_T* pLoc );

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
uint8_t* find_device( WB_DEVICE_ID_T devid ) GSI_DEPRECATED; /* USE find_device_adr INSTEAD! */

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 * @todo remove this deprecated function!
 */
void discoverPeriphery( void ); // GSI_DEPRECATED;

#ifdef __DOXYGEN__
/*
 * This is a local function and normally not visible for applications, but
 * Doxygen need this declaration in the header when the function makes
 * recursive calls.
 */
SDB_LOCATION_T* sdbSerarchRecursive( SDB_RECORD_T* pParentSdb,
                                     SDB_LOCATION_T pFoundSdb[],
                                     uint32_t  base,
                                     uint32_t  msiBase,
                                     uint32_t  msiLast,
                                     uint32_t* pIdx,
                                     const uint32_t qty,
                                     const WB_VENDOR_ID_T venId,
                                     const WB_DEVICE_ID_T devId );
#endif /* ifdef __DOXYGEN__ */

#ifdef __cplusplus
}
#endif

#endif /* ifndef _SDB_LM32_H */
/*================================== EOF ====================================*/
