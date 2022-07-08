/*!
 * @file scu_logutil.h
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
#ifndef _SCU_LOGUTIL_H
#define _SCU_LOGUTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup LM32_LOG
 * @ingroup PRINTF
 * @brief Sends a log-message via vLm32Log() and vprintf().
 * @note This function is only during the initialization allowed!
 */
void scuLog( const unsigned int filter, const char* format, ... );

/*! ---------------------------------------------------------------------------
 * @ingroup LM32_LOG
 * @brief Prints a error-message via UART and stop the LM32 firmware.
 * @param pErrorMessage String containing a error message.
 */
void die( const char* pErrorMessage );

#ifdef __cplusplus
}
#endif
#endif /* _SCU_LOGUTIL_H */
/*================================== EOF ====================================*/