/*!
 *  @file mem-mon.cpp
 *  @brief Main module for the SCU memory monitor.
 *
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
#include <exception>
#include <cstdlib>
#include <message_macros.hpp>
#include <scu_ddr3_access.hpp>
#include "mem_cmdline.hpp"
#include "mem_browser.hpp"

using namespace std;
using namespace Scu::mmu;

/*! ---------------------------------------------------------------------------
 */
void onUnexpectedException( void )
{
   ERROR_MESSAGE( "Unexpected exception occurred!" );
   throw 0;     // throws int (in exception-specification)
}


/*! ---------------------------------------------------------------------------
 */
int main( int argc, char** ppArgv )
{
   set_unexpected( onUnexpectedException );
   try
   {
      CommandLine oCmdLine( argc, ppArgv );

      /*!
       * @todo Checking in the future whether it's a SCU3 or a SCU4 and
       *       create the appropriate object. In the case of SCU3
       *       it's the object of DDR3-RAM like now.
       */
      Scu::Ddr3Access oDdr3( oCmdLine() );

      Browser browse( &oDdr3, oCmdLine );
      if( oCmdLine.isDelete() )
      {
         if( browse.isPresent() )
         {
            if( oCmdLine.isVerbose() )
               cout << "Deleting memory management partitions!" << endl;
            browse.clear();
         }
         else
            WARNING_MESSAGE( "No memory management found!" );

         if( oCmdLine.getRequestedSize() == 0 )
            return EXIT_SUCCESS;
      }

      if( oCmdLine.getRequestedSize() != 0 )
      {
         for( const auto& seg: oCmdLine.getSegmentVect() )
         {
            cout << seg.m_tag; //TODO!!!!
         }

         if( oCmdLine.isVerbose() )
         {
            cout << "Creating memory segment with tag: 0x"
                 << hex << uppercase << oCmdLine.getNewTag()
                 << ", size: " << dec << oCmdLine.getRequestedSize() << endl;
         }
         MMU_ADDR_T addr;
         size_t len = oCmdLine.getRequestedSize();
         const MMU_STATUS_T status = browse.allocate( oCmdLine.getNewTag(), addr, len, true );
         if( status == ALREADY_PRESENT )
         {
            WARNING_MESSAGE( "Memory segment already allocated!"
                             " Requested segment memory space: " << oCmdLine.getRequestedSize()
                             << ", actual segment memory space: " << len );
         }
         else if( status != OK )
         {
            ERROR_MESSAGE( browse.status2String( status ) );
            return EXIT_FAILURE;
         }

         if( oCmdLine.isDoExit() )
            return EXIT_SUCCESS;
      }

      browse( cout );
   }
   catch( std::exception& e )
   {
      ERROR_MESSAGE( "std::exception occurred: \"" << e.what() << '"' );
      return EXIT_FAILURE;
   }
   catch( ... )
   {
      ERROR_MESSAGE( "Undefined exception occurred!" );
      return EXIT_FAILURE;
   }

   return EXIT_SUCCESS;
}

//================================== EOF ======================================
