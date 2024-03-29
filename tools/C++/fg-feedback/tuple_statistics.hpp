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
#include <TAverageBuilder.hpp>
#include <scu_fg_feedback.hpp>
//#include "fg-feedback.hpp"

#ifndef CONFIG_USE_TUPLE_STATISTICS
#error Macro CONFIG_USE_TUPLE_STATISTICS has to be defined in Makefile!
#endif

namespace Scu
{

///////////////////////////////////////////////////////////////////////////////
class TupleStatistics
{
   static constexpr uint MAX_SET_CONSTANT_TIMES = 1000;
   using TUPLE_T = FgFeedbackTuple::TUPLE_T;

   class FrqencyAverage: public TAverageBuilder<uint>
   {
   public:
      FrqencyAverage( void ): TAverageBuilder(60) {}
   };
   
   struct TUPLE_ITEM_T
   {
      FgFeedbackTuple*  m_pChannel;
      TUPLE_T           m_oTuple;
      uint              m_stopCount;
      uint              m_count;
      uint              m_frequency;
      FrqencyAverage    m_oAverage;
   };

   std::vector<TUPLE_ITEM_T> m_tupleList;

   FgFeedbackAdministration* m_pParent;
   bool                  m_first;
#ifdef CONFIG_MIL_FG
   bool                  m_AddacPresent;
   bool                  m_MilPresent;
#endif
   uint64_t              m_printTime;
   uint64_t              m_gateTime;

public:
   TupleStatistics( FgFeedbackAdministration* pParent );
   ~TupleStatistics( void );

   void clear( void );

   void add( FgFeedbackTuple* pChannel, const TUPLE_T& rTuple );

   void print( void );
};

} /* namespace Scu */

#endif
//================================== EOF ======================================
