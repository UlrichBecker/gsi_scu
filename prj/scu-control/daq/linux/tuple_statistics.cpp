/*!
 *  @file tuple_statistics.cpp
 *  @brief Module makes a statistic of all incoming feedback tupled.
 *
 *  @date 04.03.2024
 *  @copyright (C) 2024 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#include <algorithm>
#include <iomanip>
#include <scu_fg_feedback.hpp>
#include "tuple_statistics.hpp"


using namespace Scu;
using namespace std;

/*!----------------------------------------------------------------------------
 */
TupleStatistics::TupleStatistics( FgFeedbackAdministration* pParent )
   :m_pParent( pParent )
   ,m_first( true )
#ifdef CONFIG_MIL_FG
   ,m_AddacPresent( false )
   ,m_MilPresent( false )
#endif
{
   DEBUG_MESSAGE_M_FUNCTION( "" );
   m_printTime = daq::getSysMicrosecs();
   m_gateTime = m_printTime;
#ifdef CONFIG_MIL_FG
   m_AddacPresent = m_pParent->getNumOfFoundNonMilFg() > 0;
   m_MilPresent   = m_pParent->getNumOfFoundMilFg() > 0;
#endif
   clear();
}

/*!----------------------------------------------------------------------------
 */
TupleStatistics::~TupleStatistics( void )
{
   DEBUG_MESSAGE_M_FUNCTION( "" );
#ifndef CONFIG_DEBUG_MESSAGES
   if( !m_first )
      cout << ESC_CLR_SCR << gsi::setxy() << flush;
#endif
}

/*!----------------------------------------------------------------------------
 */
void TupleStatistics::clear( void )
{
   m_tupleList.clear();
   m_first = true;
}

/*!----------------------------------------------------------------------------
 */
void TupleStatistics::add( FgFeedbackTuple* pChannel, const TUPLE_T& rTuple )
{
   for( auto& i: m_tupleList )
   {
      if( i.m_pChannel != pChannel )
         continue;

      /*
       * Detecting whether a function generator has stopped.
       */
      if( rTuple.m_setValue == i.m_oTuple.m_setValue )
      {
         if( i.m_stopCount < MAX_SET_CONSTANT_TIMES )
            i.m_stopCount++;
      }
      else
         i.m_stopCount = 0;

      i.m_oTuple = rTuple;
      i.m_count++;
      return;
   }

   m_tupleList.push_back(
   {
      .m_pChannel      = pChannel,
      .m_oTuple        = rTuple,
      .m_stopCount     = 0,
      .m_count         = 1,
      .m_frequency     = 0
   });

#ifdef CONFIG_MIL_FG
   if( pChannel->isMil() )
      m_MilPresent = true;
   else
      m_AddacPresent = true;
#endif

   if( m_tupleList.size() > 1 )
   {
      sort( m_tupleList.begin(), m_tupleList.end(),
            []( const TUPLE_ITEM_T& a, const TUPLE_ITEM_T& b ) -> bool
            {
               if( a.m_pChannel->getSocket() == b.m_pChannel->getSocket() )
                  return a.m_pChannel->getFgNumber() < b.m_pChannel->getFgNumber();
               return a.m_pChannel->getSocket() < b.m_pChannel->getSocket();
            });
   }
}

/*!----------------------------------------------------------------------------
 */
void TupleStatistics::print( void )
{
   uint64_t time = daq::getSysMicrosecs();
   if( m_printTime >= time )
      return;
   m_printTime = time + daq::MICROSECS_PER_SEC / 4;

   const uint64_t deltaTime = time - m_gateTime;
   if( deltaTime >= daq::MICROSECS_PER_SEC )
   {
      m_gateTime = time;
   }

   if( m_first )
   {
      m_first = false;
      cout << ESC_CLR_SCR << flush;
   }
   uint y = 0;
   for( auto& i: m_tupleList )
   {
      y++;
      /*!
       * Has this function generator stopped?
       */
      if( i.m_stopCount >= MAX_SET_CONSTANT_TIMES )
         cout << ESC_FG_RED;
      else if( i.m_oAverage.isValid() ) 
         cout << ESC_FG_GREEN;
      else
         cout << ESC_FG_BLUE;

      if( m_gateTime == time )
      {
         i.m_frequency = i.m_count * daq::MICROSECS_PER_SEC / deltaTime;
         i.m_count = 0;
      }
      cout << gsi::setxy( 1, y ) << ESC_CLR_LINE << y
           << gsi::setxy( 4, y ) << i.m_pChannel->getFgName()
           << gsi::setxy( 16, y ) << "Tuples: " << i.m_oAverage( i.m_frequency ) << " Hz"
           << gsi::setxy( 34, y ) << "set: " << i.m_pChannel->convertFromRawValue(i.m_oTuple.m_setValue) << " " << i.m_pChannel->getUnit()
           << gsi::setxy( 58, y ) << "act: " << i.m_pChannel->convertFromRawValue(i.m_oTuple.m_actValue) << " " << i.m_pChannel->getUnit()
           << ESC_NORMAL;
   }
   cout << gsi::setxy( 1, y ) << endl;

   {
      static uint i = 0;
      static const char fan[] = { '|', '/', '-', '\\' };
      cout << fan[i++] << ESC_CLR_LINE << endl;
      i %= ARRAY_SIZE( fan );
   }

#ifdef CONFIG_MIL_FG
   if( m_MilPresent )
   {
      const auto level = static_cast<float>(m_pParent->getMilFiFoLevelPerTenThousand()) / 100.0;
      if( level > 98.0 )
         cout << ESC_ERROR;
      else if( level > 90.0 )
         cout << ESC_WARNING;
      cout << "MIL-DAQ- FiFo- level: " << fixed << setprecision(2) << setw( 6 ) << level << '%'
           << ESC_NORMAL ESC_CLR_LINE << endl;
   }
   if( m_AddacPresent )
   {
#endif
      const auto level = static_cast<float>(m_pParent->getAddacFiFoLevelPerTenThousand()) / 100.0;
      if( level > 98.0 )
         cout << ESC_ERROR;
      else if( level > 90.0 )
         cout << ESC_WARNING;
      cout << "ADDAC-DAQ- FiFo- level: " << fixed << setprecision(2) << setw( 6 ) << level << '%'
           << ESC_NORMAL << endl;
#ifdef CONFIG_MIL_FG
   }
#endif
}

//================================== EOF ======================================
