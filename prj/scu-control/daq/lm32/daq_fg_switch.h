/*!
 *  @file daq_fg_switch.h
 *  @brief Module for switching both DAQ channels for set- and actual values,
 *         belonging to the concerned function generator, on or off.
 *
 *  @date 22.11.2023
 *  @copyright (C) 2023 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
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
#ifndef _DAQ_FG_SWITCH_H
#define _DAQ_FG_SWITCH_H

#ifdef __cplusplus
extern "C" {
#endif

/*! ---------------------------------------------------------------------------
 * @brief Enables the feedback-channel for the given function generator number
 *        in the given slot.
 *
 * Counterpart to daqDisableFgFeedBack
 * @see daqDisableFgFeedBack
 * @param slot SCU-bus slot number
 * @param fgNum Function generator number 0 or 1
 * @param tag ECA-tag for trigger condition
 */
void daqEnableFgFeedback( const unsigned int slot, const unsigned int fgNum,
                          const uint32_t tag );

/*! ---------------------------------------------------------------------------
 * @brief Disables the feedback-channel for the given function generator number
 *        in the given slot.
 *
 * Counterpart to daqEnableFgFeedback
 * @see daqEnableFgFeedback
 * @param slot SCU-bus slot number
 * @param fgNum Function generator number 0 or 1
 */
void daqDisableFgFeedback( const unsigned int slot, const unsigned int fgNum );

#ifdef __cplusplus
}
#endif
#endif
/*================================== EOF ====================================*/
