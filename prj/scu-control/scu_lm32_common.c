/*!
 *  @file scu_lm32_common.c
 *  @brief Common defines module of SCU function generators in LM32.
 *
 *  @date 19.08.2022
 *  @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 *  For testing:
 *  @see https://www-acc.gsi.de/wiki/Hardware/Intern/Saft_Fg_Ctl
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
#include "scu_temperature.h"
#include "scu_lm32_common.h"

/*!
 * @brief Base pointer of SCU bus.
 * @see initializeGlobalPointers
 */
void* g_pScub_base       = NULL;

/*!
 * @brief Base pointer of irq controller for SCU bus
 * @see initializeGlobalPointers
 */
void* g_pScub_irq_base   = NULL;

#if defined( CONFIG_MIL_FG ) && defined( CONFIG_MIL_PIGGY )
/*!
 * @brief Base pointer of MIL extension macro
 * @see initializeGlobalPointers
 */
void* g_pScu_mil_base    = NULL;

/*!
 * @brief Base pointer of IRQ controller for dev bus extension
 * @see initializeGlobalPointers
 */
void* g_pMil_irq_base    = NULL;
#endif

/*====================== Begin of shared memory area ========================*/
/*!
 * @brief Memory space of shared memory for communication with Linux-host
 *        and initializing of them.
 */
SCU_SHARED_DATA_T SHARED g_shared = SCU_SHARED_DATA_INITIALIZER;
/*====================== End of shared memory area ==========================*/


/*! ---------------------------------------------------------------------------
 * @see scu_lm32_common.h
 */
void initializeGlobalPointers( void )
{
   initOneWire();

   /*
    * Additional periphery needed for SCU.
    */

   g_pScub_base = find_device_adr( GSI, SCU_BUS_MASTER );
   if( (int)g_pScub_base == ERROR_NOT_FOUND )
      die( "SCU-bus not found!" );

   g_pScub_irq_base = (uint32_t*)find_device_adr( GSI, SCU_IRQ_CTRL );
   if( (int)g_pScub_irq_base == ERROR_NOT_FOUND )
      die( "Interrupt control for SCU-bus not found!" );

#if defined( CONFIG_MIL_FG ) && defined( CONFIG_MIL_PIGGY )
   g_pScu_mil_base = (void*)find_device_adr( GSI, SCU_MIL );
   if( (int)g_pScu_mil_base == ERROR_NOT_FOUND )
      die( "MIL-bus not found!" );

   g_pMil_irq_base = (uint32_t*)find_device_adr( GSI, MIL_IRQ_CTRL );
   if( (int)g_pMil_irq_base == ERROR_NOT_FOUND )
      die( "Interrupt control for MIL-bus not found!" );
#endif
}

/*! ---------------------------------------------------------------------------
 * @see scu_lm32_common.h
 */
void printCpuId( void )
{
   unsigned int* cpu_info_base;
   cpu_info_base = (unsigned int*)find_device_adr( GSI, CPU_INFO_ROM );
   if( (int)cpu_info_base == ERROR_NOT_FOUND )
      die( "No CPU INFO ROM found!" );

   scuLog( LM32_LOG_INFO, "CPU-ID: 0x%04X\nNumber MSI endpoints: %d\n",
           cpu_info_base[0], cpu_info_base[1] );
}


/*! ---------------------------------------------------------------------------
 * @see scu_lm32_common.h
 */
void tellMailboxSlot( void )
{
   const int slot = getMsiBoxSlot( 0x10 ); //TODO Where does 0x10 come from?
   if( slot == -1 )
      die( "No free slots in MsgBox left!" );

   scuLog( LM32_LOG_INFO,
           ESC_FG_MAGENTA "Configured slot %d in MsgBox\n" ESC_NORMAL, slot );

   g_shared.oSaftLib.oFg.mailBoxSlot = slot;
}

/*! ---------------------------------------------------------------------------
 * @brief Scans for fgs on mil extension and scu bus.
 */
void scanFgs( void )
{
#if defined( CONFIG_READ_MIL_TIME_GAP ) && defined( CONFIG_MIL_FG )
   suspendGapReading();
#endif
#if __GNUC__ >= 9
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif
   fgListFindAll( g_pScub_base,
              #if defined( CONFIG_MIL_FG ) && defined( CONFIG_MIL_PIGGY )
                 g_pScu_mil_base,
              #endif
                 g_shared.oSaftLib.oFg.aMacros,
                 &g_shared.oSaftLib.oTemperatures.ext_id );
#if __GNUC__ >= 9
  #pragma GCC diagnostic pop
#endif
   printFgs();
}

