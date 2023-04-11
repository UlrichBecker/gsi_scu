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
 * @brief Returns the SDB record type.
 */
STATIC inline ALWAYS_INLINE
SDB_SELECT_T getRecordType( sdb_record_t* pRecord )
{
   return (SDB_SELECT_T)pRecord->empty.record_type;
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
STATIC uint32_t getMsiAdrLast( sdb_location_t* pLoc )
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
   if( pLoc->pSdb->empty.record_type == SDB_DEVICE )
      return pLoc->adr + pLoc->pSdb->device.sdb_component.addr_last.low;

   if( pLoc->pSdb->empty.record_type == SDB_BRIDGE )
      return pLoc->adr + pLoc->pSdb->bridge.sdb_component.addr_last.low;

   return ERROR_NOT_FOUND;
}
#endif

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
STATIC inline sdb_record_t* getChild( sdb_location_t* pLoc )
{
   return (sdb_record_t*)( pLoc->adr + pLoc->pSdb->bridge.sdb_child.low );
}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 * @brief Helper loop-macro for browsing all SDB devices.
 */
#define FOR_EACH_SDB_RECORD( pParentSdb, pCurrentSdb )      \
   pCurrentSdb = pParentSdb;                                \
   for( int records = pParentSdb->interconnect.sdb_records; \
        records > 0;                                        \
        pCurrentSdb++, records-- )


/*!----------------------------------------------------------------------------
 * @ingroup SDB
 * @brief Helper function for comparing the vendor and device id.
 */
STATIC inline ALWAYS_INLINE
bool compareId( const sdb_record_t* pRecord, const WB_VENDOR_ID_T venId,
                                             const WB_DEVICE_ID_T devId )
{
   return (pRecord->bridge.sdb_component.product.vendor_id.low == venId) &&
          (pRecord->bridge.sdb_component.product.device_id == devId );
}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 * @brief Searches a SDB record recursively by vendor-ID and device-ID.
 */
STATIC sdb_location_t* find_sdb_deep( sdb_record_t* pParent_sdb,
                                      sdb_location_t* pFound_sdb,
                                      uint32_t  base,
                                      uint32_t  msi_base,
                                      uint32_t  msi_last,
                                      uint32_t* pIdx,
                                      const uint32_t qty,
                                      const WB_VENDOR_ID_T venId,
                                      const WB_DEVICE_ID_T devId )
{
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

      if( (pRecord->msi.msi_flags & OWN_MSI) == 0 )
         continue;

      if( (msi_base == NO_MSI) || compareId( pRecord, 0, 0 ) )
         msi_base = NO_MSI;
      else
         msi_adr = pRecord->msi.sdb_component.addr_first.low;

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
               DBPRINT2("Target record at 0x%08X\n", base + pRecord->bridge.sdb_component.addr_first.low);
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
            find_sdb_deep( (sdb_record_t*)(base + pRecord->bridge.sdb_child.low),
                           pFound_sdb,
                           base + pRecord->bridge.sdb_component.addr_first.low,
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

      if( pRecord->msi.msi_flags != OWN_MSI )
         continue;

      msi_adr = pRecord->msi.sdb_component.addr_last.low;
      break;
   }

   return msi_adr;
}

/*!----------------------------------------------------------------------------
 * @ingroup SDB
 */
uint32_t getSdbAdr( sdb_location_t* pLoc )
{
   if( getRecordType( pLoc->pSdb ) == SDB_DEVICE )
      return pLoc->adr + pLoc->pSdb->device.sdb_component.addr_first.low;

   if( getRecordType( pLoc->pSdb ) == SDB_BRIDGE )
      return pLoc->adr + pLoc->pSdb->bridge.sdb_component.addr_first.low;

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
   uint32_t* adr = (uint32_t*)ERROR_NOT_FOUND;

   find_device_multi( &found_sdb, &idx, 1, venId, devId );
   if( idx > 0 )
      adr = (uint32_t*)getSdbAdr( &found_sdb );

   return adr;
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
   uint32_t* adr = (uint32_t*)ERROR_NOT_FOUND;

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
      adr = (uint32_t*)getSdbAdr( &found_sdb );

   return adr;
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