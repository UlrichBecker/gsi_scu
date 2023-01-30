/*!
 * @file EB_TEST_TEMPLATE.cpp
 * @brief Template for Linux-tests with etherbone/wishbone.
 * @date  30.01.2023
 * @author Ulrich Becker
 */
#include <exception>
#include <iomanip>
#include <EtherboneConnection.hpp>
#include <daqt_messages.hpp>
#include <scu_env.hpp>

namespace EB = FeSupport::Scu::Etherbone;
using namespace EB;

class EB_CONNECTION: public EtherboneConnection
{
public:
   EB_CONNECTION( std::string netaddress, uint timeout = EB_DEFAULT_TIMEOUT );
   ~EB_CONNECTION( void );
};

EB_CONNECTION::EB_CONNECTION( std::string netaddress, uint timeout )
   :EtherboneConnection( netaddress, timeout )
{
   connect();
}

EB_CONNECTION::~EB_CONNECTION( void )
{
   if( isConnected() )
      disconnect();
}

///////////////////////////////////////////////////////////////////////////////
/* ----------------------------------------------------------------------------
 */
void run( std::string& ebName )
{
   EB_CONNECTION oEbc( ebName );
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
