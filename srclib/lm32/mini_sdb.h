#ifndef _MINI_SDB_H
#define _MINI_SDB_H

#include <inttypes.h>
#include <stdint.h>
#include <sdb_ids.h>

#ifdef __cplusplus
extern "C" {
#endif

//periphery device pointers
volatile uint32_t* pTlu; 
volatile uint32_t* pEbm;
volatile uint32_t* pEbCfg;

volatile uint32_t* pEbmLast;
volatile uint32_t* pOledDisplay;
volatile uint32_t* pFpqCtrl;
volatile uint32_t* pFpqData;
volatile uint32_t* pEca;
volatile uint32_t* pCpuId;
volatile uint32_t* pCpuIrqSlave;
volatile uint32_t* pCpuAtomic;
volatile uint32_t* pCpuSysTime;
volatile uint32_t* pCluInfo;
volatile uint32_t* pCpuMsiBox;
volatile uint32_t* pMyMsi;
volatile uint32_t* pUart;
volatile uint32_t* pPps;
//volatile uint32_t* BASE_UART;
volatile uint32_t* pCluCB;
volatile uint32_t* pOneWire;

volatile uint32_t* pCfiPFlash;

volatile uint32_t* pDDR3_if1;
volatile uint32_t* pDDR3_if2;


typedef struct pair64
{
   uint32_t high;
   uint32_t low;
} pair64_t;

typedef struct
{
   char reserved[63];
   uint8_t record_type;
} sdb_empty_t;

typedef struct
{
   pair64_t  vendor_id;
   uint32_t  device_id;
   uint32_t  version;
   uint32_t  date;
   char      name[19];
   uint8_t   record_type;
}  sdb_product_t;

typedef struct
{
   pair64_t addr_first;
   pair64_t addr_last;
   sdb_product_t product;
} sdb_component_t;

typedef struct
{
   uint32_t msi_flags;
   uint32_t bus_specific;
   sdb_component_t sdb_component;
} sdb_msi_t;

typedef struct
{
   uint16_t abi_class;
   uint8_t abi_ver_major;
   uint8_t abi_ver_minor;
   uint32_t bus_specific;
   sdb_component_t sdb_component;
} sdb_device_t;

typedef struct
{
   pair64_t sdb_child;
   sdb_component_t sdb_component;
} sdb_bridge_t;

typedef struct
{
   uint32_t sdb_magic;
   uint16_t sdb_records;
   uint8_t sdb_version;
   uint8_t sdb_bus_type;
   sdb_component_t sdb_component;
} SDB_INTERCONNECT_T;

typedef union
{
   sdb_empty_t        empty;
   sdb_msi_t          msi;
   sdb_device_t       device;
   sdb_bridge_t       bridge;
   SDB_INTERCONNECT_T interconnect;
} sdb_record_t;

typedef struct
{
   sdb_record_t* sdb;
   uint32_t adr;
   uint32_t msi_first;
   uint32_t msi_last;
} sdb_location_t;

sdb_location_t* find_device_multi( sdb_location_t* pFound_sdb,
                                   uint32_t* pIdx,
                                   const uint32_t qty,
                                   const uint32_t venId,
                                   const uint32_t devId );

uint32_t* find_device_adr( const uint32_t venId, const uint32_t devId );

sdb_location_t* find_device_multi_in_subtree( sdb_location_t* pLoc,
                                              sdb_location_t* pFound_sdb,
                                              uint32_t* pIdx,
                                              const uint32_t qty,
                                              const uint32_t venId,
                                              const uint32_t devId );

uint32_t* find_device_adr_in_subtree( sdb_location_t *pLoc,
                                      const uint32_t venId,
                                      const uint32_t devId );

sdb_location_t *find_sdb_deep( sdb_record_t* pParent_sdb,
                               sdb_location_t* pFound_sdb,
                               uint32_t  base,
                               uint32_t  msi_base,
                               uint32_t  msi_last,
                               uint32_t* pIdx,
                               const uint32_t  qty,
                               const uint32_t  venId,
                               const uint32_t  devId );

uint32_t       getSdbAdr( sdb_location_t* loc);

uint32_t       getSdbAdrLast( sdb_location_t* loc );
uint32_t       getMsiAdr( sdb_location_t* loc);
uint32_t       getMsiAdrLast( sdb_location_t* loc );
sdb_record_t*  getChild( sdb_location_t* loc);
uint32_t       getMsiUpperRange( void );


uint8_t*       find_device( uint32_t devid ); //DEPRECATED, USE find_device_adr INSTEAD!

void           discoverPeriphery( void );

#ifdef __cplusplus
}
#endif

#endif
