/*!
 * @file mini_sdb.c
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
#include <stdio.h>
#include <stdbool.h>
#include "mini_sdb.h"
#include "dbg.h"
#include "memlayout.h"

#ifndef SDB_ROOT_ADDR
/*!
 * @ingroup SDB
 * @brief Base address of SDB
 * @todo Check whether this value will changed in SCU4!
 */
#define SDB_ROOT_ADDR 0x91600800
#endif

/*=================== Construction of type sdb_record_t =====================*/
/*!
 * @ingroup SDB
 * @see sdb_product_t
 * @see sdb_component_t
 * @see sdb_bridge_t
 */
typedef struct
{
   uint32_t high;
   uint32_t low;
} pair64_t;

STATIC_ASSERT( sizeof(pair64_t) == sizeof(uint64_t) );

/*!
 * @ingroup SDB
 * @see sdb_component_t
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
   pair64_t      addr_first;
   pair64_t      addr_last;
   sdb_product_t product;
} sdb_component_t;

STATIC_ASSERT( sizeof(sdb_component_t) == 56 );

/*!
 * @ingroup SDB
 * @see sdb_union_t
 */
typedef struct
{
   uint32_t msi_flags;
   uint32_t bus_specific;
} sdb_msi_t;

STATIC_ASSERT( sizeof(sdb_msi_t) == 8 );

/*!
 * @ingroup SDB
 * @see sdb_union_t
 */
typedef struct
{
   uint16_t abi_class;
   uint8_t  abi_ver_major;
   uint8_t  abi_ver_minor;
   uint32_t bus_specific;
} sdb_device_t;

STATIC_ASSERT( sizeof(sdb_device_t) == 8 );

/*!
 * @ingroup SDB
 * @see sdb_union_t
 */
typedef struct
{
   pair64_t sdb_child;
} sdb_bridge_t;

STATIC_ASSERT( sizeof(sdb_bridge_t) == 8 );

/*!
 * @ingroup SDB
 * @see sdb_union_t
 */
typedef struct
{
   uint32_t sdb_magic;
   uint16_t sdb_records;
   uint8_t  sdb_version;
   uint8_t  sdb_bus_type;
} SDB_INTERCONNECT_T;

STATIC_ASSERT( sizeof(SDB_INTERCONNECT_T) == 8 );

/*!
 * @ingroup SDB
 */
typedef union
{
   sdb_msi_t          msi;
   sdb_device_t       device;
   sdb_bridge_t       bridge;
   SDB_INTERCONNECT_T interconnect;
} sdb_union_t;

STATIC_ASSERT( sizeof(sdb_union_t) == 8 );

/*!
 * @ingroup SDB
 */
typedef struct
{
   sdb_union_t     sdb_union;
   sdb_component_t sdb_component;
} sdb_record_t;

STATIC_ASSERT( sizeof(sdb_record_t) == 64 );
STATIC_ASSERT( offsetof( sdb_record_t, sdb_component ) == 8 );
STATIC_ASSERT( offsetof( sdb_record_t, sdb_component.product.record_type ) ==
               sizeof(sdb_record_t)-sizeof(uint8_t) );

/*============== End of construction of type sdb_record_t ===================*/

volatile uint32_t* g_pCpuIrqSlave;
volatile uint32_t* g_pCpuSysTime;
volatile uint32_t* g_pCpuMsiBox;
volatile uint32_t* g_pMyMsi;

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 * @brief Returns the pointer to the root SDB record.
 */
STATIC inline ALWAYS_INLINE
sdb_record_t* getSdbRoot( void )
{
   sdb_record_t* ptr;
   asm volatile
   (
      ".long " TO_STRING( SDB_ROOT_ADDR ) "\n\t"
      : "=r" (ptr)
      :
      : "memory"
   );
   return ptr;
}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 * @brief Extracts the pointer of type sdb_record_t from a object of type
 *        sdb_location_t. 
 */
STATIC inline ALWAYS_INLINE
sdb_record_t* getSdbRecord( sdb_location_t* pLoc )
{
   return (sdb_record_t*)pLoc->pSdb;
}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 * @brief Returns the SDB record type.
 */
STATIC inline ALWAYS_INLINE
SDB_SELECT_T getRecordType( sdb_record_t* pRecord )
{
   return (SDB_SELECT_T)pRecord->sdb_component.product.record_type;
}


/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
STATIC inline uint32_t getMsiAdr( sdb_location_t* pLoc )
{
   return pLoc->msi_first;
}

