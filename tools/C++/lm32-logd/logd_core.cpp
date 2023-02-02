/*!
 *  @file logd_core.cpp
 *  @brief Main functionality of LM32 log daemon.
 *
 *  @date 21.04.2022
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
#ifndef __DOCFSM__
 #include <iomanip>
 #include <iostream>
 #include <sstream>
 #include <syslog.h>
 #include <scu_mmu_tag.h>
 #include <daq_calculations.hpp>
 #include <daqt_messages.hpp>
 #include "logd_core.hpp"
#endif

/*!
 * @see https://github.com/UlrichBecker/DocFsm
 */
#define FSM_DECLARE_STATE( newState, attr... ) newState

/*!
 * @see https://github.com/UlrichBecker/DocFsm
 */
#define FSM_INIT_FSM( initState, attr... ) STATE_T state = initState

/*!
 * @see https://github.com/UlrichBecker/DocFsm
 */
#define FSM_TRANSITION( target, attr... ) { state = target; break; }

/*!
 * @see https://github.com/UlrichBecker/DocFsm
 */
#define FSM_TRANSITION_NEXT( target, attr... ) { state = target; next = true; break; }

/*!
 * @see https://github.com/UlrichBecker/DocFsm
 */
#define FSM_TRANSITION_SELF( attr...) break


using namespace Scu;
using namespace std;

constexpr uint LM32_OFFSET   = 0x10000000;
constexpr uint BUILD_ID_ADDR = LM32_OFFSET + 0x100;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief Central output function.
 */
