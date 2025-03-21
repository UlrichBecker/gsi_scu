/*!
 *  @file logd_core.cpp
 *  @brief Main functionality of LM32 log daemon.
 *  >>> PvdS <<<
 *  @see https://www-acc.gsi.de/wiki/Frontend/LM32Developments#Handling_of_diagnostic_and_logging_messages_created_by_a_LM32_45Application_via_the_linux_45_application_34lm32_45logd_34
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
 #include <message_macros.hpp>
 #ifdef CONFIG_USE_SAFTLIB_MODULE_FOR_TAI_TO_UTC
   #include <scu_env.hpp>
   #include "Time.hpp"
 #endif
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

constexpr uint BUILD_ID_ADDR = Lm32Access::OFFSET + 0x100;

static_assert( EB_DATA32 == sizeof(uint32_t), "" );


///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief Central output function.
 */
int Lm32Logd::StringBuffer::sync( void )
{
   std::stringstream selfMessage;

   if( m_rParent.m_isError )
   { /*
      * Self logging...
      */
      if( m_rParent.m_rCmdLine.isPrintFilter() )
      { /*
         * In the case of self-logging there is no valid
         * filter to print.
         */
         selfMessage << "##, ";
      }
      if( !m_rParent.m_rCmdLine.isNoTimestamp() )
      { /*
         * In the case of self-logging there is no valid white rabbit
         * timestamp present so a WR-timestamp becomes emulated by the
         * PC-system-time by a lower accuracy of factor 1000.
         * That's better than nothing.
         */
         daq::USEC_T selfTimestamp = daq::getSysMicrosecs() * 1000;
         if( !m_rParent.m_rCmdLine.isUtc() || (m_rParent.m_rCmdLine.getLocalTimeOffset() == 0) )
            selfTimestamp += m_rParent.m_taiToUtcOffset;
         selfTimestamp += m_rParent.m_rCmdLine.getLocalTimeOffset();

         if( m_rParent.m_rCmdLine.isHumanReadableTimestamp() )
         {
            selfMessage << daq::wrToTimeDateString( selfTimestamp );
            selfMessage << " + " << std::setw( 9 ) << std::setfill( '0' )
                        << (selfTimestamp % daq::NANOSECS_PER_SEC) << " ns";
         }
         else
         {
            selfMessage << selfTimestamp;
         }
         selfMessage << ": ";
      }
      selfMessage << "ERROR: lm32-logd self: ";
   }

   if( m_rParent.m_rCmdLine.isDemonize() )
   {
      if( m_rParent.m_logfile.is_open() )
      {
         if( m_rParent.m_isError )
            m_rParent.m_logfile << selfMessage.str() << str() << std::endl;
         else
            m_rParent.m_logfile << str() << std::flush;
      }
      else if( m_rParent.m_isSyslogOpen )
      {
         if( m_rParent.m_isError )
            ::syslog( LOG_ERR, "%s%s", selfMessage.str().c_str(), str().c_str() );
         else
            ::syslog( LOG_NOTICE, "%s", str().c_str() );
      }
   }
   else
   {
      if( m_rParent.m_isError )
         ERROR_MESSAGE( selfMessage.str() << str() );
      else
         std::cout << str() << std::flush;
   }
   str("");
   m_rParent.m_isError = false;
   return std::stringbuf::sync();
}

///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Macro for log-messages which concerns the log-daemon self.
 */
#define LOG_SELF( msg... )           \
{                                    \
    setError();                      \
    *this << msg << std::flush;      \
}

/*! ---------------------------------------------------------------------------
 */