/*! ---------------------------------------------------------------------------
 * @brief initialize procedure at startup
 */
void initAndScan( void )
{
   /*
    *  No function generator macros assigned to channels at startup!
    */
   for( unsigned int channel = 0; channel < ARRAY_SIZE(g_shared.oSaftLib.oFg.aRegs); channel++ )
      g_shared.oSaftLib.oFg.aRegs[channel].macro_number = SCU_INVALID_VALUE;

   /*
    * Update one wire ID and temperatures.
    */
   updateTemperature();

   /*
    * Scans for SCU-bus slave cards and function generators.
    */
   scanFgs();
}

/*! ---------------------------------------------------------------------------
 * @see scu_lm32_common.h
 */
void mmuAllocateDaqBuffer( void )
{
#ifdef CONFIG_USE_MMU
   MMU_STATUS_T status;

 #ifdef CONFIG_SCU_DAQ_INTEGRATION
   STATIC_ASSERT( sizeof(size_t) == sizeof(g_shared.sDaq.ringAdmin.indexes.offset) );
   STATIC_ASSERT( sizeof(size_t) == sizeof(g_shared.sDaq.ringAdmin.indexes.capacity) );
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
   status = mmuAlloc( TAG_ADDAC_DAQ,
                     (size_t*)&g_shared.sDaq.ringAdmin.indexes.offset,
                     (size_t*)&g_shared.sDaq.ringAdmin.indexes.capacity,
                     true );
   #pragma GCC diagnostic pop
 #endif /* ifdef CONFIG_SCU_DAQ_INTEGRATION */
   scuLog( LM32_LOG_INFO, "MMU-Tag 0x%04X for ADDAC-DAQ-buffer: %s\n",
           TAG_ADDAC_DAQ, mmuStatus2String( status ) );
 #if defined( CONFIG_MIL_FG ) && defined( CONFIG_MIL_DAQ_USE_RAM )
   STATIC_ASSERT( sizeof(size_t) == sizeof(g_shared.mDaq.memAdmin.indexes.offset) );
   STATIC_ASSERT( sizeof(size_t) == sizeof(g_shared.mDaq.memAdmin.indexes.capacity) );
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
   status = mmuAlloc( TAG_MIL_DAQ,
                      (size_t*)&g_shared.mDaq.memAdmin.indexes.offset,
                      (size_t*)&g_shared.mDaq.memAdmin.indexes.capacity,
                      true );
   #pragma GCC diagnostic pop
   scuLog( LM32_LOG_INFO, "MMU-Tag 0x%04X for MIL-DAQ-buffer:   %s\n",
           TAG_MIL_DAQ, mmuStatus2String( status ) );
 #endif /* defined( CONFIG_MIL_FG ) && defined( CONFIG_MIL_DAQ_USE_RAM ) */
#endif /* CONFIG_USE_MMU */

#ifdef CONFIG_SCU_DAQ_INTEGRATION
   scuLog( LM32_LOG_INFO, "ADDAC-DAQ buffer offset:   %5u item\n",
           g_shared.sDaq.ringAdmin.indexes.offset );
   scuLog( LM32_LOG_INFO, "ADDAC-DAQ buffer capacity: %5u item\n",
           g_shared.sDaq.ringAdmin.indexes.capacity );
#endif /* ifdef CONFIG_SCU_DAQ_INTEGRATION */

#if defined( CONFIG_MIL_FG ) && defined( CONFIG_MIL_DAQ_USE_RAM )
   scuLog( LM32_LOG_INFO, "MIL-DAQ buffer offset:     %5u item\n",
           g_shared.mDaq.memAdmin.indexes.offset );
   scuLog( LM32_LOG_INFO, "MIL-DAQ buffer capacity:   %5u item\n",
           g_shared.mDaq.memAdmin.indexes.capacity );
#endif /* if defined( CONFIG_MIL_FG ) && defined( CONFIG_MIL_DAQ_USE_RAM ) */
}

/*================================== EOF ====================================*/