int Lm32Logd::StringBuffer::sync( void )
{
   static const char* errorMessage = "ERROR: lm32-logd self: ";

   if( m_rParent.m_rCmdLine.isDemonize() )
   {
      if( m_rParent.m_logfile.is_open() )
      {
         if( m_rParent.m_isError )
            m_rParent.m_logfile << errorMessage << str() << std::flush;
         else
            m_rParent.m_logfile << str() << std::flush;
      }
      else if( m_rParent.m_isSyslogOpen )
      {
         if( m_rParent.m_isError )
            ::syslog( LOG_ERR, "%s%s", errorMessage, str().c_str() );
         else
            ::syslog( LOG_NOTICE, "%s", str().c_str() );
      }
   }
   else
   {
      if( m_rParent.m_isError )
         ERROR_MESSAGE( str() );
      else
         std::cout << str() << std::flush;
   }
   str("");
   m_rParent.m_isError = false;
   return std::stringbuf::sync();
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
Lm32Logd::Lm32Logd( mmuEb::EtherboneConnection& roEtherbone, CommandLine& rCmdLine )
   :std::iostream( &m_oStrgBuffer )
   ,m_oStrgBuffer( *this )
   ,m_rCmdLine( rCmdLine )
   ,m_oMmu( &roEtherbone )
   ,m_lastTimestamp( 0 )
   ,m_isError( false )
   ,m_isSyslogOpen( false )
   ,m_pMiddleBuffer( nullptr )
   ,m_poTerminal( nullptr )
{
   DEBUG_MESSAGE_M_FUNCTION("");

   if( m_rCmdLine.isDemonize() )
   {
      if( m_rCmdLine.getLogfileName().empty() )
      {
         DEBUG_MESSAGE( "Opening syslog" );
         ::openlog( "LM32", LOG_PID, LOG_DAEMON );
         m_isSyslogOpen = true;
      }
      else
      {
         DEBUG_MESSAGE( "Opening file: " << m_rCmdLine.getLogfileName() );
         m_logfile.open( m_rCmdLine.getLogfileName(), ios::out | ios::app  );
         if( !m_logfile.is_open() )
         {
            string msg = "Unable to open ";
            msg += m_rCmdLine.getLogfileName();
            throw std::runtime_error( msg );
         }
      }
   }
   else
   {
      assert( m_poTerminal == nullptr );
      m_poTerminal = new Terminal;
   }

   try
   {
      m_lm32Base = m_oMmu.getEb()->findDeviceBaseAddress( mmuEb::gsiId,
                                                          mmuEb::lm32_ram_user );
   }
   catch( std::exception& e )
   {
      if( m_rCmdLine.isDemonize() )
      {
         m_isError = true;
         *this << e.what() << endl;
      }
      throw e;
   }

   /*
    * The string addresses of LM32 comes from the perspective of the LM32.
    * Therefore a offset correction has to made here.
    */
   if( m_lm32Base < LM32_OFFSET )
   {
      const char* text = "LM32 base address is corrupt!";
      if( m_poTerminal != nullptr )
         m_poTerminal->reset();

      if( m_rCmdLine.isDemonize() )
      {
         m_isError = true;
         *this << text << std::flush;
      }

      throw std::runtime_error( text );
   }
   m_lm32Base -= LM32_OFFSET;

   if( m_rCmdLine.isReadBuildId() || m_rCmdLine.isAddBuildId() )
   {
      std::string idStr;

      readStringFromLm32( idStr, BUILD_ID_ADDR, true );
      if( m_rCmdLine.isReadBuildId() )
      {
         cout << idStr << endl;
         if( m_poTerminal != nullptr )
            m_poTerminal->reset();
         ::exit( EXIT_SUCCESS );
      }

      *this << idStr << std::flush;
   }


   if( !m_oMmu.isPresent() )
   {
      const char* text = "MMU not present!";

      if( m_poTerminal != nullptr )
         m_poTerminal->reset();

      if( m_rCmdLine.isDemonize() )
      {
         m_isError = true;
         *this << text << std::flush;
      }

      throw std::runtime_error( text );
   }

   mmu::MMU_STATUS_T status = m_oMmu.allocate( mmu::TAG_LM32_LOG, m_offset, m_capacity );
   if( status != mmu::OK )
   {
      string text = m_oMmu.status2String( status );
      if( m_poTerminal != nullptr )
         m_poTerminal->reset();

      if( m_rCmdLine.isDemonize() )
      {
         m_isError = true;
         *this << text << std::flush;
      }

      throw std::runtime_error( text );
   }

   if( m_rCmdLine.isVerbose() )
   {
      cout << "Found MMU-tag:  0x" << std::hex << std::uppercase << setw( 4 ) << setfill('0')
            << (int)mmu::TAG_LM32_LOG << std::dec
            << "\nAddress:        " << m_offset
            << "\nCapacity:       " << m_capacity << endl;
   }

   assert( (m_offset * sizeof(mmu::RAM_PAYLOAD_T)) % sizeof(SYSLOG_MEM_ITEM_T) == 0 );
   m_fifoAdminBase = m_offset * sizeof(mmu::RAM_PAYLOAD_T) + m_oMmu.getBase();

   m_offset   += SYSLOG_FIFO_ADMIN_SIZE;
   m_capacity -= SYSLOG_FIFO_ADMIN_SIZE;

   if( m_rCmdLine.isVerbose() )
   {
      cout << "Begin:          " << m_offset
           << "\nMax. log items: " << m_capacity / SYSLOG_FIFO_ITEM_SIZE << endl;
   }

   m_fiFoAdmin.admin.indexes.offset   = 0;
   m_fiFoAdmin.admin.indexes.capacity = 0;
   m_fiFoAdmin.admin.indexes.start    = 0;
   m_fiFoAdmin.admin.indexes.end      = 0;
   m_fiFoAdmin.admin.wasRead          = 0;
   updateFiFoAdmin( m_fiFoAdmin );

   if( m_rCmdLine.isVerbose() )
   {
      cout << "At the moment "
            << sysLogFifoGetItemSize( &m_fiFoAdmin )
            << " Log-items in FiFo." << endl;
   }
}

/*! ---------------------------------------------------------------------------
 */
Lm32Logd::~Lm32Logd( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
   if( m_pMiddleBuffer != nullptr )
   {
      DEBUG_MESSAGE( "Deleting reserved memory for middle buffer." );
      delete[] m_pMiddleBuffer;
   }
   if( m_poTerminal != nullptr )
   {
      delete m_poTerminal;
   }
   if( m_isSyslogOpen )
   {
      DEBUG_MESSAGE( "Closing syslog." );
      ::closelog();
   }
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::read( const etherbone::address_t eb_address,
                     eb_user_data_t pData,
                     const etherbone::format_t format,
                     const uint size )
{
   assert( m_oMmu.getEb()->isConnected() );
   try
   {
      m_oMmu.getEb()->read( eb_address, pData, format, size );
   }
   catch( std::exception& e )
   {
      if( m_rCmdLine.isDemonize() )
      {
         m_isError = true;
         *this << e.what() << endl;
      }
      throw e;
   }
}

/*! ---------------------------------------------------------------------------
 */
#ifdef CONFIG_IMPLEMENT_DDR3_WRITE
void Lm32Logd::ddr3Write( const etherbone::address_t eb_address,
                          const uint64_t* pData,
                          const uint size )
{
   assert( m_oMmu.getEb()->isConnected() );
   try
   {
      m_oMmu.getEb()->ddr3Write( eb_address, pData, size );
   }
   catch( std::exception& e )
   {
      if( m_rCmdLine.isDemonize() )
      {
         m_isError = true;
         *this << e.what() << endl;
      }
      throw e;
   }
}
#else
void Lm32Logd::write( const etherbone::address_t eb_address,
                      const eb_user_data_t pData,
                      const etherbone::format_t format,
                      const uint size )
{
   assert( m_oMmu.getEb()->isConnected() );
   try
   {
      m_oMmu.getEb()->write( eb_address, pData, format, size );
   }
   catch( std::exception& e )
   {
      if( m_rCmdLine.isDemonize() )
      {
         m_isError = true;
         *this << e.what() << endl;
      }
      throw e;
   }
}
#endif
/*! ---------------------------------------------------------------------------
 */
int Lm32Logd::readKey( void )
{
   if( m_poTerminal == nullptr )
      return 0;

   return Terminal::readKey();
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::updateFiFoAdmin( SYSLOG_FIFO_ADMIN_T& rAdmin )
{
   constexpr uint LEN32 = sizeof(SYSLOG_FIFO_ADMIN_T) / sizeof(uint32_t);

   read( m_fifoAdminBase, &rAdmin, EB_DATA32 | EB_LITTLE_ENDIAN, LEN32 );
   if( (rAdmin.admin.indexes.offset   != m_offset) ||
       (rAdmin.admin.indexes.capacity != m_capacity) )
   {
      const char* text = "LM32 syslog fifo is corrupt!";
      if( m_rCmdLine.isDemonize() )
      {
         m_isError = true;
         *this << text << std::endl;
      }
      throw std::runtime_error( text );
   }
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::setResponse( uint64_t n )
{
   DEBUG_MESSAGE_M_FUNCTION( n );
   static_assert( offsetof( SYSLOG_FIFO_ADMIN_T, admin.wasRead ) % sizeof( SYSLOG_MEM_ITEM_T ) == 0, "" );

#ifdef CONFIG_IMPLEMENT_DDR3_WRITE
   ddr3Write( m_fifoAdminBase + offsetof( SYSLOG_FIFO_ADMIN_T, admin.wasRead ),
              &n, 1 );
#else
   write( m_fifoAdminBase + offsetof( SYSLOG_FIFO_ADMIN_T, admin.wasRead ),
          static_cast<eb_user_data_t>(&n),
          EB_DATA32 | EB_LITTLE_ENDIAN, 2 );
#endif
}

constexpr uint LM32_MEM_SIZE = 147456;
constexpr uint HIGHST_ADDR = LM32_MEM_SIZE + LM32_OFFSET;

/*! ---------------------------------------------------------------------------
 */
uint Lm32Logd::readLm32( char* pData, std::size_t len, const std::size_t offset )
{
   DEBUG_MESSAGE_M_FUNCTION("");

   if( (offset-LM32_OFFSET + len) >= LM32_MEM_SIZE )
   {
      DEBUG_MESSAGE( "End of memory, can't read full length of: " << len );
      len -= (offset-LM32_OFFSET + len) - LM32_MEM_SIZE;
   }

   read( m_lm32Base + offset, pData, EB_BIG_ENDIAN | EB_DATA8, len );

   return len;
}

/*! ---------------------------------------------------------------------------
 */
inline bool Lm32Logd::isDecDigit( const char c )
{
   return gsi::isInRange( c, '0', '9');
}

/*! ---------------------------------------------------------------------------
 */
uint Lm32Logd::readStringFromLm32( std::string& rStr, uint addr, const bool alwaysLinefeed )
{
   DEBUG_MESSAGE_M_FUNCTION("");

   if( !gsi::isInRange( addr, LM32_OFFSET, HIGHST_ADDR ) )
   {
      throw std::runtime_error( "String address is corrupt!" );
   }

   enum STATE_T
   {
      FSM_DECLARE_STATE( NO_ESC,      color=blue ),
      FSM_DECLARE_STATE( ESC_CHAR,    color=green ),
      FSM_DECLARE_STATE( ESC_FIRST,   color=cyan ),
      FSM_DECLARE_STATE( ESC_DIGIT,   color=magenta ),
      FSM_DECLARE_STATE( ESC_OP_CODE, color=red )
   };

   FSM_INIT_FSM( NO_ESC, color=blue );
   char buffer[16];
   uint ret = 0;
   while( true )
   {
      const uint len = readLm32( buffer, sizeof( buffer ), addr );
      for( uint i = 0; i < len; i++ )
      {
         if( (buffer[i] == '\0') || (addr + i >= HIGHST_ADDR) )
         {
            DEBUG_MESSAGE( "received string: \"" << rStr.substr(rStr.length()-ret) << "\"" );
            return ret;
         }
         /*
          * FSM for filtering out escape sequences if demanded.
          */
         bool next;
         do
         { /*
            * Flag becomes "true" within the macro FSM_TRANSITION_NEXT.
            */
            next = false;
            switch( state )
            {
               case NO_ESC:
               {
                  if( !m_rCmdLine.isForConsole() && !alwaysLinefeed )
                  {
                     if( (buffer[i] == '\n') || (buffer[i] == '\r') )
                     {
                        if( buffer[i] == '\n')
                        {
                           rStr += ' ';
                           ret++;
                        }
                        FSM_TRANSITION_SELF( color=blue );
                     }
                     if( (buffer[i] == '\e') && !m_rCmdLine.isAllowedEscSequences() )
                     {
                        FSM_TRANSITION( ESC_CHAR, color=green, label='Esc' );
                     }
                  }
                  rStr += buffer[i];
                  ret++;
                  FSM_TRANSITION_SELF();
               }

               case ESC_CHAR:
               {
                  if( buffer[i] == '[' )
                     FSM_TRANSITION( ESC_FIRST, label ='[' );

                  FSM_TRANSITION( NO_ESC );
               }

               case ESC_FIRST:
               {
                  if( isDecDigit( buffer[i] ) )
                     FSM_TRANSITION( ESC_DIGIT, label='0-9' );

                  FSM_TRANSITION_NEXT( ESC_OP_CODE );
               }

               case ESC_DIGIT:
               {
                  if( isDecDigit( buffer[i] ) )
                     FSM_TRANSITION_SELF( label ='0-9');

                  FSM_TRANSITION_NEXT( ESC_OP_CODE );
               }

               case ESC_OP_CODE:
               {
                  if( (buffer[i] == ';') || isDecDigit( buffer[i] ) )
                     FSM_TRANSITION( ESC_DIGIT, label='0-9 or ;' );

                  FSM_TRANSITION( NO_ESC );
               }
            }
         }
         while( next );
      }
      addr += len;
   }
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::readItems( SYSLOG_FIFO_ITEM_T* pData, const uint len )
{
   DEBUG_MESSAGE_M_FUNCTION( " len = " << len );
   DEBUG_MESSAGE( "Read-index: " << sysLogFifoGetReadIndex( &m_fiFoAdmin ) );

   read( m_oMmu.getBase() +
         sysLogFifoGetReadIndex( &m_fiFoAdmin ) *
         sizeof(SYSLOG_MEM_ITEM_T),
         pData,
         EB_DATA32 | EB_LITTLE_ENDIAN,
         len * sizeof(SYSLOG_FIFO_ITEM_T) / sizeof(uint32_t) );
   sysLogFifoAddToReadIndex( &m_fiFoAdmin, len );
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::readItems( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");

   SYSLOG_FIFO_ADMIN_T fifoAdmin;

   updateFiFoAdmin( fifoAdmin );
   if( fifoAdmin.admin.wasRead != 0 )
      return;

   uint size = sysLogFifoGetSize( &fifoAdmin );
   if( size == 0 )
      return;

   if( (size % SYSLOG_FIFO_ITEM_SIZE) != 0 )
      return;

   m_fiFoAdmin = fifoAdmin;

   if( m_pMiddleBuffer == nullptr )
   {
      DEBUG_MESSAGE( "Allocating middle buffer for a maximum of "
                     << m_rCmdLine.getMaxItems() << " log-messages." );
      m_pMiddleBuffer = new SYSLOG_FIFO_ITEM_T[ m_rCmdLine.getMaxItems() * SYSLOG_FIFO_ITEM_SIZE ];
   }

   const uint readTotalLen = min( size, static_cast<uint>(m_rCmdLine.getMaxItems() * SYSLOG_FIFO_ITEM_SIZE) );
   uint len = readTotalLen;
   const uint numOfItems = readTotalLen / SYSLOG_FIFO_ITEM_SIZE;

   SYSLOG_FIFO_ITEM_T* pData = m_pMiddleBuffer;

   uint lenToEnd = sysLogFifoGetUpperReadSize( &m_fiFoAdmin );
   if( lenToEnd < readTotalLen )
   {
      DEBUG_MESSAGE( "reading first part"  );
      readItems( pData, lenToEnd );
      pData += (lenToEnd / SYSLOG_FIFO_ITEM_SIZE);
      len   -= lenToEnd;
   }
   assert( sysLogFifoGetUpperReadSize( &m_fiFoAdmin ) >= readTotalLen );
   readItems( pData, len );
   setResponse( readTotalLen );

   DEBUG_MESSAGE( "received: " << numOfItems << " items" );
   for( uint i = 0; i < numOfItems; i++ )
   {
      std::string output;
      evaluateItem( output, m_pMiddleBuffer[i] );
      *this << output << std::flush;
   }
}

/*! ---------------------------------------------------------------------------
 */
inline bool Lm32Logd::isPaddingChar( const char c )
{
   switch( c )
   {
      case '0': /* No break here! */
      case ' ': /* No break here! */
      case '.': /* No break here! */
      case '_':
      {
         return true;
      }
   }
   return false;
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::evaluateItem( std::string& rOutput, const SYSLOG_FIFO_ITEM_T& item )
{
   DEBUG_MESSAGE_M_FUNCTION("");

   if( item.filter >= BIT_SIZEOF( CommandLine::FILTER_FLAG_T ) )
   {
      m_isError = true;
      *this << "Filter value " << item.filter <<  " out of range!" << std::endl;
      return;
   }

   if( m_rCmdLine.getFilterFlags() != 0 )
   {
      if( (m_rCmdLine.getFilterFlags() & (1 << item.filter)) == 0 )
         return;
   }

   if( m_lastTimestamp >= item.timestamp )
   { /*
      * A non-causal deviation of lower or equal of one second will tolerated
      * because by using FreeRTOS its possible that the order of log-messages
      * becomes exchanged.
      */
      if( (m_lastTimestamp - item.timestamp) > daq::NANOSECS_PER_SEC )
      {
         m_isError = true;
         *this << "Invalid timestamp: last: " << m_lastTimestamp
               << ", actual: " << item.timestamp << std::endl;
         m_lastTimestamp = 0;
         return;
      }
   }
   m_lastTimestamp = item.timestamp;

   if( m_rCmdLine.isPrintFilter() )
   {
      std::stringstream stream;
      stream << std::setw( 2 ) << std::setfill( ' ' ) << item.filter
             << ", ";
      rOutput += stream.str();
   }

   if( !m_rCmdLine.isNoTimestamp() )
   {
      if( m_rCmdLine.isHumanReadableTimestamp() )
      {
         rOutput += daq::wrToTimeDateString( item.timestamp );
         std::stringstream stream;
         stream << " + " << std::setw( 9 ) << std::setfill( '0' )
                << (item.timestamp % daq::NANOSECS_PER_SEC) << " ns";
         rOutput += stream.str();
      }
      else
      {
         rOutput += std::to_string(item.timestamp);
      }
      rOutput += ": ";
   }

   if( !gsi::isInRange( item.format, LM32_OFFSET, HIGHST_ADDR ) )
   {
      m_isError = true;
      *this << "Address of format string is invalid: 0x"
            << std::hex << std::uppercase << std::setfill('0')
            << std::setw( 8 ) << item.format << std::dec << " !"
            << std::endl;
      return;
   }

   std::string format;
   readStringFromLm32( format, item.format );

   enum STATE_T
   {
      FSM_DECLARE_STATE( NORMAL, color=blue ),
      FSM_DECLARE_STATE( PADDING_CHAR, color=green ),
      FSM_DECLARE_STATE( PADDING_SIZE, color=cyan ),
      FSM_DECLARE_STATE( PARAM, color=magenta )
   };

   FSM_INIT_FSM( NORMAL, color=blue );
   char paddingChar = ' ';
   uint paddingSize = 0;

   uint ai = 0;
   uint base = 10;
   for( uint i = 0; i < format.length(); i++ )
   {
      bool next;
      do
      { /*
         * Flag becomes "true" within the macro FSM_TRANSITION_NEXT.
         */
         next = false;
         switch( state )
         {
            case NORMAL:
            {
               if( format[i] == '%' && (ai < ARRAY_SIZE(item.param)) )
               {
                  paddingChar = ' ';
                  paddingSize = 0;
                  FSM_TRANSITION( PADDING_CHAR, label='char == %', color=green );
               }

               rOutput += format[i];

               FSM_TRANSITION_SELF( color=blue );
            }

            case PADDING_CHAR:
            {
               if( format[i] == '%' )
               {
                  rOutput += format[i];
                  FSM_TRANSITION( NORMAL, label='char == %', color=blue );
               }
               if( isPaddingChar( format[i] ) )
               {
                  paddingChar = format[i];
                  FSM_TRANSITION( PADDING_SIZE, color=cyan );
               }
               if( isDecDigit( format[i] ) )
               {
                  FSM_TRANSITION_NEXT( PADDING_SIZE  );
               }
               FSM_TRANSITION_NEXT( PARAM, color=magenta );
            }

            case PADDING_SIZE:
            {
               if( !isDecDigit( format[i] ) )
                  FSM_TRANSITION_NEXT( PARAM, color=magenta );
               paddingSize *= 10;
               paddingSize += format[i] - '0';
               FSM_TRANSITION_SELF( color=cyan );
            }

            case PARAM:
            {
               bool signum  = false;
               bool unknown = false;
               bool done    = false;
               unsigned char hexOffset = 0;
               assert( ai < ARRAY_SIZE(item.param) );
               switch( format[i] )
               {
                  case 'S': /* No break here! */
                  case 's':
                  {
                     if( gsi::isInRange( item.param[ai], LM32_OFFSET, HIGHST_ADDR ) )
                        readStringFromLm32( rOutput, item.param[ai] );
                     else
                     {
                        m_isError = true;
                        *this << "String address of parameter " << (ai+1)
                              << " is invalid: 0x"
                              << std::hex  << std::uppercase << std::setfill('0')
                              << std::setw( 8 ) << item.param[ai] << std::dec << " !"
                              << std::flush;
                     }
                     ai++;
                     done = true;
                     break;
                  }
                  case 'c':
                  {
                     rOutput += item.param[ai++];
                     done = true;
                     break;
                  }
                  case 'X':
                  {
                     base = 16;
                     hexOffset = 'A' - '9' - 1;
                     break;
                  }
                  case 'x':
                  {
                     base = 16;
                     hexOffset = 'a' - '9' - 1;
                     break;
                  }
                  case 'p':
                  {
                     base = 16;
                     hexOffset = 'A' - '9' - 1;
                     paddingChar = '0';
                     paddingSize = sizeof(uint32_t) * 2;
                     break;
                  }
                  case 'i': /* No break here! */
                  case 'd':
                  {
                     base = 10;
                     signum = true;
                     break;
                  }
                  case 'u':
                  {
                     base = 10;
                     break;
                  }
                  case 'o':
                  {
                     base = 8;
                     break;
                  }
             #ifndef CONFIG_NO_BINARY_PRINTF_FORMAT
                  case 'b':
                  {
                     base = 2;
                     break;
                  }
             #endif
                  default:
                  {
                     unknown = true;
                     break;
                  }
               }
               if( unknown )
                  FSM_TRANSITION_NEXT( NORMAL, color=blue );

               if( done )
                  FSM_TRANSITION( NORMAL, color=blue );

               bool     isNegative = false;
               uint32_t value = item.param[ai++];
               STATIC_ASSERT( sizeof(value) == sizeof(item.param[0]) );
               if( signum && ((value & (1 << (BIT_SIZEOF(value)-1))) != 0) )
               {
                  value = -value;
                  if( paddingChar == '0' )
                     rOutput += '-';
                  else
                     isNegative = true;

                  if( paddingSize > 0 )
                     paddingSize--;
               }

               unsigned char digitBuffer[BIT_SIZEOF(value)+1];
               unsigned char* revPtr = digitBuffer + ARRAY_SIZE(digitBuffer);
               *--revPtr = '\0';
               assert( base > 0 );
               do
               {
                  unsigned char c = (value % base) + '0';
                  if( c > '9' )
                     c += hexOffset;
                  *--revPtr = c;
                  value /= base;
                  assert( revPtr >= digitBuffer );
                  if( paddingSize > 0 )
                     paddingSize--;
               }
               while( value > 0 );

               if( isNegative && (revPtr > digitBuffer) )
                  *--revPtr = '-';

               while( (paddingSize-- != 0) && (revPtr > digitBuffer) )
                  *--revPtr = paddingChar;

               rOutput += reinterpret_cast<char*>(revPtr);

               FSM_TRANSITION( NORMAL, label='param was read', color=blue );
            } /* case PARAM */
         } /* switch( state ) */
      }
      while( next );
   } /* for( uint i = 0; i < format.length(); i++ ) */

   if( m_rCmdLine.isForConsole() )
      return;

   if( m_isSyslogOpen )
      return;

   rOutput += '\n';
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::operator()( const bool& rExit )
{
   DEBUG_MESSAGE_M_FUNCTION("");

   if( m_rCmdLine.isSingleShoot() )
   {
      DEBUG_MESSAGE( "Single shoot is active" );
      readItems();
      return;
   }

   uint intervalTime = 0;
   while( !rExit && (readKey() != '\e') )
   {
      const uint it = daq::getSysMicrosecs();
      if( it > intervalTime )
      {
         intervalTime = it + m_rCmdLine.getPollInterwalTime() * 1000;
         readItems();
      }
      ::usleep( 1000 );
   }
   DEBUG_MESSAGE( "Loop left by " << (rExit? "SIGTERM":"Esc") );
}

//================================== EOF ======================================
