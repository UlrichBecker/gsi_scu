/*!
 * @file sys_exception.c
 * @brief Handling of non IRQ- LM32-exceptions.
 *
 * @date 19.08.2022
 * @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see sys_exception.h
 * @see crt0ScuLm32.S
 * @see stups.c
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
#include <scu_syslog.h>
#include <scu_logutil.h>
#include <eb_console_helper.h>
#include <lm32Interrupts.h>
#include "sys_exception.h"

//#define CONFIG_STOP_ON_LM32_EXCEPTION

/*! ---------------------------------------------------------------------------
 * @brief Callback function becomes invoked by LM32 when an exception has
 *        been appeared.
 */
void _onException( const uint32_t sig )
{
   char* str;
   #define _CASE_SIGNAL( S ) case S: str = #S; break;
   switch( sig )
   {
      _CASE_SIGNAL( SIGINT )
      _CASE_SIGNAL( SIGTRAP )
      _CASE_SIGNAL( SIGFPE )
      _CASE_SIGNAL( SIGSEGV )
      default: str = "unknown"; break;
   }
   scuLog( LM32_LOG_ERROR, ESC_ERROR "Exception occurred: %d -> %s\n"
                                  #ifdef CONFIG_STOP_ON_LM32_EXCEPTION
                                     "System stopped!\n"
                                  #else
                                     "Restarting application!"
                                  #endif
                           ESC_NORMAL, sig, str );
#ifdef CONFIG_STOP_ON_LM32_EXCEPTION
   irqDisable();
   while( true );
#else
   LM32_RESTART_APP();
#endif
}

/*================================== EOF ====================================*/
