/*!
 *  @file daq_statistics.cpp
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
#ifndef CONFIG_USE_ADDAC_DAQ_BLOCK_STATISTICS
 #error Macro CONFIG_USE_ADDAC_DAQ_BLOCK_STATISTICS has to be defined in Makefile!
#endif

#include "daq_statistics.hpp"
#include <scu_function_generator.h>
#include <message_macros.hpp>
#include <iomanip>
#include <algorithm>

using namespace Scu::daq;
using namespace std;

/*!----------------------------------------------------------------------------
 */
Statistics::Statistics( FgFeedbackAdministration* pParent, const USEC_T printInterval )
   :m_hasUpdated( false )
   ,m_printInterval( printInterval )
   ,m_nextPrintTime( 0 )
   ,m_pParent( pParent )
{
   DEBUG_MESSAGE_M_FUNCTION();
   clear();
}

/*!----------------------------------------------------------------------------
 */
Statistics::~Statistics( void )
{
   DEBUG_MESSAGE_M_FUNCTION();
}

/*!----------------------------------------------------------------------------
 */
void Statistics::clear( void )
{
   m_daqChannelList.clear();
   m_nextPrintTime = 0;
}

/*!----------------------------------------------------------------------------
 */
void Statistics::add( DAQ_DESCRIPTOR_T& rDescriptor )
{
   m_hasUpdated = true;
   const uint serialNumber = static_cast<uint>(daqDescriptorGetSlot( &rDescriptor )) * 100 +
                             static_cast<uint>(daqDescriptorGetChannel(&rDescriptor));

   for( auto& i: m_daqChannelList )
   {
      if( i.m_serialNumber == serialNumber )
      {
         i.m_counter++;
         i.m_counterUpdated = true;
         return;
      }
   }

   if( m_daqChannelList.size() > (MAX_FG_CHANNELS * 2) )
   {
      ERROR_MESSAGE( "Received DAQ-block out of maximum possible DAQ channels of: " << (MAX_FG_CHANNELS * 2) );
      return;
   }

   const BLOCK_T newBlock =
   {
      .m_serialNumber   = serialNumber,
      .m_slot           = static_cast<uint>(daqDescriptorGetSlot( &rDescriptor )),
      .m_channel        = static_cast<uint>(daqDescriptorGetChannel( &rDescriptor )),
      .m_counter        = 1,
      .m_counterUpdated = true
   };
   m_daqChannelList.push_back( newBlock );
   if( m_daqChannelList.size() > 1 )
   {
      sort( m_daqChannelList.begin(), m_daqChannelList.end(),
            []( BLOCK_T& a, BLOCK_T& b ) -> bool
            {
               return a.m_serialNumber < b.m_serialNumber;
            });
   }
}

/*!----------------------------------------------------------------------------
 */
void Statistics::print( void )
{
   if( !m_hasUpdated )
      return;

   const USEC_T time = getSysMicrosecs();
   if( time < m_nextPrintTime )
      return;
   m_nextPrintTime = time + m_printInterval;

   m_hasUpdated = false;

   cout << ESC_CLR_SCR << flush;
   uint y = 0;
   for( auto& i: m_daqChannelList )
   {
      y++;
      if( i.m_counterUpdated )
      {
         i.m_counterUpdated = false;
         cout << ESC_BOLD ESC_FG_GREEN;
      }
      else
         cout << ESC_NORMAL ESC_FG_BLUE;

      cout << "\e[" << y << ";1H" << y << "\e[" << y << ";4HSlot: " << i.m_slot <<
              ",\e[" << y << ";14HChannel: " << i.m_channel <<
              ", received: " << i.m_counter;
   }
   cout << ESC_NORMAL << endl;

   float level = static_cast<float>(m_pParent->getAddacBufferLevel()) * 100.0 /
                 static_cast<float>(m_pParent->getAddacBufferCapacity());

   if( level > 90.0 )
      cout << ESC_WARNING;
   cout << "FiFo- level: " << fixed << setprecision(2) << setw( 6 ) << level << '%'
        << ESC_NORMAL << endl;
}

//================================== EOF=======================================
