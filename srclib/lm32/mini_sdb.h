/*!
 * @file mini_sdb.h
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
#ifndef _MINI_SDB_H
#define _MINI_SDB_H
#ifndef __lm32__
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
//periphery device pointers
volatile uint32_t* g_pCpuIrqSlave;
volatile uint32_t* g_pCpuSysTime;
volatile uint32_t* g_pCpuMsiBox;
volatile uint32_t* g_pMyMsi;
//volatile unsigned char *BASE_SYSCON;

STATIC_ASSERT( sizeof(WB_VENDOR_ID_T) == sizeof(uint32_t) );
STATIC_ASSERT( sizeof(WB_DEVICE_ID_T) == sizeof(uint32_t) );

/*!
 * @ingroup SDB
 */
typedef struct
{
   uint32_t high;
   uint32_t low;
} pair64_t;

STATIC_ASSERT( sizeof(pair64_t) == sizeof(uint64_t) );

/*!
 * @ingroup SDB
 */
typedef struct
{
   uint8_t reserved[63];
   uint8_t record_type;
} sdb_empty_t;

STATIC_ASSERT( sizeof(sdb_empty_t) == 64 );

/*!
 * @ingroup SDB
 */
typedef struct
{
   pair64_t  vendor_id;
   uint32_t  device_id;
   uint32_t  version;
   uint32_t  date;
   uint8_t   name[19];
   uint8_t   record_type;
} sdb_product_t;

STATIC_ASSERT( sizeof(sdb_product_t) == 40 );

/*!
 * @ingroup SDB
 */
typedef struct
{
   pair64_t addr_first;
   pair64_t addr_last;
   sdb_product_t product;
} sdb_component_t;

STATIC_ASSERT( sizeof(sdb_component_t) == 56 );

/*!
 * @ingroup SDB
 */
typedef struct
{
   uint32_t msi_flags;
   uint32_t bus_specific;
   sdb_component_t sdb_component;
} sdb_msi_t;

STATIC_ASSERT( sizeof(sdb_msi_t) == 64 );
STATIC_ASSERT( offsetof( sdb_empty_t, record_type ) == offsetof( sdb_msi_t, sdb_component.product.record_type ));

/*!
 * @ingroup SDB
 */
typedef struct
{
   uint16_t abi_class;
   uint8_t abi_ver_major;
   uint8_t abi_ver_minor;
   uint32_t bus_specific;
   sdb_component_t sdb_component;
} sdb_device_t;

STATIC_ASSERT( sizeof(sdb_device_t) == 64 );
STATIC_ASSERT( offsetof( sdb_empty_t, record_type ) == offsetof( sdb_device_t, sdb_component.product.record_type ));

/*!
 * @ingroup SDB
 */
typedef struct
{
   pair64_t sdb_child;
   sdb_component_t sdb_component;
} sdb_bridge_t;

STATIC_ASSERT( sizeof(sdb_bridge_t) == 64 );
STATIC_ASSERT( offsetof( sdb_empty_t, record_type ) == offsetof( sdb_bridge_t, sdb_component.product.record_type ));

/*!
 * @ingroup SDB
 */
typedef struct
{
   uint32_t sdb_magic;
   uint16_t sdb_records;
   uint8_t sdb_version;
   uint8_t sdb_bus_type;
   sdb_component_t sdb_component;
} SDB_INTERCONNECT_T;

STATIC_ASSERT( sizeof(SDB_INTERCONNECT_T) == 64 );
STATIC_ASSERT( offsetof( sdb_empty_t, record_type ) == offsetof( SDB_INTERCONNECT_T, sdb_component.product.record_type ));

/*!
 * @ingroup SDB
 * @todo Remove the object sdb_component from all components of this union and
 *       put this union together with sdb_component in a new structure!
 */
typedef union
{
   sdb_empty_t        empty;
   sdb_msi_t          msi;
   sdb_device_t       device;
   sdb_bridge_t       bridge;
   SDB_INTERCONNECT_T interconnect;
} sdb_record_t;

STATIC_ASSERT( sizeof(sdb_record_t) == 64 );

/*!
 * @ingroup SDB
 */
typedef struct
{
   sdb_record_t* pSdb;
   uint32_t      adr;
   uint32_t      msi_first;
   uint32_t      msi_last;
} sdb_location_t;

STATIC_ASSERT( sizeof(sdb_location_t) == 16 );

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
sdb_location_t* find_device_multi( sdb_location_t* pFound_sdb,
                                   uint32_t* pIdx,
                                   const uint32_t qty,
                                   const WB_VENDOR_ID_T venId,
                                   const WB_DEVICE_ID_T devId );

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
uint32_t* find_device_adr( const WB_VENDOR_ID_T venId, const WB_DEVICE_ID_T devId );

/*!----------------------------------------------------------------------------
 */
sdb_location_t* find_device_multi_in_subtree( sdb_location_t* pLoc,
                                              sdb_location_t* pFound_sdb,
                                              uint32_t* pIdx,
                                              const uint32_t qty,
                                              const WB_VENDOR_ID_T venId,
                                              const WB_DEVICE_ID_T devId );

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
uint32_t* find_device_adr_in_subtree( sdb_location_t* pLoc,
                                      const WB_VENDOR_ID_T venId,
                                      const WB_DEVICE_ID_T devId );

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
uint32_t getSdbAdr( sdb_location_t* pLoc );

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
uint8_t* find_device( WB_DEVICE_ID_T devid ) GSI_DEPRECATED; /* USE find_device_adr INSTEAD! */

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 * @todo remove this deprecated function!
 */
void discoverPeriphery( void ); // GSI_DEPRECATED;

#ifdef __cplusplus
}
#endif

#endif /* ifndef _MINI_SDB_H */
/*================================== EOF ====================================*/
