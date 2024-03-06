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
#include "fg-feedback.hpp"
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
   clear();
}

/*!----------------------------------------------------------------------------
 */
TupleStatistics::~TupleStatistics( void )
{
   DEBUG_MESSAGE_M_FUNCTION( "" );
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
      i.m_count++;
      i.m_hasUpdated = true;
      print( rTuple );
      return;
   }

   m_tupleList.push_back(
   {
      .m_pChannel   = pChannel,
      .m_count      = 1,
      .m_hasUpdated = true
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

   print( rTuple );
}

/*!----------------------------------------------------------------------------
 */
void TupleStatistics::print( const TUPLE_T& rTuple )
{
   if( m_first )
   {
      m_first = false;
      cout << ESC_CLR_SCR << flush;
   }
   uint y = 0;
   for( auto& i: m_tupleList )
   {
      y++;
      if( !i.m_hasUpdated )
         continue;
      i.m_hasUpdated = false;
      cout << "\e[" << y << ";1H" ESC_CLR_LINE << y
           << "\e[" << y << ";4H" << i.m_pChannel->getFgName()
           << "\e[" << y << ";16HCount: " << i.m_count
           << "\e[" << y << ";34Hset: " << daq::rawToVoltage(rTuple.m_setValue) << " V"
           << "\e[" << y << ";48Hact: " << daq::rawToVoltage(rTuple.m_actValue) << " V";
   }
   cout << "\e[" << y << ";1H" << endl;

#ifdef CONFIG_MIL_FG
   if( m_MilPresent )
   {
      const auto level = static_cast<float>(m_pParent->getMilFiFoLevelPerTenThousand()) / 100.0;
      if( level > 98.0 )
         cout << ESC_ERROR;
      else if( level > 90.0 )
         cout << ESC_WARNING;
      cout << "MIL-DAQ- FiFo- level: " << fixed << setprecision(2) << setw( 6 ) << level << '%'
           << ESC_NORMAL << endl;
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
