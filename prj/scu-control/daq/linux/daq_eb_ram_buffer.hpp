/*!
 *  @file daq_eb_ram_buffer.hpp
 *  @brief Linux whishbone/etherbone interface for accessing the
 *         LM32 shared memory and the
 *         DDR3-RAM for SCU3 or -- in future -- SRAM for SCU4.
 *
 *  @see scu_ramBuffer.h
 *
 *  @see scu_ddr3.h
 *  @see scu_ddr3.c
 *  @date 19.06.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 *******************************************************************************
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>
 ******************************************************************************
 */
#ifndef _DAQ_EB_RAM_BUFFER_HPP
#define _DAQ_EB_RAM_BUFFER_HPP

#include <scu_control_config.h>
#include <EtherboneConnection.hpp>
#include <scu_lm32_access.hpp>
#include <scu_ddr3_access.hpp>
#include <scu_sram_access.hpp>
#include <scu_assert.h>
#include <daq_ramBuffer.h>
#include <daq_calculations.hpp>


// ... a little bit paranoia, I know ... ;-)
static_assert( EB_DATA8  == sizeof(uint8_t),  "" );
static_assert( EB_DATA16 == sizeof(uint16_t), "" );
static_assert( EB_DATA32 == sizeof(uint32_t), "" );
static_assert( EB_DATA64 == sizeof(uint64_t), "" );

namespace DaqEb = FeSupport::Scu::Etherbone;

namespace Scu
{
namespace daq
{

///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Interface class for accesses to LM32 shared memory and DDR3_RAM for
 *        SCU3 respectively in the future SRAM for SCU4
 *        via EtherboneConnection.
 */
class EbRamAccess
{
public:
   using EBC_PTR_T = EtherboneAccess::EBC_PTR_T;

private:
   /*!
    * @brief Specializing the class Lm32Access for shared-memory accesses
    *        of LM32.
    */
   class Lm32ShMemAccess: public Lm32Access
   {
   public:
      Lm32ShMemAccess( EBC_PTR_T );
   };

   /*!
    * @brief Access object for LM32 shared memory.
    */
   Lm32ShMemAccess             m_oLm32;

   /*!
    * @brief Pointer to access object to DDR3 for SCU3
    *        or SRAM for SCU4
    */
   RamAccess*                  m_poRamBuffer;

#ifdef CONFIG_EB_TIME_MEASSUREMENT /*========================================*/
public:
    struct TIME_MEASUREMENT_T
    {
       enum WB_ACCESS_T
       {
          UNKNOWN    = 0,
          LM32_READ  = 1,
          LM32_WRITE = 2,
          DDR3_READ  = 3
       };

       USEC_T      m_duration;
       USEC_T      m_timestamp;
       std::size_t m_dataSize;
       WB_ACCESS_T m_eAccess;

       TIME_MEASUREMENT_T( const USEC_T duration )
          :m_duration( duration )
          ,m_timestamp( 0 )
          ,m_dataSize( 0 )
          ,m_eAccess( UNKNOWN ) {}
    };

    struct MAX_DURATION: public TIME_MEASUREMENT_T
    {
       MAX_DURATION( void ): TIME_MEASUREMENT_T( 0 ) {}
    };

    struct MIN_DURATION: public TIME_MEASUREMENT_T
    {
       MIN_DURATION( void ): TIME_MEASUREMENT_T( static_cast<USEC_T>(~0) ) {}
    };

    using WB_ACCESS_T = TIME_MEASUREMENT_T::WB_ACCESS_T;

private:
    MAX_DURATION               m_oMaxDuration;
    MIN_DURATION               m_oMinDuration;
    USEC_T                     m_startTime;
#endif /* #ifdef CONFIG_EB_TIME_MEASSUREMENT ================================*/

public:

   /*!
    * @brief Constructor establishes the etherbone connection if it's not
    *        already been done outside.
    * @param poEb Pointer to the object of type EtherboneConnection.
    */
   EbRamAccess( EBC_PTR_T poEb );

   /*!
    * @brief Destructor terminates the ehtherbone connection if the connection was made
    *        by this object.
    */
   ~EbRamAccess( void );

   /*!
    * @brief Returns the pointer of the etherbone connection object of type:
    *        EtherboneConnection:
    */
   EBC_PTR_T getEbPtr( void )
   {
      return m_oLm32.getEb();
   }

   /*!
    * @brief Returns the TCP address e.g. "tcp/scuxl4711
    *        or in the case the application runs in the IPC of the SCU:
    *        "dev/wbm0"
    */
   const std::string& getNetAddress( void )
   {
      return m_oLm32.getNetAddress();
   }

   /*!
    * @brief Returns the IP address or - when it runs inside of a SCU - the
    *        wishbone name without the prefix "tcp/" or "dev/"
    */
   const std::string getScuDomainName( void )
   {
      return getNetAddress().substr( getNetAddress().find_first_of( '/' ) + 1 );
   }