#if 0
/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
STATIC inline uint32_t getMsiAdrLast( sdb_location_t* pLoc )
{
   return pLoc->msi_last;
}
#endif

#if 0
/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
STATIC uint32_t getSdbAdrLast( sdb_location_t* pLoc )
{
   switch( getRecordType( getSdbRecord( pLoc )) )
   {
      case SDB_DEVICE: case SDB_BRIDGE:
      {
         return pLoc->adr + getSdbRecord( pLoc )->sdb_component.addr_last.low;
      }

      default: break;
   }

   return ERROR_NOT_FOUND;
}
#endif

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
STATIC inline sdb_record_t* getChild( sdb_location_t* pLoc )
{
   if( getRecordType( getSdbRecord( pLoc ) ) == SDB_BRIDGE )
      return (sdb_record_t*)( pLoc->adr + getSdbRecord( pLoc )->sdb_union.bridge.sdb_child.low );
   else
      return NULL;
}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 * @brief Helper loop-macro for browsing all SDB devices.
 */
#define FOR_EACH_SDB_RECORD( pParentSdb, pCurrentSdb )                \
   pCurrentSdb = pParentSdb;                                          \
   for( int records = pParentSdb->sdb_union.interconnect.sdb_records; \
        records > 0;                                                  \
        pCurrentSdb++, records-- )

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 * @brief Helper function for comparing the vendor and device id.
 */
STATIC inline ALWAYS_INLINE
bool compareId( const sdb_record_t* pRecord, const WB_VENDOR_ID_T venId,
                                             const WB_DEVICE_ID_T devId )
{
   return (pRecord->sdb_component.product.vendor_id.low == venId) &&
          (pRecord->sdb_component.product.device_id == devId );

}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 * @brief Searches a SDB record recursively by vendor-ID and device-ID.
 */
STATIC sdb_location_t* find_sdb_deep( sdb_record_t* pParent_sdb,
                                      sdb_location_t pFound_sdb[],
                                      uint32_t  base,
                                      uint32_t  msi_base,
                                      uint32_t  msi_last,
                                      uint32_t* pIdx,
                                      const uint32_t qty,
                                      const WB_VENDOR_ID_T venId,
                                      const WB_DEVICE_ID_T devId )
{
   if( pParent_sdb == NULL )
      return NULL;
   
   sdb_record_t* pRecord;

   /*
    * discover MSI address before moving on to possible next Crossbar
    */
   uint32_t msi_cnt = 0;
   uint32_t msi_adr = 0;
   FOR_EACH_SDB_RECORD( pParent_sdb, pRecord )
   {
      if( getRecordType( pRecord ) != SDB_MSI )
         continue;

      if( (pRecord->sdb_union.msi.msi_flags & OWN_MSI) == 0 )
         continue;

      if( (msi_base == NO_MSI) || compareId( pRecord, 0, 0 ) )
         msi_base = NO_MSI;
      else
         msi_adr = pRecord->sdb_component.addr_first.low;

      msi_cnt++;
   }

   if( msi_cnt > 1 )
   { /*
      * This is an error, the CB layout is messed up
      */
      DBPRINT1("Found more than 1 MSI at 0x%08X par 0x%08X\n", base, (uint32_t)(unsigned char*)pParent_sdb);
      *pIdx = 0;
      return pFound_sdb;
   }

   FOR_EACH_SDB_RECORD( pParent_sdb, pRecord )
   {
      switch( getRecordType( pRecord ) )
      {
         case SDB_BRIDGE: case SDB_DEVICE: case SDB_MSI:
         {
            if( compareId( pRecord, venId, devId ) )
            {
               DBPRINT2("Target record at 0x%08X\n", base + pRecord->sdb_component.addr_first.low);
               pFound_sdb[*pIdx].pSdb      = pRecord;
               pFound_sdb[*pIdx].adr       = base;
               pFound_sdb[*pIdx].msi_first = msi_base + msi_adr;
               pFound_sdb[*pIdx].msi_last  = msi_base + msi_adr + msi_last;
               (*pIdx)++;
            }

            if( getRecordType( pRecord ) != SDB_BRIDGE )
               break;
            /*
             * CAUTION: Recursive call in the case of SDB-bridge!
             */
            find_sdb_deep( (sdb_record_t*)(base + pRecord->sdb_union.bridge.sdb_child.low),
                           pFound_sdb,
                           base + pRecord->sdb_component.addr_first.low,
                           msi_base+msi_adr,
                           msi_last,
                           pIdx,
                           qty,
                           venId,
                           devId );
            break;
         }

         default: break;
      }

      if( *pIdx >= qty )
         break;
   } /* FOR_EACH_SDB_RECORD() */
   return pFound_sdb;
}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
STATIC uint32_t getMsiUpperRange( void )
{
   uint32_t msi_adr    = 0;

   /*
    * get upper range of MSI target
    */
   sdb_record_t* pRecord;
   FOR_EACH_SDB_RECORD( getSdbRoot(), pRecord )
   {
      if( getRecordType( pRecord ) != SDB_MSI )
         continue;

      if( pRecord->sdb_union.msi.msi_flags != OWN_MSI )
         continue;

      msi_adr = pRecord->sdb_component.addr_last.low;
      break;
   }

   return msi_adr;
}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
uint32_t getSdbAdr( sdb_location_t* pLoc )
{
   switch( getRecordType( getSdbRecord( pLoc ) ) )
   {
      case SDB_DEVICE: case SDB_BRIDGE:
      {
         return pLoc->adr + getSdbRecord( pLoc )->sdb_component.addr_first.low;
      }

      default: break;
   }

   return ERROR_NOT_FOUND;
}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
sdb_location_t* find_device_multi( sdb_location_t* pFound_sdb,
                                   uint32_t* pIdx,
                                   const uint32_t qty,
                                   const WB_VENDOR_ID_T venId,
                                   const WB_DEVICE_ID_T devId )
{
   return find_sdb_deep( getSdbRoot(),
                         pFound_sdb,
                         0,
                         0,
                         getMsiUpperRange(),
                         pIdx,
                         qty,
                         venId,
                         devId );
}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
uint32_t* find_device_adr( const WB_VENDOR_ID_T venId, const WB_DEVICE_ID_T devId )
{
   sdb_location_t found_sdb;
   uint32_t idx = 0;

   find_device_multi( &found_sdb, &idx, 1, venId, devId );
   if( idx > 0 )
      return (uint32_t*)getSdbAdr( &found_sdb );

   return (uint32_t*)ERROR_NOT_FOUND;
}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
sdb_location_t* find_device_multi_in_subtree( sdb_location_t* pLoc,
                                              sdb_location_t* pFound_sdb,
                                              uint32_t* pIdx,
                                              const uint32_t qty,
                                              const WB_VENDOR_ID_T venId,
                                              const WB_DEVICE_ID_T devId )
{
   return find_sdb_deep( getChild(pLoc),
                         pFound_sdb,
                         getSdbAdr( pLoc ),
                         getMsiAdr( pLoc ),
                         getMsiUpperRange(),
                         pIdx,
                         qty,
                         venId,
                         devId );
}

