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
#include "tuple_statistics.hpp"


using namespace Scu::daq;
using namespace std;

/*!----------------------------------------------------------------------------
 */
TupleStatistics::TupleStatistics( AllDaqAdministration* pParent )
   :m_pParent( pParent )
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
}

/*!----------------------------------------------------------------------------
 */
void TupleStatistics::add( FbChannel* pChannel, const TUPLE_T& rTuple )
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
   uint y = 0;
   for( auto& i: m_tupleList )
   {
      y++;
      if( !i.m_hasUpdated )
         continue;
      i.m_hasUpdated = false;
      cout << "\e[" << y << ";1H" << y << "\e[" << y << ";4H" << i.m_pChannel->getFgName()
      << " Count: " << i.m_count
      << "\e[" << y << ";16Hset: " << rTuple.m_setValue
      << "\e[" << y << ";24Hact: " << rTuple.m_actValue;
   }
   cout << endl;
}

//================================== EOF ======================================
