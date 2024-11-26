/*!
 *  @file mem_browser.cpp
 *  @brief Browser module of memory monitor.
 *  @see https://www-acc.gsi.de/wiki/Frontend/Memory_Management_On_SCU
 *  @date 12.04.2022
 *  @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#include <iomanip>
#include <message_macros.hpp>
#include "mem_browser.hpp"

using namespace Scu::mmu;
using namespace std;

/*!----------------------------------------------------------------------------
 */
Browser::Browser( RamAccess* poRam, CommandLine& rCmdLine )
   :Mmu( poRam )
   ,m_rCmdLine( rCmdLine )
{
}

/*!----------------------------------------------------------------------------
 */
Browser::~Browser( void )
{
}

/*!----------------------------------------------------------------------------
 */
void Browser::checkMmuPresent( void )
{
   if( isPresent() )
      return;
   throw runtime_error( "No MMU found on this SCU!" );
}

/*!----------------------------------------------------------------------------
 */
int Browser::operator()( std::ostream& out )
{
   checkMmuPresent();

   MMU_ITEM_T currentItem;
   currentItem.iNext = 0;
   uint level = 0;

   string separator;
   if( m_rCmdLine.isVerbose() )
   {
      out << "\n  tag   |  begin   |   end    |   size   |  consumption\n";
      out <<   "--------+----------+----------+----------+--------------\n";
      separator = " |";
   }
   else
      separator = ", ";

   const uint factor      = m_rCmdLine.isInBytes()? sizeof(RAM_PAYLOAD_T) : 1;
   const uint wide        = 9; //m_rCmdLine.isInBytes()? 9 : 8;
   const uint maxCapacity = getMaxCapacity64();
   do
   {
      readNextItem( currentItem );
      if( level > 0 )
      {
         if( m_rCmdLine.isTagInDecimal() )
            out << "  " << setw( 5 );
         else
            out << " 0x" << hex << uppercase << setw( 4 ) << setfill('0');

         out << currentItem.tag << separator << dec;

         out << setfill( ' ' ) << setw( wide ) << currentItem.iStart * factor  << separator;
         out << setfill( ' ' ) << setw( wide ) << (currentItem.iStart + currentItem.length-1) * factor << separator;
         out << setfill( ' ' ) << setw( wide ) << currentItem.length * factor << separator;

         float size = (static_cast<float>( MMU_ITEMSIZE + currentItem.length) * 100.0)
                      / static_cast<float>(maxCapacity);
         out << fixed << setprecision(6) << setw( 10 ) << size << '%';

         out << endl;
      }
      level += MMU_ITEMSIZE + currentItem.length;
   }
   while( (currentItem.iNext != 0) && (level <= maxCapacity) );

   if( currentItem.iNext != 0 )
   {
      throw runtime_error( "No end of list found. MMU could be corrupt!" );
   }

   float size = (static_cast<float>(level+MMU_ITEMSIZE) * 100.0)
                      / static_cast<float>(maxCapacity);

   const uint NETTO_MAX = maxCapacity - MMU_ITEMSIZE;
   if( m_rCmdLine.isVerbose() )
   {
      out << "========================================================\n";
      out << "total:       "
          << level * factor << " of " << NETTO_MAX *  factor << ",\n"
          << "free:        " << (NETTO_MAX - level) * factor << ",\n"
          << "capacity:    " << maxCapacity * factor << ",\n"
          << "consumption: " << fixed << setprecision(6) << setw( 10 ) << size << '%' << endl;
   }
   else
   {
      out << level * factor << "/" << NETTO_MAX * factor << ", "
          << (NETTO_MAX - level) * factor << ", "
          << maxCapacity * factor << ", "
          << fixed << setprecision(6) << setw( 10 ) << size << '%' << endl;
   }

   return 0;
}

//================================== EOF ======================================