/*!----------------------------------------------------------------------------
 */
uint32_t* find_device_adr_in_subtree( sdb_location_t* pLoc, const WB_VENDOR_ID_T venId, const WB_DEVICE_ID_T devId )
{
   sdb_location_t found_sdb;
   uint32_t idx = 0;

   find_sdb_deep( getChild( pLoc ),
                  &found_sdb,
                  getSdbAdr( pLoc ),
                  getMsiAdr( pLoc ),
                  getMsiUpperRange(),
                  &idx,
                  1,
                  venId,
                  devId );
   if( idx > 0 )
      return (uint32_t*)getSdbAdr( &found_sdb );

   return (uint32_t*)ERROR_NOT_FOUND;
}

/*!----------------------------------------------------------------------------
 */
//DEPRECATED, USE find_device_adr INSTEAD!
uint8_t* find_device( WB_DEVICE_ID_T devid )
{
   return (uint8_t*)find_device_adr( GSI, devid );
}

/*!----------------------------------------------------------------------------
 */
void discoverPeriphery( void )
{
   sdb_location_t    found_sdb[20];

   g_pCpuSysTime     = find_device_adr( GSI, CPU_SYSTEM_TIME );
   g_pCpuIrqSlave    = find_device_adr( GSI, CPU_MSI_CTRL_IF );

   g_pCpuMsiBox      = NULL;
   g_pMyMsi          = NULL;
   uint32_t idx      = 0;
   find_device_multi( &found_sdb[0], &idx, 1, GSI, MSI_MSG_BOX );
   if( idx != 0 )
   {
      g_pCpuMsiBox    = (uint32_t*)getSdbAdr(&found_sdb[0]);
      g_pMyMsi        = (uint32_t*)getMsiAdr(&found_sdb[0]);
   }
}
/*================================== EOF ====================================*/
