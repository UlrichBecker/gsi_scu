/*!
 * @file sdb_lm32.c
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
#include "sdb_lm32.h"
#include "dbg.h"

#ifndef SDB_ROOT_ADDR
/*!
 * @ingroup SDB
 * @brief Base address of SDB
 * @todo Check whether this value will changed in SCU4!
 */
#define SDB_ROOT_ADDR 0x91600800
#endif

/*
 * Being sure that a pointer is a 32-bit value in LM32.
 */
STATIC_ASSERT( sizeof(void*) == sizeof(uint32_t) );

/*=================== Construction of type SDB_RECORD_T =====================*/

/*!
 * @ingroup SDB
 * @brief Indicates the which record type is actual in the union.
 */
typedef enum
{
   SDB_INTERCONNET = 0x00,
   SDB_DEVICE      = 0x01,
   SDB_BRIDGE      = 0x02,
   SDB_MSI         = 0x03
} SDB_RECORD_TYPE_T;

/*!
 * @ingroup SDB
 * @brief Helper type for 64-bit accesses.
 * @see SDB_PRODUCT_T
 * @see SDB_COMPONENT_T
 * @see SDB_BRIDGE_T
 */
typedef struct
{
   uint32_t high;
   uint32_t low;
} PAIR64_T;

STATIC_ASSERT( sizeof(PAIR64_T) == sizeof(uint64_t) );

/*!
 * @ingroup SDB
 * @see SDB_COMPONENT_T
 */
typedef struct
{
   PAIR64_T  vendorId;
   uint32_t  deviceId;
   uint32_t  version;
   uint32_t  date;
   uint8_t   name[19];
   uint8_t   recordType;
} SDB_PRODUCT_T;

STATIC_ASSERT( sizeof(SDB_PRODUCT_T) == 40 );

/*!
 * @ingroup SDB
 */
typedef struct
{
   PAIR64_T      addrFirst;
   PAIR64_T      addrLast;
   SDB_PRODUCT_T product;
} SDB_COMPONENT_T;

STATIC_ASSERT( sizeof(SDB_COMPONENT_T) == 56 );

/*!
 * @ingroup SDB
 * @see SDB_UNION_T
 */
typedef struct
{
   uint32_t msiFlags;
   uint32_t busSpecific;
} SDB_MSI_T;

STATIC_ASSERT( sizeof(SDB_MSI_T) == 8 );

/*!
 * @ingroup SDB
 * @see SDB_UNION_T
 */
typedef struct
{
   uint16_t abiClass;
   uint8_t  abiVerMajor;
   uint8_t  abiVerMinor;
   uint32_t busSpecific;
} SDB_DEVICE_T;

STATIC_ASSERT( sizeof(SDB_DEVICE_T) == 8 );

/*!
 * @ingroup SDB
 * @see SDB_UNION_T
 */
typedef struct
{
   PAIR64_T sdbChild;
} SDB_BRIDGE_T;

STATIC_ASSERT( sizeof(SDB_BRIDGE_T) == 8 );

/*!
 * @ingroup SDB
 * @see SDB_UNION_T
 */
typedef struct
{
   uint32_t sdbMagic;
   uint16_t sdbRecords;
   uint8_t  sdbVersion;
   uint8_t  sdbBusType;
} SDB_INTERCONNECT_T;

STATIC_ASSERT( sizeof(SDB_INTERCONNECT_T) == 8 );

/*!
 * @ingroup SDB
 */
typedef union
{
   SDB_MSI_T          msi;
   SDB_DEVICE_T       device;
   SDB_BRIDGE_T       bridge;
   SDB_INTERCONNECT_T interconnect;
} SDB_UNION_T;

STATIC_ASSERT( sizeof(SDB_UNION_T) == 8 );

/*!
 * @ingroup SDB
 */
typedef struct
{
   SDB_UNION_T     sdbUnion;
   SDB_COMPONENT_T sdbComponent;
} SDB_RECORD_T;

STATIC_ASSERT( sizeof(SDB_RECORD_T) == 64 );
STATIC_ASSERT( offsetof( SDB_RECORD_T, sdbComponent ) == 8 );
STATIC_ASSERT( offsetof( SDB_RECORD_T, sdbComponent.product.recordType ) ==
               sizeof(SDB_RECORD_T)-sizeof(uint8_t) );

/*============== End of construction of type SDB_RECORD_T ===================*/

volatile uint32_t* g_pWbZycleAtomic;
volatile uint32_t* g_pCpuIrqSlave;
volatile uint32_t* g_pCpuSysTime;
volatile uint32_t* g_pCpuMsiBox;
volatile uint32_t* g_pMyMsi;
volatile uint32_t* g_pCpuId;

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 * @note SDB base address is AUTOMAPPED in GATEWARE
 * @brief Returns the pointer to the root SDB record.
 */