Lm32Logd::Lm32Logd( RamAccess* poRam, CommandLine& rCmdLine )
   :std::iostream( &m_oStrgBuffer )
   ,m_oStrgBuffer( *this )
   ,m_rCmdLine( rCmdLine )
   ,m_oMmu( poRam )
   ,m_oLm32( m_oMmu.getEb() )
   ,m_lastTimestamp( 0 )
   ,m_isError( false )
   ,m_isSyslogOpen( false )
   ,m_pMiddleBuffer( nullptr )
   ,m_poTerminal( nullptr )
   ,m_taiToUtcOffset( 0 )
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

   if( m_rCmdLine.isReadBuildId() || m_rCmdLine.isAddBuildId() )
   {
      std::string idStr;

      readStringFromLm32( idStr, BUILD_ID_ADDR, true );
      if( m_rCmdLine.isReadBuildId() )
      {
         cout << idStr << endl;
         if( m_poTerminal != nullptr )
            m_poTerminal->reset();
         /*
          * In this case the work is done.
          */
         ::exit( EXIT_SUCCESS );
      }

      *this << idStr << std::flush;
   }

   if( !m_rCmdLine.isNoTimestamp() && (m_rCmdLine.isUtc() || (m_rCmdLine.getLocalTimeOffset() != 0)) )
   {
   #ifdef CONFIG_USE_SAFTLIB_MODULE_FOR_TAI_TO_UTC
      if( isRunningOnScu() )
      {
         try
         {
            m_taiToUtcOffset = saftlib::UTC_offset_TAI( daq::getSysMicrosecs() * 1000 );
         }
         catch( std::exception& e )
         {
            m_taiToUtcOffset = daq::DELTA_UTC_TAI_NS;
            string msg( e.what() );
            msg.pop_back();
            msg += "! Using -";
            msg += to_string( m_taiToUtcOffset );
            msg += "ns as TAI to UTC offset.";
            if( m_rCmdLine.isDemonize() )
               LOG_SELF( msg )
            else
               WARNING_MESSAGE( msg );
         }
      }
      else
   #endif
         m_taiToUtcOffset = daq::DELTA_UTC_TAI_NS;
   }
   DEBUG_MESSAGE( "m_taiToUtcOffset: " << m_taiToUtcOffset );

   setBurstLimit( m_rCmdLine.getBurstLimit() );

   const uint reuqestedLogItems = m_rCmdLine.getMaxItemsInMemory() * SYSLOG_FIFO_ITEM_SIZE + SYSLOG_FIFO_ADMIN_SIZE;
   m_capacity = reuqestedLogItems;
   mmu::MMU_STATUS_T status = m_oMmu.allocate( mmu::TAG_LM32_LOG, m_offset, m_capacity, true );
   if( !m_oMmu.isOkay( status ) )
   {
      string text = m_oMmu.status2String( status );
      if( m_poTerminal != nullptr )
         m_poTerminal->reset();

      if( m_rCmdLine.isDemonize() )
         LOG_SELF( text )

      throw std::runtime_error( text );
   }

   if( (status == mmu::ALREADY_PRESENT) && (reuqestedLogItems != m_capacity) )
   {
      string text = "Memory for log-messages already allocated by another process, bud requested maximum number of items: ";
      text += to_string(static_cast<int>(m_rCmdLine.getMaxItemsInMemory()));
      text += " differs from the actual number: ";
      text += to_string(static_cast<int>((m_capacity - SYSLOG_FIFO_ADMIN_SIZE)/SYSLOG_FIFO_ITEM_SIZE));
      if( m_rCmdLine.isDemonize() )
         LOG_SELF( text )
      else
         WARNING_MESSAGE( text );
   }

   if( m_capacity < (SYSLOG_FIFO_ADMIN_SIZE + SYSLOG_FIFO_ITEM_SIZE) )
   {
      string errText = "Allocated memory of ";
      errText += to_string( m_capacity );
      errText += " 64-bit words is to small! At least ";
      errText += to_string( SYSLOG_FIFO_ADMIN_SIZE + SYSLOG_FIFO_ITEM_SIZE );
      errText += " 64-bit words shall be requested.";
      if( m_rCmdLine.isDemonize() )
         LOG_SELF( errText )

      throw std::runtime_error( errText );
   }

   if( m_rCmdLine.isVerbose() )
   {
      cout << "Found MMU-tag:  0x" << std::hex << std::uppercase << setw( 4 ) << setfill('0')
           << (int)mmu::TAG_LM32_LOG << std::dec
           << "\nAddress:        " << m_offset
           << "\nCapacity:       " << m_capacity << endl;
   }

   assert( (m_offset * sizeof(mmu::RAM_PAYLOAD_T)) % sizeof(SYSLOG_MEM_ITEM_T) == 0 );
   m_fifoAdminBase = m_offset;

   m_offset   += SYSLOG_FIFO_ADMIN_SIZE;
   m_capacity -= SYSLOG_FIFO_ADMIN_SIZE;

   /*
    * In the case the memory segment was already allocated by the tool "mem-mon"
    * it could be, that the allocated memory capacity isn't dividable by the
    * size of a sys-log item. In this case it becomes dividable by sacrificing
    * some 64-bit words.
    */
   m_capacity -= (m_capacity % SYSLOG_FIFO_ITEM_SIZE);

   /*
    * Was the memory allocated by this application,
    * or a reset is requested by command line option?
    */
   if( (status == mmu::OK) || m_rCmdLine.isReset() )
   {
      if( m_rCmdLine.isVerbose() )
      {
         cout << "Resetting log-FiFo." << endl;
      }
      resetFiFo();
   }

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
      cout << "Up to " << GET_ARRAY_SIZE_OF_MEMBER( SYSLOG_FIFO_ITEM_T, param )
           << " extra parameters per log-item possible." << endl;
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
      m_poTerminal->reset();
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
void Lm32Logd::setBurstLimit( int burstLimit )
{
   /*
    * Checking wether it is the DDR3-RAM of SCU3 and not the SRAM of SCU4.
    */
   if( dynamic_cast<Ddr3Access*>(m_oMmu.getRamAccess()) != nullptr )
   { /*
      * Yes, it is the DDR3-RAM, so the function Ddr3Access::setBurstLimit() is
      * available.
      */
      static_cast<Ddr3Access*>(m_oMmu.getRamAccess())->setBurstLimit( burstLimit );
   }
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::read( const uint index,
                     void* pData,
                     const uint size )
{
   assert( m_oMmu.getEb()->isConnected() );
   try
   {
      m_oMmu.getRamAccess()->read( index, reinterpret_cast<uint64_t*>(pData), size );
   }
   catch( std::exception& e )
   {
      if( m_rCmdLine.isDemonize() )
         LOG_SELF( e.what() )
      throw e;
   }
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::write( const uint index,
                      const void* pData,
                      const uint size )
{
   assert( m_oMmu.getEb()->isConnected() );
   try
   {
      m_oMmu.getRamAccess()->write( index, reinterpret_cast<const uint64_t*>(pData), size );
   }
   catch( std::exception& e )
   {
      if( m_rCmdLine.isDemonize() )
         LOG_SELF( e.what() )
      throw e;
   }
}

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
bool Lm32Logd::_updateFiFoAdmin( SYSLOG_FIFO_ADMIN_T& rAdmin )
{
   static_assert( (sizeof(SYSLOG_FIFO_ADMIN_T) % sizeof(uint64_t) == 0 ), "" );

   read( m_fifoAdminBase,
         reinterpret_cast<uint64_t*>(&rAdmin),
         sizeof(SYSLOG_FIFO_ADMIN_T) / sizeof(uint64_t) );

   if( rAdmin.admin.indexes.offset != m_offset )
      return false;

   if( rAdmin.admin.indexes.capacity != m_capacity )
      return false;

   /*
    * All seems to be OK.
    */
   return true;
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::updateFiFoAdmin( SYSLOG_FIFO_ADMIN_T& rAdmin )
{
   if( _updateFiFoAdmin( rAdmin ) )
      return;

   const char* text = "Fifo error. Trying to reinitialize FiFo.";

   if( m_rCmdLine.isDemonize() )
      LOG_SELF( text << "\n" )
   else
      WARNING_MESSAGE( text );

   resetFiFo();

   if( _updateFiFoAdmin( rAdmin ) )
      return;

   text = "LM32 syslog FiFo is corrupt!";
   if( m_rCmdLine.isDemonize() )
      LOG_SELF( text << "\n" )

   throw std::runtime_error( text );
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::resetFiFo( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");

   SYSLOG_FIFO_ADMIN_T fifoAdmin;

   fifoAdmin.admin.indexes.offset   = m_offset;
   fifoAdmin.admin.indexes.capacity = m_capacity;
   ramRingReset( &fifoAdmin.admin.indexes );
   fifoAdmin.admin.wasRead = 0;
   fifoAdmin.__padding__ = 0;

   write( m_fifoAdminBase, reinterpret_cast<uint64_t*>(&fifoAdmin),
          sizeof(SYSLOG_FIFO_ADMIN_T) / sizeof(uint64_t) );
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::setResponse( uint64_t n )
{
   DEBUG_MESSAGE_M_FUNCTION( n );
   static_assert( offsetof( SYSLOG_FIFO_ADMIN_T, admin.wasRead ) % sizeof( SYSLOG_MEM_ITEM_T ) == 0, "" );

   write( m_fifoAdminBase +
            offsetof( SYSLOG_FIFO_ADMIN_T, admin.wasRead ) / sizeof( SYSLOG_MEM_ITEM_T ),
          &n,
          1
        );
}

/*! ---------------------------------------------------------------------------
 */
uint Lm32Logd::readLm32( char* pData, std::size_t len, const std::size_t offset )
{
   DEBUG_MESSAGE_M_FUNCTION("");
   static_assert( sizeof( *pData ) == EB_DATA8, "" );

   if( (offset-Lm32Access::OFFSET + len) >= Lm32Access::MEM_SIZE )
   {
      DEBUG_MESSAGE( "End of memory, can't read full length of: " << len );
      len -= (offset-Lm32Access::OFFSET + len) - Lm32Access::MEM_SIZE;
   }

   try
   {
      m_oLm32.read( offset, pData, len );
   }
   catch( std::exception& e )
   {
      if( m_rCmdLine.isDemonize() )
         LOG_SELF( e.what() )
      throw e;
   }
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

   if( !gsi::isInRange( addr, Lm32Access::OFFSET, Lm32Access::MAX_ADDR ) )
   {
      string errTxt = "String address is corrupt!";
      if( m_rCmdLine.isDemonize() )
         LOG_SELF( errTxt )

      throw std::runtime_error( errTxt );
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
         if( (buffer[i] == '\0') || (addr + i >= Lm32Access::MAX_ADDR) )
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
            * Flag "next" becomes "true" within the macro FSM_TRANSITION_NEXT.
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

   read( sysLogFifoGetReadIndex( &m_fiFoAdmin ),
         reinterpret_cast<uint64_t*>(pData),
         len * sizeof(SYSLOG_MEM_ITEM_T) / sizeof(uint64_t) );

   sysLogFifoAddToReadIndex( &m_fiFoAdmin, len );
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::readItems( void )
{
 //  DEBUG_MESSAGE_M_FUNCTION("");

   SYSLOG_FIFO_ADMIN_T fifoAdmin;

   updateFiFoAdmin( fifoAdmin );
   if( fifoAdmin.admin.wasRead != 0 )
   { /*
      * Last posted messages was not yet acknowledged by LM32,
      * therefore there are no new messages present.
      */
      /*!
       * @todo In very rare cases there still seem to be problems here.
       *       Workaround: Restart this application by option -r.
       */
      return;
   }

   uint size = sysLogFifoGetSize( &fifoAdmin );
   if( size == 0 )
   { /*
      * No log-messages present.
      */
      return;
   }

   if( (size % SYSLOG_FIFO_ITEM_SIZE) != 0 )
   { /*
      * Log messages not jet compleated by LM32.
      */
      /*!
       * @todo Check this - maybe this could be a problem!
       */
      return;
   }
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
      case '0': FALL_THROUGH
      case ' ': FALL_THROUGH
      case '.': FALL_THROUGH
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
      LOG_SELF( "Filter value " << item.filter <<  " out of range!\n" )
      return;
   }

   if( m_rCmdLine.getFilterFlags() != 0 )
   {
      if( (m_rCmdLine.getFilterFlags() & (1 << item.filter)) == 0 )
         return;
   }

   if( m_taiToUtcOffset > static_cast<TYPEOF(m_taiToUtcOffset)>(item.timestamp) )
   {
      LOG_SELF( "Incorrect timestamp, the time is much too early: " << item.timestamp );
      return;
   }
   uint64_t timestamp = item.timestamp;
   if( m_rCmdLine.isUtc() || (m_rCmdLine.getLocalTimeOffset() != 0) )
      timestamp -= m_taiToUtcOffset;

   /*!
    * @todo Implement here the code of automatically local time offset
    *       if this application runs on SCU
    */
   timestamp += m_rCmdLine.getLocalTimeOffset();


   if( m_lastTimestamp >= timestamp )
   { /*
      * A non-causal deviation of lower or equal of one second will tolerated
      * because by using FreeRTOS its possible that the order of log-messages
      * becomes exchanged.
      */
      if( (m_lastTimestamp - timestamp) > daq::NANOSECS_PER_SEC )
      {
         LOG_SELF( "Invalid timestamp: last: " << m_lastTimestamp
                 << ", actual: " << timestamp )
         m_lastTimestamp = 0;
         return;
      }
   }
   m_lastTimestamp = timestamp;

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
         rOutput += daq::wrToTimeDateString( timestamp );
         std::stringstream stream;
         stream << " + " << std::setw( 9 ) << std::setfill( '0' )
                << (timestamp % daq::NANOSECS_PER_SEC) << " ns";
         rOutput += stream.str();
      }
      else
      {
         rOutput += std::to_string(timestamp);
      }
      rOutput += ": ";
   }

   if( !gsi::isInRange( item.format, Lm32Access::OFFSET, Lm32Access::MAX_ADDR ) )
   {
      LOG_SELF( "Address of format string is invalid: 0x"
                << std::hex << std::uppercase << std::setfill('0')
                << std::setw( 8 ) << item.format << std::dec << " !\n" )
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

   /*
    * Evaluating of the format-string.
    */
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
                  case 'S': FALL_THROUGH
                  case 's':
                  {
                     if( gsi::isInRange( item.param[ai], Lm32Access::OFFSET, Lm32Access::MAX_ADDR ) )
                        readStringFromLm32( rOutput, item.param[ai] );
                     else
                     {
                        LOG_SELF( "String address of parameter " << (ai+1)
                                  << " is invalid: 0x"
                                  << std::hex  << std::uppercase << std::setfill('0')
                                  << std::setw( 8 ) << item.param[ai] << std::dec << " !\n" )
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
                  case 'i': FALL_THROUGH
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
               if( signum && ((value & (static_cast<uint32_t>(1) << (BIT_SIZEOF(value)-1))) != 0) )
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
   /*
    * Main loop.
    */
   DEBUG_MESSAGE( "Entering main-loop..." );
   while( !rExit && (readKey() != '\e') )
   {
      const uint it = daq::getSysMicrosecs();
      if( it > intervalTime )
      {
         intervalTime = it + m_rCmdLine.getPollInterwalTime() * 1000;
         readItems();
      }
      /*!
       * @todo Improve the CPU-load maybe at this point.
       */
      ::usleep( 1000 );
   }
   DEBUG_MESSAGE( "Loop left by " << (rExit? "SIGTERM":"Esc") );
}

//================================== EOF ======================================
