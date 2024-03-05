/*!
 *  @file tuple_statistics.hpp
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
#ifndef _TUPLE_STATISTICS_HPP
#define _TUPLE_STATISTICS_HPP
#include <vector>
#include <functional>
#include <iostream>
#include "fg-feedback.hpp"

#ifndef CONFIG_USE_TUPLE_STATISTICS
#error Macro CONFIG_USE_TUPLE_STATISTICS has to be defined in Makefile!
#endif

namespace Scu
{
namespace daq
{

///////////////////////////////////////////////////////////////////////////////
class TupleStatistics
{
   struct TUPLE_ITEM_T
   {
      FbChannel*                m_pChannel;
      uint                      m_count;
      bool                      m_hasUpdated;
   };

   std::vector<TUPLE_ITEM_T> m_tupleList;

   AllDaqAdministration* m_pParent;
#ifdef CONFIG_MIL_FG
   bool                  mAddacPresent;
   bool                  mMilPresent;
#endif
public:
   using TUPLE_T = FgFeedbackTuple::TUPLE_T;

   TupleStatistics( AllDaqAdministration* pParent );
   ~TupleStatistics( void );

   void clear( void );

   void add( FbChannel* pChannel, const TUPLE_T& rTuple );

   void print( const TUPLE_T& rTuple );
};

} /* namespace daq */
} /* namespace Scu */

#endif
//================================== EOF ======================================
