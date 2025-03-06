/*!
 * @file scu_logutil.c
 * @brief Pushes messages on serial port and on LM32-logsystem.
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 08.07.2022
 *
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
#include <mprintf.h>
#include <eb_console_helper.h>
#include <lm32_syslog.h>
#include <lm32Interrupts.h>
#include "scu_logutil.h"

/*! ---------------------------------------------------------------------------
 * @see scu_logutil.h
 */
OPTIMIZE( "-O1"  )
void scuLog( const unsigned int filter, const char* format, ... )
{
   va_list ap;

   va_start( ap, format );
   vprintf( format, ap );
   va_end( ap );
#ifdef CONFIG_USE_LM32LOG
   va_start( ap, format );
   vLm32log( filter, format, ap );
   va_end( ap );
#endif
}

/*! ---------------------------------------------------------------------------
 * @see scu_logutil.h
 */
void die( const char* pErrorMessage )
{
   scuLog( LM32_LOG_ERROR, ESC_ERROR
           "\nPanic: \"%s\"\n+++ LM32 stopped! +++\n" ESC_NORMAL, pErrorMessage );
#ifndef CONFIG_REINCERNATE
   irqDisable();
   while( true );
#else
   scuLog( LM32_LOG_ERROR, ESC_ERROR "...continued...\n" ESC_NORMAL );
#endif
}

/*================================== EOF ====================================*/
