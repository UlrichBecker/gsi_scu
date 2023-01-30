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

namespace EB = FeSupport::Scu::Etherbone;
using namespace EB;
using namespace std;


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


///////////////////////////////////////////////////////////////////////////////
class EB_CONNECTION: public EtherboneConnection
{
    etherbone::address_t m_ddr3If1Addr;
public:
   EB_CONNECTION( std::string netaddress, uint timeout = EB_DEFAULT_TIMEOUT );
   ~EB_CONNECTION( void );

   void     ddr3Write( const uint i, uint64_t value64 );
   void     ddr3Write( const uint i, const uint64_t* pData, uint len );
   uint64_t ddr3Read( const uint i );
   void     ddr3Read( const uint i, uint64_t* pData, const uint len );
};

/* ----------------------------------------------------------------------------
 */
EB_CONNECTION::EB_CONNECTION( std::string netaddress, uint timeout )
   :EtherboneConnection( netaddress, timeout )
{
   connect();
   m_ddr3If1Addr = findDeviceBaseAddress( EB::gsiId, EB::wb_ddr3ram );
}

/* ----------------------------------------------------------------------------
 */
EB_CONNECTION::~EB_CONNECTION( void )
{
   if( isConnected() )
      disconnect();
}

/* ----------------------------------------------------------------------------
 */
void EB_CONNECTION::ddr3Write( const uint i, uint64_t value64 )
{
#if 0
   write( m_ddr3If1Addr + i * sizeof(uint64_t), &value64,
          sizeof(uint32_t) | EB_LITTLE_ENDIAN,
          sizeof(uint64_t)/sizeof(uint32_t) );
#else
   EtherboneConnection::ddr3Write( m_ddr3If1Addr + i * sizeof(uint64_t),
                                   &value64, 1 );

#endif
}

/* ----------------------------------------------------------------------------
 */
void EB_CONNECTION::ddr3Write( const uint i, const uint64_t* pData, uint len )
{
   EtherboneConnection::ddr3Write( m_ddr3If1Addr + i, pData, len );
}

/* ----------------------------------------------------------------------------
 */
uint64_t EB_CONNECTION::ddr3Read( const uint i )
{
   uint64_t ret = 0;

   read( m_ddr3If1Addr + i * sizeof(uint64_t), &ret,
         sizeof(uint32_t) | EB_LITTLE_ENDIAN,
         sizeof(uint64_t)/sizeof(uint32_t) );
   return ret;
}

/* ----------------------------------------------------------------------------
 */
void EB_CONNECTION::ddr3Read( const uint i, uint64_t* pData, const uint len )
{
   read( m_ddr3If1Addr + i * sizeof(uint64_t), pData,
         sizeof(uint32_t) | EB_LITTLE_ENDIAN,
         len * sizeof(uint64_t)/sizeof(uint32_t) );
}

/* ----------------------------------------------------------------------------
 */
bool ioTest( EB_CONNECTION& roEbc, uint i, uint64_t pattern )
{
   cout << "writing pattern: 0x" << setfill( '0' ) << setw( 16 ) << hex
        << uppercase << pattern << endl;
   roEbc.ddr3Write( i, pattern );

   uint64_t recPattern = roEbc.ddr3Read( i );
   cout << "reading pattern: 0x" << setfill( '0' ) << setw( 16 ) << hex
        << uppercase << recPattern << endl;

   if( pattern != recPattern )
   {
      cout << ESC_FG_RED "Failed!" ESC_NORMAL << endl;
      return true;
   }

   cout << ESC_FG_GREEN ESC_BOLD "Pass!" ESC_NORMAL << endl;
   return false;
}

/* ----------------------------------------------------------------------------
 */
bool arrayTest( EB_CONNECTION& roEbc, const uint offset )
{
   cout << "Writing array of " << dec << ARRAY_SIZE(g_testArray) << " items." << endl;

   roEbc.ddr3Write( offset, g_testArray, ARRAY_SIZE(g_testArray) );

   uint64_t targetArray[ARRAY_SIZE(g_testArray)];
   ::memset( targetArray, 0, sizeof(targetArray) );


   if( ::memcmp( targetArray, g_testArray, sizeof(g_testArray) ) != 0 )
   {
      cout << ESC_FG_RED "Failed!" ESC_NORMAL << endl;
      return true;
   }
   cout << ESC_FG_GREEN ESC_BOLD "Pass!" ESC_NORMAL << endl;
   return false;
}

///////////////////////////////////////////////////////////////////////////////
/* ----------------------------------------------------------------------------
 */
void run( std::string& ebName )
{
   EB_CONNECTION oEbc( ebName );

   ioTest( oEbc, 1, 0x1122334455667788 );
   ioTest( oEbc, 1, 0xAAAAAAAA55555555 );
   ioTest( oEbc, 1, 0xF0F0F0F0F0F0F0F0 );

   arrayTest( oEbc, 10000000 );
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
