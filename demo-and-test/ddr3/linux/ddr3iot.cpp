/*!
 * @file ddr3iot.cpp
 * @brief Test program for writing and reading the DDR3 RAM via the
 *        FE-class EtherboneConnection
 * @date  30.01.2023
 * @author Ulrich Becker
 */
#include <exception>
#include <iomanip>
#include <string.h>
#include <eb_console_helper.h>
#include <EtherboneConnection.hpp>
#include <daqt_messages.hpp>
#include <scu_env.hpp>
#include <daq_calculations.hpp>
#include <scu_ddr3_access.hpp>

namespace EB = FeSupport::Scu::Etherbone;
using namespace EB;
using namespace std;
using namespace Scu;
using namespace Scu::daq;

static const uint64_t g_testArray[10] =
{
   0x0000000000000000,
   0x1111111111111111,
   0x2222222222222222,
   0x3333333333333333,
   0x4444444444444444,
   0x5555555555555555,
   0x6666666666666666,
   0x7777777777777777,
   0x8888888888888888,
   0x9999999999999999
};

/* ----------------------------------------------------------------------------
 */
bool ioTest( RamAccess* poRam, uint i, uint64_t pattern, bool burst = false )
{

   cout << "writing pattern: 0x" << setfill( '0' ) << setw( 16 ) << hex
        << uppercase << pattern << dec << " at index: " << i << endl;
   poRam->write( i, &pattern, 1 );

   uint64_t recPattern = 0;
   poRam->read( i, &recPattern, 1, burst );
   cout << "reading pattern: 0x" << setfill( '0' ) << setw( 16 ) << hex
        << uppercase << recPattern << endl;

   if( pattern != recPattern )
   {
      cout << ESC_FG_RED "Failed!" ESC_NORMAL << endl;
      return true;
   }

   cout << ESC_FG_GREEN ESC_BOLD "Pass!" ESC_NORMAL "\n" << endl;
   return false;
}

/* ----------------------------------------------------------------------------
 */
bool arrayTest( RamAccess* poRam, const uint offset, const bool burst = false )
{
   cout << "Writing array of " << dec << ARRAY_SIZE(g_testArray)
        <<  " items"  << endl;

   poRam->write( offset, g_testArray, ARRAY_SIZE(g_testArray) );

   uint64_t targetArray[ARRAY_SIZE(g_testArray)];
   ::memset( targetArray, 0, sizeof(targetArray) );
   cout << "Reading array of " << dec << ARRAY_SIZE(targetArray) << " items." << endl;

   poRam->read( offset, targetArray, ARRAY_SIZE(targetArray), burst );

   if( ::memcmp( targetArray, g_testArray, sizeof(g_testArray) ) != 0 )
   {
      cout << ESC_FG_RED "Failed!" ESC_NORMAL << endl;
      return true;
   }

   cout << ESC_FG_GREEN ESC_BOLD "Pass!" ESC_NORMAL << endl;
   return false;
}

/* ----------------------------------------------------------------------------
 */
void bigDataTest( RamAccess* poRam, uint size, bool burst )
{
   uint64_t* pSendBuffer = new uint64_t[size];

   for( uint i = 0; i < size; i++ )
   {
      pSendBuffer[i] = i;
   }

   cout << "Writing array of " << dec << size << " 64 bit words." << endl;
   poRam->write( 0, pSendBuffer, size );

   uint64_t* pReceiveBuffer = new uint64_t[size];
   ::memset( pReceiveBuffer, 0, size * sizeof(uint64_t) );

   cout << "Reading array of " << dec << size << " 64 bit words." << endl;

   uint64_t startTime = getSysMicrosecs();
   poRam->read( 0, pReceiveBuffer, size, burst );
   startTime = getSysMicrosecs() - startTime;
   cout << "Duration: " << startTime << " us" << endl;

   if( ::memcmp( pReceiveBuffer, pSendBuffer, size * sizeof(uint64_t) ) != 0 )
      cout << ESC_FG_RED "Failed!" ESC_NORMAL << endl;
   else
      cout << ESC_FG_GREEN ESC_BOLD "Pass!" ESC_NORMAL << endl;


   delete[] pSendBuffer;
   delete[] pReceiveBuffer;
}

/* ----------------------------------------------------------------------------
 */
void run( std::string& ebName )
{
   Ddr3Access oDdr3( ebName );

   ioTest( &oDdr3, 5, 0x1122334455667788, true );
   ioTest( &oDdr3, 5, 0xAAAAAAAA55555555, true );
   ioTest( &oDdr3, 1, 0xF0F0F0F0F0F0F0F0, true );
   ioTest( &oDdr3, 1, 0xFFFFFFFF00000000, true  );

   arrayTest( &oDdr3, 2000000, true );

   bigDataTest( &oDdr3,200, true );
   bigDataTest( &oDdr3,200, false );
}

//=============================================================================
int main( int argc, char** ppArgv )
{
   try
   {
      std::string ebName;
      if( Scu::isRunningOnScu() )
      {
         ebName = "dev/wbm0";
      }
      else
      {
         if( argc < 2 )
         {
            ERROR_MESSAGE( "Missing SCU- name!" );
            return EXIT_FAILURE;
         }
         ebName = "tcp/";
         ebName += ppArgv[1];
      }
      run( ebName );
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
