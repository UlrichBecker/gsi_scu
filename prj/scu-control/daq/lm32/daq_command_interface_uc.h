/*!
 *  @file daq_command_interface_uc.h
 *  @brief Definition of DAQ-commandos and data object for shared memory
 *         LM32 part
 *
 *  @date 27.02.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 ******************************************************************************
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
 ******************************************************************************
 */
#ifndef _DAQ_COMMAND_INTERFACE_UC_H
#define _DAQ_COMMAND_INTERFACE_UC_H
#if !defined(__lm32__) && !defined(__CPPCHECK__)
  #error This module is compilable for LM32 only!
#endif
#if defined( CONFIG_SCU_DAQ_INTEGRATION ) && defined( CONFIG_DAQ_SINGLE_APP )
 #error Either CONFIG_SCU_DAQ_INTEGRATION or CONFIG_DAQ_SINGLE_APP !
#endif
#if !defined( CONFIG_SCU_DAQ_INTEGRATION ) && !defined( CONFIG_DAQ_SINGLE_APP )
 #error Nither CONFIG_SCU_DAQ_INTEGRATION nor CONFIG_DAQ_SINGLE_APP defined!
#endif

#include <daq.h>
#include <daq_ramBuffer_lm32.h>
#ifdef CONFIG_RTOS
   #include <scu_task_daq.h>
#else
   #include <daq_main.h>
#endif
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   #include <scu_shared_mem.h>
   #include <scu_lm32_common.h>
#else
   #include <daq_command_interface.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_DAQ_SINGLE_APP
   #define GET_SHARED() g_shared
#endif
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   extern SCU_SHARED_DATA_T g_shared;
   #define GET_SHARED() g_shared.sDaq
#endif


/*! ---------------------------------------------------------------------------
 * @brief Executes a a from host (Linux) requested function.
 */
bool executeIfRequested( DAQ_ADMIN_T* pDaqAdmin );


#ifdef __cplusplus
}
#endif

#endif /* ifndef _DAQ_COMMAND_INTERFACE_UC_H */
/*================================== EOF ====================================*/