STATIC inline ALWAYS_INLINE
SDB_RECORD_T* getSdbRoot( void )
{
   SDB_RECORD_T* ptr;

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
 * @brief Extracts the pointer of type SDB_RECORD_T from a object of type
 *        SDB_LOCATION_T.
 */
STATIC inline ALWAYS_INLINE
SDB_RECORD_T* getSdbRecord( SDB_LOCATION_T* pLoc )
{
   return (SDB_RECORD_T*)pLoc->pSdb;
}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 * @brief Returns the SDB record type.
 */
STATIC inline ALWAYS_INLINE
SDB_RECORD_TYPE_T getRecordType( SDB_RECORD_T* pRecord )
{
   return (SDB_RECORD_TYPE_T)pRecord->sdbComponent.product.recordType;
}


/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
STATIC inline uint32_t getMsiAdr( SDB_LOCATION_T* pLoc )
{
   return pLoc->msiFirst;
}

#if 0
/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
STATIC inline uint32_t getMsiAdrLast( SDB_LOCATION_T* pLoc )
{
   return pLoc->msiLast;
}
#endif

#if 0
/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
STATIC uint32_t getSdbAdrLast( SDB_LOCATION_T* pLoc )
{
   switch( getRecordType( getSdbRecord( pLoc )) )
   {
      case SDB_DEVICE: case SDB_BRIDGE:
      {
         return pLoc->adr + getSdbRecord( pLoc )->sdbComponent.addrLast.low;
      }

      default: break;
   }

   return ERROR_NOT_FOUND;
}
#endif


/*!----------------------------------------------------------------------------
 * @ingroup SDB
 * @brief Helper loop-macro for browsing all SDB devices.
 */
#define FOR_EACH_SDB_RECORD( pParentSdb, pCurrentSdb )              \
   pCurrentSdb = pParentSdb;                                        \
   for( int records = pParentSdb->sdbUnion.interconnect.sdbRecords; \
        records > 0;                                                \
        pCurrentSdb++, records-- )

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 * @brief Helper function for comparing the vendor and device id.
 */
STATIC inline ALWAYS_INLINE
bool compareId( const SDB_RECORD_T* pRecord, const WB_VENDOR_ID_T venId,
                                             const WB_DEVICE_ID_T devId )
{
   return (pRecord->sdbComponent.product.vendorId.low == venId) &&
          (pRecord->sdbComponent.product.deviceId == devId );

}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 * @brief Searches a SDB record recursively by vendor-ID and device-ID.
 */
STATIC SDB_LOCATION_T* sdbSerarchRecursive( SDB_RECORD_T* pParentSdb,
                                            SDB_LOCATION_T pFoundSdb[],
                                            uint32_t  base,
                                            uint32_t  msiBase,
                                            uint32_t  msiLast,
                                            uint32_t* pIdx,
                                            const uint32_t qty,
                                            const WB_VENDOR_ID_T venId,
                                            const WB_DEVICE_ID_T devId )
{
   if( pParentSdb == NULL )
      return NULL;

   SDB_RECORD_T* pRecord;

   /*
    * Discover MSI address before moving on to possible next crossbar.
    */
   uint32_t msiCount = 0;
   uint32_t msiAddr = 0;
   FOR_EACH_SDB_RECORD( pParentSdb, pRecord )
   {
      if( getRecordType( pRecord ) != SDB_MSI )
         continue;

      if( (pRecord->sdbUnion.msi.msiFlags & OWN_MSI) == 0 )
         continue;

      if( (msiBase == NO_MSI) || compareId( pRecord, 0, 0 ) )
         msiBase = NO_MSI;
      else
         msiAddr = pRecord->sdbComponent.addrFirst.low;

      msiCount++;
   }

   if( msiCount > 1 )
   { /*
      * More then one MSI-record found, CB layout is corrupt!
      */
      DBPRINT1( "Found more than 1 MSI at 0x%08X par 0x%p\n", base, pParentSdb );
      *pIdx = 0;
      return pFoundSdb;
   }

   FOR_EACH_SDB_RECORD( pParentSdb, pRecord )
   {
      switch( getRecordType( pRecord ) )
      {
         case SDB_BRIDGE: FALL_THROUGH
         case SDB_DEVICE: FALL_THROUGH
         case SDB_MSI:
         {
            const uint32_t msiFirst = msiBase + msiAddr;
            if( compareId( pRecord, venId, devId ) )
            {
               DBPRINT2("Target record at 0x%08X\n", base + pRecord->sdbComponent.addrFirst.low);
               pFoundSdb[*pIdx].pSdb     = pRecord;
               pFoundSdb[*pIdx].adr      = base;
               pFoundSdb[*pIdx].msiFirst = msiFirst;
               pFoundSdb[*pIdx].msiLast  = msiFirst + msiLast;
               (*pIdx)++;
            }

            if( getRecordType( pRecord ) != SDB_BRIDGE )
               break;
            /*
             * CAUTION: Recursive call in the case of SDB-bridge!
             */
            sdbSerarchRecursive( (SDB_RECORD_T*)(base + pRecord->sdbUnion.bridge.sdbChild.low),
                                 pFoundSdb,
                                 base + pRecord->sdbComponent.addrFirst.low,
                                 msiFirst,
                                 msiLast,
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
   return pFoundSdb;
}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
STATIC uint32_t getMsiUpperRange( void )
{
   uint32_t msiAddr = 0;

   /*
    * get upper range of MSI target
    */
   SDB_RECORD_T* pRecord;
   FOR_EACH_SDB_RECORD( getSdbRoot(), pRecord )
   {
      if( getRecordType( pRecord ) != SDB_MSI )
         continue;

      if( pRecord->sdbUnion.msi.msiFlags != OWN_MSI )
         continue;

      msiAddr = pRecord->sdbComponent.addrLast.low;
      break;
   }

   return msiAddr;
}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
uint32_t getSdbAdr( SDB_LOCATION_T* pLoc )
{
   switch( getRecordType( getSdbRecord( pLoc ) ) )
   {
      case SDB_DEVICE: case SDB_BRIDGE:
      {
         return pLoc->adr + getSdbRecord( pLoc )->sdbComponent.addrFirst.low;
      }

      default: break;
   }

   return ERROR_NOT_FOUND;
}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
OPTIMIZE( "-O0"  )
SDB_LOCATION_T* find_device_multi( SDB_LOCATION_T* pFoundSdb,
                                   uint32_t* pIdx,
                                   const uint32_t qty,
                                   const WB_VENDOR_ID_T venId,
                                   const WB_DEVICE_ID_T devId )
{
   return sdbSerarchRecursive( getSdbRoot(),
                               pFoundSdb,
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
   SDB_LOCATION_T foundSdb;
   uint32_t idx = 0;

   find_device_multi( &foundSdb, &idx, 1, venId, devId );
   if( idx > 0 )
      return (uint32_t*)getSdbAdr( &foundSdb );

   return (uint32_t*)ERROR_NOT_FOUND;
}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
STATIC inline SDB_RECORD_T* getChild( SDB_LOCATION_T* pLoc )
{
   if( getRecordType( getSdbRecord( pLoc ) ) == SDB_BRIDGE )
      return (SDB_RECORD_T*)( pLoc->adr + getSdbRecord( pLoc )->sdbUnion.bridge.sdbChild.low );
   else
      return NULL;
}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
SDB_LOCATION_T* find_device_multi_in_subtree( SDB_LOCATION_T* pLoc,
                                              SDB_LOCATION_T* pFoundSdb,
                                              uint32_t* pIdx,
                                              const uint32_t qty,
                                              const WB_VENDOR_ID_T venId,
                                              const WB_DEVICE_ID_T devId )
{
   return sdbSerarchRecursive( getChild( pLoc ),
                               pFoundSdb,
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
uint32_t* find_device_adr_in_subtree( SDB_LOCATION_T* pLoc, const WB_VENDOR_ID_T venId, const WB_DEVICE_ID_T devId )
{
   SDB_LOCATION_T foundSdb;
   uint32_t idx = 0;

   sdbSerarchRecursive( getChild( pLoc ),
                        &foundSdb,
                        getSdbAdr( pLoc ),
                        getMsiAdr( pLoc ),
                        getMsiUpperRange(),
                        &idx,
                        1,
                        venId,
                        devId );
   if( idx > 0 )
      return (uint32_t*)getSdbAdr( &foundSdb );

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
void wbZycleEnterBase( void )
{
   *g_pWbZycleAtomic = 1;
}

/*!----------------------------------------------------------------------------
 */
void wbZycleExitBase( void )
{
   *g_pWbZycleAtomic = 0;
}

/*!----------------------------------------------------------------------------
 */
bool isInWbZycle( void )
{
   return *g_pWbZycleAtomic != 0;
}

/*!----------------------------------------------------------------------------
 */
void discoverPeriphery( void )
{
   SDB_LOCATION_T    foundSdb[1];

   g_pCpuSysTime     = find_device_adr( GSI, CPU_SYSTEM_TIME );
   g_pCpuIrqSlave    = find_device_adr( GSI, CPU_MSI_CTRL_IF );
   g_pCpuId          = find_device_adr( GSI, CPU_INFO_ROM );
   g_pWbZycleAtomic  = find_device_adr( GSI, CPU_ATOM_ACC );

   *g_pWbZycleAtomic = 0;

   g_pCpuMsiBox      = NULL;
   g_pMyMsi          = NULL;
   uint32_t idx      = 0;
   find_device_multi( foundSdb, &idx, ARRAY_SIZE(foundSdb), GSI, MSI_MSG_BOX );
   if( idx != 0 )
   {
      g_pCpuMsiBox    = (uint32_t*)getSdbAdr(&foundSdb[0]);
      g_pMyMsi        = (uint32_t*)getMsiAdr(&foundSdb[0]);
   }
}
/*================================== EOF ====================================*/