   /*!
    * @brief Returns "true" when the etherbone/wishbone conection has been
    *        established.
    */
   bool isConnected( void )
   {
      return m_oLm32.isConnected();
   }

   /*!
    * @brief Returns the currently used burst-limit for DDR3 reading.
    */
   int getBurstLimit( void );

   /*!
    * @brief Sets a new burst-limit for DDR3 reading.
    */
   void setBurstLimit( int burstLimit = Ddr3Access::NEVER_BURST );

#ifdef CONFIG_EB_TIME_MEASSUREMENT /*========================================*/
private:

   void startTimeMeasurement( void )
   {
      m_startTime = getSysMicrosecs();
   }

   void stopTimeMeasurement( const std::size_t size, const WB_ACCESS_T access );

public:

   WB_ACCESS_T getWbMeasurementMaxTime( USEC_T& rTimestamp, USEC_T& rDuration, std::size_t& rSize );
   WB_ACCESS_T getWbMeasurementMinTime( USEC_T& rTimestamp, USEC_T& rDuration, std::size_t& rSize );
#else
   #define startTimeMeasurement()
   #define stopTimeMeasurement( a, b )
#endif /* else ifdef CONFIG_EB_TIME_MEASSUREMENT ============================ */

   /*!
    * @brief Reads data from the SCU-RAM.
    * @note At the time it's the DDR3-RAM yet!
    * @param pData Pointer to the destination buffer of type RAM_DAQ_PAYLOAD_T.
    * @param len Number of RAM-items of type RAM_DAQ_PAYLOAD_T to read.
    * @param offset Offset in RAM_DAQ_PAYLOAD_T units.
    */
   void readRam( RAM_DAQ_PAYLOAD_T* pData, std::size_t len, uint offset )
   {
      assert( m_poRamBuffer->isConnected() );

      startTimeMeasurement();
      m_poRamBuffer->read( offset, reinterpret_cast<uint64_t*>(pData), len );
      stopTimeMeasurement( len * sizeof( pData->ad32 ), TIME_MEASUREMENT_T::DDR3_READ );
   }

   /*!
    * @brief Reads circular administrated data from the SCU-RAM.
    * @note At the time it's the DDR3-RAM yet!
    * @param pData Pointer to the destination buffer of type RAM_DAQ_PAYLOAD_T.
    * @param len Number of RAM-items of type RAM_DAQ_PAYLOAD_T to read.
    * @param rIndexes Ring buffer administrator object.
    */
   void readRam( RAM_DAQ_PAYLOAD_T* pData, std::size_t len, RAM_RING_INDEXES_T& rIndexes );

   /*!
    * @brief Reads data from the LM32 shared memory area.
    * @note In this case a homogeneous data object is provided so
    *       the etherbone-library cant convert that in to little endian.\n
    *       Therefore in the case of the receiving of heterogeneous data e.g.
    *       structures, the conversion in little endian has to be made
    *       in upper software layers after.
    * @param pData Destination address for the received data.
    * @param len   Data length (array size).
    * @param offset Offset in bytes in the shared memory (default is zero)
    * @param format Base data size can be EB_DATA8, EB_DATA16, EB_ADDR32 or
    *               EB_ADDR64 defined in "etherbone.h" \n
    *               Default is EB_DATA8.
    */
   void readLM32( eb_user_data_t pData,
                  const std::size_t len,
                  const std::size_t offset = 0,
                  const etherbone::format_t format = EB_DATA8 )
   {
      startTimeMeasurement();
      m_oLm32.read( offset, pData, len, format | EB_BIG_ENDIAN );
      stopTimeMeasurement( len * (format & 0xFF), TIME_MEASUREMENT_T::LM32_READ );
   }


   /*!
    * @brief Writes data in the LM32 shared memory area.
    * @note In this case a homogeneous data object is provided so
    *       the etherbone-library cant convert that in to big endian.\n
    *       Therefore in the case of the sending of heterogeneous data e.g.
    *       structures, the conversion in little endian has to be made
    *       in upper software layers at first.
    * @param pData Source address of the data to copy
    * @param len   Data length (array size)
    * @param offset Offset in bytes in the shared memory (default is zero)
    * @param format Base data size can be EB_DATA8, EB_DATA16, EB_ADDR32 or
    *               EB_ADDR64 defined in "etherbone.h" \n
    *               Default is EB_DATA8.
    */
   void writeLM32( const eb_user_data_t pData,
                   const std::size_t len,
                   const std::size_t offset = 0,
                   const etherbone::format_t format = EB_DATA8 )
   {
      startTimeMeasurement();
      m_oLm32.write( offset, pData, len, format | EB_BIG_ENDIAN );
      stopTimeMeasurement( len * (format & 0xFF), TIME_MEASUREMENT_T::LM32_WRITE );
   }
};

} // namespace daq
} // namespace Scu
#endif /* _DAQ_EB_RAM_BUFFER_HPP */
//================================== EOF ======================================
