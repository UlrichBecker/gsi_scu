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
 * @defgroup SDB Administration of Self Described Bus (SDB)
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
   ERROR_NOT_FOUND     = 0xFFFFFFFE,
   NO_MSI              = 0xDEADBEE3,
   OWN_MSI             = (1<<31)
} WB_STATUS_T;

/*!
 * @ingroup SDB
 * @brief Device ID
 */
typedef enum
{
   GSI                 = 0x00000651,
   CERN                = 0x0000CE42
} WB_VENDOR_ID_T;

//MSI message forwarding box for master2master MSI
/*!
 * @ingroup SDB
 * @brief Device ID
 */
typedef enum
{
   MSI_MSG_BOX         = 0xFAB0BDD8,

   //CPU periphery
   CPU_INFO_ROM        = 0x10040085,
   CPU_ATOM_ACC        = 0x10040100,
   CPU_SYSTEM_TIME     = 0x10040084,
   CPU_TIMER_CTRL_IF   = 0x10040088,
   CPU_MSI_CTRL_IF     = 0x10040083,
   CPU_MSI_TGT         = 0x1F1A4E39,

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
   CB_GENERIC          = 0xEEF0B198,
   DPRAM_GENERIC       = 0x66CFEB52,
   IRQ_ENDPOINT        = 0x10050082,
   PCIE_IRQ_ENDP       = 0x8A670E73,

   //IO Devices
   OLED_DISPLAY        = 0x93A6F3C4,
   SSD1325_SER_DRIVER  = 0x55D1325D,
   ETHERBONE_MASTER    = 0x00000815,
   ETHERBONE_CFG       = 0x68202B22,

   ECA_EVENT           = 0x8752BF45,
   ECA_CTRL            = 0x8752BF44,
   TLU                 = 0x10051981,
   WR_UART             = 0xE2D13D04,
   WR_PPS_GEN          = 0xDE0D8CED,
   SCU_BUS_MASTER      = 0x9602EB6F,
   SCU_IRQ_CTRL        = 0x9602EB70,
   WB_FG_IRQ_CTRL      = 0x9602EB71,
   MIL_IRQ_CTRL        = 0x9602EB72,

   WR_1Wire            = 0x779C5443,
   User_1Wire          = 0x4C8A0635,
   WB_FG_QUAD          = 0x863E07F0,

   WR_CFIPFlash        = 0x12122121,
   WB_DDR3_if1         = 0x20150828,
   WB_DDR3_if2         = 0x20160525,
   WB_PSEUDO_SRAM      = 0x169EDCB7,
   WR_SYS_CON          = 0xFF07FC47,
   WB_REMOTE_UPDATE    = 0x38956271,
   WB_ASMI             = 0x48526423,
   WB_SCU_REG          = 0xE2D13D04,
   WB_TIMER            = 0xD8BAAA13
} WB_DEVICE_ID_T;

#ifdef __cplusplus
} /* namespace Scu */
} /* extern "C" */
#endif

#endif /* ifndef _SDB_IDS_H */
/*================================== EOF ====================================*/
