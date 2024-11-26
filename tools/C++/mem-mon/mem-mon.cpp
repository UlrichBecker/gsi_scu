/*!
 *  @file mem-mon.cpp
 *  @brief Main module for the SCU memory monitor.
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
#include <exception>
#include <cstdlib>
#include <memory>
#include <message_macros.hpp>
#include <scu_ddr3_access.hpp>
#include <scu_sram_access.hpp>
#include <BusException.hpp>
#include "mem_cmdline.hpp"
#include "mem_browser.hpp"

#ifndef CONFIG_OECORE_SDK_VERSION
   #warning "CAUTION: Module becomes not build by YOCTO SDK !"
#endif

using namespace std;
using namespace Scu::mmu;
namespace EB = FeSupport::Scu::Etherbone;

#ifndef CONFIG_OECORE_SDK_VERSION
/*! ---------------------------------------------------------------------------
 */
void onUnexpectedException( void )
{
   ERROR_MESSAGE( "Unexpected exception occurred!" );
   throw 0;     // throws int (in exception-specification)
}
#endif

/*! ---------------------------------------------------------------------------
 */
int main( int argc, char** ppArgv )
{
#ifndef CONFIG_OECORE_SDK_VERSION
   set_unexpected( onUnexpectedException );
#endif
   try
   {
      CommandLine oCmdLine( argc, ppArgv );
      oCmdLine();
      Scu::RamAccess* _pRam = nullptr;
      try
      {
         _pRam = new Scu::Ddr3Access( oCmdLine.getScuUrl() );
         DEBUG_MESSAGE( "Using DDR3-RAM on SCU3" );
      }
      catch( EB::BusException& e )
      {
         string exceptText = e.what();
         if( exceptText.find( "VendorId" ) == string::npos )
            throw EB::BusException( e );

         _pRam = new Scu::SramAccess( oCmdLine.getScuUrl() );
         DEBUG_MESSAGE( "Using SRAM on SCU4" );
      }
      unique_ptr<Scu::RamAccess> pRam( _pRam );

      Browser browse( pRam.get(), oCmdLine );
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

         if( oCmdLine.getSegmentVect().empty() )
            return EXIT_SUCCESS;
      }

      if( !oCmdLine.getSegmentVect().empty() )
      {
         for( const auto& seg: oCmdLine.getSegmentVect() )
         {
            if( oCmdLine.isVerbose() )
            {
               cout << "Creating memory segment with tag: 0x"
                    << hex << uppercase << seg.m_tag
                    << ", size: " << dec << seg.m_size << endl;
            }
            MMU_ADDR_T addr;
            size_t len = seg.m_size;
            const MMU_STATUS_T status = browse.allocate( seg.m_tag, addr, len, true );
            if( (status == ALREADY_PRESENT) && (len != seg.m_size) )
            {
               WARNING_MESSAGE( "Memory segment 0x" << hex << uppercase << seg.m_tag << " already allocated!"
                                " Requested segment memory space: " << dec << seg.m_size
                                 << ", actual segment memory space: " << len );

            }
            else if( !browse.isOkay( status ) )
            {
               ERROR_MESSAGE( browse.status2String( status ) );
               return EXIT_FAILURE;
            }
         }

         if( oCmdLine.isDoExit() )
            return EXIT_SUCCESS;
      }

      browse( cout );
   }
   catch( std::exception& e )
   {
      if( e.what()[0] == '\0' )
         return EXIT_SUCCESS;
      ERROR_MESSAGE( e.what() );
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
