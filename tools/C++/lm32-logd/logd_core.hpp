/*!
 *  @file logd_core.hpp
 *  @brief Main functionality of LM32 log daemon.
 *  >>> PvdS <<<
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
#ifndef _LOGD_CORE_HPP
#define _LOGD_CORE_HPP

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <daqt_read_stdin.hpp>
#include <scu_mmu_fe.hpp>
#include <scu_ddr3_access.hpp>
#include <scu_sram_access.hpp>
#include <scu_lm32_access.hpp>
#include <lm32_syslog_common.h>
#include "logd_cmdline.hpp"

namespace Scu
{

///////////////////////////////////////////////////////////////////////////////////////
/*! -----------------------------------------------------------------------------------
 * @brief Specializing the LM32 access class for reading text-strings from
 *        LM32 memory.
 */
class Lm32LogAccess: public Lm32Access
{
   void init( void )
   {
      if( m_baseAddress < OFFSET )
      {
         throw std::runtime_error( "LM32 address is corrupt!" );
      }
      m_baseAddress -= OFFSET;
   }

public:
   /*!
    * @brief Constructor which uses a shared object of EtherboneConnection.
    *        It establishes a connection if not already done.
    * @param pEbc Pointer to object of type EtherboneConnection
    */
   Lm32LogAccess( EBC::EtherboneConnection* pEbc )
      :Lm32Access( pEbc )
   {
      init();
   }

   /*!
    * @brief Constructor which creates a object of type EtherboneConnection and
    *        establishes a connection.
    * @param rScuName In the case this application runs on ASL, the name of the target SCU.
    *                 In the case this application runs on a SCU then the name is "/dev/wbm0"
    * @param timeout Response timeout.
    */
   Lm32LogAccess( std::string& rScuName, uint timeout = EB_DEFAULT_TIMEOUT )
      :Lm32Access( rScuName, timeout )
   {
      init();
   }
};

///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Main-class of the LM32 log-system.
 * Receives the message items of the DDR3-RAM and reads ASCII-strings of
 * the LM32-memory.
 */
class Lm32Logd: public std::iostream
{
   /*!
    * @brief Class contains the buffer of the output stream.
    */
   class StringBuffer: public std::stringbuf
   {
      Lm32Logd&   m_rParent;

   public:
      StringBuffer( Lm32Logd& rParent )
         :m_rParent( rParent ) {}

      /*!
       * @brief Callback function becomes invoked by the stream manipulators
       *        std::endl and/or std::flush.
       * It puts the buffers content in the Linux-syslog system or to standard
       * out or in a file.
       */
      int sync( void ) override;
   };

   StringBuffer         m_oStrgBuffer;
   CommandLine&         m_rCmdLine;
   mmu::Mmu             m_oMmu;
   Lm32LogAccess        m_oLm32;
   uint                 m_fifoAdminBase;
   mmu::MMU_ADDR_T      m_offset;
   std::size_t          m_capacity;
   uint64_t             m_lastTimestamp;
   bool                 m_isError;
   bool                 m_isSyslogOpen;
   SYSLOG_FIFO_ADMIN_T  m_fiFoAdmin;

   SYSLOG_FIFO_ITEM_T*  m_pMiddleBuffer;
   std::ofstream        m_logfile;

   Terminal*            m_poTerminal;

   int64_t              m_taiToUtcOffset;

public:
   /*!
    * @brief Constructor makes all necessary initialization
    *        and prints the build-ID of the LM32-app if
    *        in the option demanded.
    * @param poRam Pointer to an object of the abstract memory access class.
    * @param rCmdLine Reverence to the object of the command line parser.
    */
   Lm32Logd( RamAccess* poRam, CommandLine& rCmdLine );

   /*!
    * @brief Destructor closes the sys-log of Linux if used.
    */
   ~Lm32Logd( void );

   /*!
    * @brief Functor performs the log-systems main-loop.
    * @param rExit Reference to exit condition, if "true" so
    *              the main-loop will left.
    */
   void operator()( const bool& rExit );

   /*!
    * @brief Returns the last received time-stamp.
    */
   uint64_t getLastTimestamp( void )
   {
      return m_lastTimestamp;
   }

   /*!
    * @brief Sets the error-flag for self diagnostic log-output.
    * This flag becomes automatically reset after the output
    * of the self diagnostic.
    */
   void setError( void )
   {
      m_isError = true;
   }

   /*!
    * @brief Sets the burst-limit in 64-bit words when a DDR3-RAM is used,
    *        otherwise this function is without effect.
    * @param burstLimit Number of 64-bit data words at when the
    *                   burst reading becomes active.\n
    *                   The value of zero miens that the burst reading
    *                   is always active.
    */
   void setBurstLimit( int burstLimit );

private:
   /*!
    * @brief Reads via the WB/EB-bus fron the SCU-TAM.
    */
   void read( const uint index,
              void* pData,
              const uint size );

   /*!
    * @brief Writes a array of 64-bit values via WB/EB-bus into the SCU-RAM.
    */
   void write( const uint index,
               const void* pData,
               const uint size );

   /*!
    * @brief Reads the LM32- memory
    */
   uint readLm32( char* pData, std::size_t len,
                  const std::size_t offset );

   /*!
    * @brief Reads a zero terminated ASCII-string from the LM32-memory.
    */
   uint readStringFromLm32( std::string& rStr, uint addr, const bool = false );

   bool _updateFiFoAdmin( SYSLOG_FIFO_ADMIN_T& );
   /*!
    * @brief Reads FiFo indexes (pointer) from the DDR3-RAM.
    */
   void updateFiFoAdmin( SYSLOG_FIFO_ADMIN_T& );

   /*!
    * @brief Resets the log-fifo in allocated memory segment.
    * @note CAUTION: All unread log items will be lost!
    */
   void resetFiFo( void );

   /*!
    * @brief Writes the number of DDR3-FiFo- log-items which has been read by the log-daemon
    *        in the DDR3-RAM, so that the LM32-app can delete them.
    */
   void setResponse( uint64_t n );

   /*!
    * @brief Reads log-items from the DDR3-memory.
    */
   void readItems( SYSLOG_FIFO_ITEM_T* pData, const uint len );

   /*!
    * @brief Reads log-items from the DDR3-memory.
    */
   void readItems( void );

   /*!
    * @brief Evaluates one log-item.
    */
   void evaluateItem( std::string& rOutput, const SYSLOG_FIFO_ITEM_T& item );

   /*!
    * @brief Reads a key from the PC-keyboard.
    */
   int readKey( void );

   static bool isPaddingChar( const char c );
   static bool isDecDigit( const char c );
};

} // namespace Scu

#endif // ifndef _LOGD_CORE_HPP
//================================== EOF ======================================
