/*!
 *  @file daq_statistics.hpp
 *  @brief Module makes a statistic of all incoming ADDAC daq-blocks. 
 *
 *  @date 15.11.2023
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
#ifndef _DAQ_STATISTICS_HPP
#define _DAQ_STATISTICS_HPP

#include <daq_descriptor.h>
#include <daq_calculations.hpp>
#include <scu_fg_feedback.hpp>
#include <vector>
#include <iostream>

namespace Scu
{
namespace daq
{

///////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
 * @brief Class makes a statistic of all incoming ADDAC-DAQ data-blocks and
 *        prints the result in stdout.
 */
class Statistics
{
   /*!
    * @brief Data type of a item in the DAQ channel list.
    */
   struct BLOCK_T
   {  /*!
       * @brief The serial number helps in finding and
       *        to sort the channel items in the list.
       */
      uint m_serialNumber;

      /*!
       * @brief SCU-bus slot respectively slave number.
       */
      uint m_slot;

      /*!
       * @brief DAQ- channel number of slave device.
       */
      uint m_channel;

      /*!
       * @brief Number of currently received DAQ blocks.
       */
      uint m_counter;

      /*!
       * @brief Becomes "true" if the counter was updated.
       */
      bool m_counterUpdated;
   };

   /*!
    * @brief List of recognized active DAQ channels.
    */
   std::vector<BLOCK_T>  m_daqChannelList;
   
   /*!
    * @brief Becomes true by every call of function "add())",
    *        becomes false by every call of function "print()".
    */
   bool m_hasUpdated;

   /*!
    * @brief Minimum time in microseconds for the next print- time.
    */
   const USEC_T m_printInterval;

   /*!
    * @brief Time in the future for the next print.
    */
   USEC_T m_nextPrintTime;

   /*!
    * @brief Pointer to parent object this will used at to show the memory level.
    */
   FgFeedbackAdministration* m_pParent;

public:
   /*!
    * @brief Constructor
    */
   Statistics( FgFeedbackAdministration* pParent, const USEC_T printInterval = 1000000 );

   /*!
    * @brief Destructor
    */
   ~Statistics( void );
   
   /*!
    * @brief Adds the DAQ-descriptor to the channel list if not already
    *        present, or increments the block-counter if
    *        was found in the list.
    * @param rDescriptor Reference of receives descriptor.
    */
   void add( DAQ_DESCRIPTOR_T& rDescriptor );
   
   /*!
    * @brief Clears the DAQ- channel list.
    */
   void clear( void );
   
   /*!
    * @brief Prints the content of the channel list in stdout.
    */
   void print( void );
};

} // namespace daq
} // namespace Scu
#endif
//================================== EOF ======================================
