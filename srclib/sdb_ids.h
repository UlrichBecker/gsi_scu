/*!
 * @brief Definition of SDB addresses, outsourced from
 *        mini_sdb.h
 *
 * @file      sdb_ids.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      21.02.2019
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
#ifndef _SDB_IDS_H
#define _SDB_IDS_H

/*!
 * @defgroup SDB Administration of Self Described Bis (SDB)
 */

#ifdef __cplusplus
extern "C" {
namespace Scu
{
#endif

/////////////////////////////////////////////////////////////////////
//! @note SBD BASE ADR IS AUTOMAPPED IN GATEWARE. USE getRootSdb() //
/////////////////////////////////////////////////////////////////////

/*!
 * @ingroup SDB
 */
typedef enum
{
   SDB_INTERCONNET     = 0x00,
   SDB_DEVICE          = 0x01,
   SDB_BRIDGE          = 0x02,
   SDB_MSI             = 0x03,
   SDB_EMPTY           = 0xFF
} SDB_SELECT_T;

/*!
 * @ingroup SDB
 */
typedef enum
{
   ERROR_NOT_FOUND     = 0xFFFFFFFE,
   NO_MSI              = 0XDEADBEE3,
   OWN_MSI             = (1<<31)
} WB_STATUS_T;

/*!
 * @ingroup SDB
 * @brief Device ID
 */
typedef enum
{
   GSI                 = 0x00000651,
   CERN                = 0x0000ce42
} WB_VENDOR_ID_T;

//MSI message forwarding box for master2master MSI
/*!
 * @ingroup SDB
 * @brief Device ID
 */
typedef enum
{
   MSI_MSG_BOX         = 0xfab0bdd8,

   //CPU periphery
   CPU_INFO_ROM        = 0x10040085,
   CPU_ATOM_ACC        = 0x10040100,
   CPU_SYSTEM_TIME     = 0x10040084,
   CPU_TIMER_CTRL_IF   = 0x10040088,
   CPU_MSI_CTRL_IF     = 0x10040083,
   CPU_MSI_TGT         = 0x1f1a4e39,

   //Cluster periphery
   LM32_CB_CLUSTER     = 0x10041000,
   CLU_INFO_ROM        = 0x10040086,
   LM32_RAM_SHARED     = 0x81111444,
   FTM_PRIOQ_CTRL      = 0x10040200,
   FTM_PRIOQ_DATA      = 0x10040201,

   //External interface to CPU RAMs & IRQs
   LM32_RAM_USER       = 0x54111351,
   LM32_IRQ_EP         = 0x10050083,

   //Generic stuff
   CB_GENERIC          = 0xeef0b198,
   DPRAM_GENERIC       = 0x66cfeb52,
   IRQ_ENDPOINT        = 0x10050082,
   PCIE_IRQ_ENDP       = 0x8a670e73,

   //IO Devices
   OLED_DISPLAY        = 0x93a6f3c4,
   SSD1325_SER_DRIVER  = 0x55d1325d,
   ETHERBONE_MASTER    = 0x00000815,
   ETHERBONE_CFG       = 0x68202b22,

   ECA_EVENT           = 0x8752bf45,
   ECA_CTRL            = 0x8752bf44,
   TLU                 = 0x10051981,
   WR_UART             = 0xe2d13d04,
   WR_PPS_GEN          = 0xde0d8ced,
   SCU_BUS_MASTER      = 0x9602eb6f,
   SCU_IRQ_CTRL        = 0x9602eb70,
   WB_FG_IRQ_CTRL      = 0x9602eb71,
   MIL_IRQ_CTRL        = 0x9602eb72,

   WR_1Wire            = 0x779c5443,
   User_1Wire          = 0x4c8a0635,
   WB_FG_QUAD          = 0x863e07f0,

   WR_CFIPFlash        = 0x12122121,
   WB_DDR3_if1         = 0x20150828,
   WB_DDR3_if2         = 0x20160525,
   WB_PSEUDO_SRAM      = 0x169edcb7,
   WR_SYS_CON          = 0xff07fc47,
   WB_REMOTE_UPDATE    = 0x38956271,
   WB_ASMI             = 0x48526423,
   WB_SCU_REG          = 0xe2d13d04
} WB_DEVICE_ID_T;

#ifdef __cplusplus
} /* namespace Scu */
} /* extern "C" */
#endif

#endif /* ifndef _SDB_IDS_H */
/*================================== EOF ====================================*/
