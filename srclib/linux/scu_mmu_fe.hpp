/*!
 * @file scu_mmu_fe.hpp
 * @brief Memory Management Unit of SCU Linux-interface for front end
 *
 * Administration of the shared memory (for SCU3 using DDR3) between
 * Linux host and LM32 application.
 *
 * @note This source code is suitable for LM32 and Linux.
 *
 * @see       scu_mmu_fe.cpp
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      06.04.2022
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
#ifndef _SCU_MMU_FE_HPP
#define _SCU_MMU_FE_HPP

#include <scu_mmu.h>
#include <scu_memory.hpp>
#include <assert.h>

namespace Scu
{
namespace mmu
{

///////////////////////////////////////////////////////////////////////////////
/*!
 * @ingroup SCU_MMU
 * @brief C++ wrapper class for SCU Memory Management Unit 
 */
class Mmu
{
   RamAccess* m_poRam;

public:
   /*!
    * @param poRam Pointer to object of type RamAccess.
    */
   Mmu( RamAccess* poRam );


   /*!
    * @brief Destructor of MMU.
    */
   ~Mmu( void );

   /*!
    * @brief Returns "true" when a MMU has been discovered.
    */
   bool isPresent( void )
   {
      assert( m_poRam->isConnected() );
      return mmuIsPresent();
   }

   /*!
    * @brief Deletes a existing partition-table.
    */
   void clear( void )
   {
      assert( m_poRam->isConnected() );
      mmuDelete();
   }

   /*!
    * @brief Returns the number of memory blocks.
    */
   uint getNumberOfBlocks( void )
   {
      assert( m_poRam->isConnected() );
      return mmuGetNumberOfBlocks();
   }

  /*!
   * @brief Allocates a memory area in the shared memory.
   * @param tag Unique tag respectively identifier for this memory area which
   *            has to be reserved.
   * @param rStartAddr Points on the value of the start address respectively
   *                   start index in smallest addressable memory items.
   * @param rLen Requested number of items to allocate in in smallest addressable
   *            memory items. 
   * @param create If true then a new memory block will created,
   *               else a existing block will found only.
   * @return @see MMU_STATUS_T
   */
   MMU_STATUS_T allocate( const MMU_TAG_T tag, MMU_ADDR_T& rStartAddr,
                          size_t& rLen, const bool create = false )
   {
      assert( m_poRam->isConnected() );
      return mmuAlloc( tag, &rStartAddr, &rLen, create );
   }

  /*! 
   * @brief Converts the status which returns the function mmuAlloc() in a
   *        ASCII-string.
   */
   std::string status2String( const MMU_STATUS_T status )
   {
      return mmuStatus2String( status );
   }

  /*!
   * @brief Evaluates the status (return value of mmuAlloc()) and
   *        returns true if mmuAlloc was successful.
   */
   bool isOkay( const MMU_STATUS_T status )
   {
      return mmuIsOkay( status );
   }

  /*!
   * @brief Returns the pointer of the object of type EtherboneConnection.
   */
   EBC::EtherboneConnection* getEb( void )
   {
      return m_poRam->getEb();
   }

   /*!
    * @brief Returns the pointer of type RamAccess.
    */
   RamAccess* getRamAccess( void )
   {
      return m_poRam;
   }

  /*!
   * @brief Returns the etherbone base address of DDR3 RAM. 
   */
   uint getBase( void )
   {
      assert( false );
      return 0;
   }

  /*!
   * @brief Writes in SCU-RAM
   */
   void write( MMU_ADDR_T index, const RAM_PAYLOAD_T* pItem, size_t len )
   {
      static_assert( sizeof(RAM_PAYLOAD_T) == sizeof(uint64_t), "" );
      assert( m_poRam->isConnected() );
      m_poRam->write( index, reinterpret_cast<const uint64_t*>(pItem), len );
   }

  /*!
   * @brief Read from SCU-RAM
   */
   void read( MMU_ADDR_T index, RAM_PAYLOAD_T* pItem, size_t len )
   {
      static_assert( sizeof(RAM_PAYLOAD_T) == sizeof(uint64_t), "" );
      assert( m_poRam->isConnected() );
      m_poRam->read( index, reinterpret_cast<uint64_t*>(pItem), len );
   }

protected:

   void readNextItem( MMU_ITEM_T& rItem )
   {
      mmuReadNextItem( &rItem );
   }
}; // class Mmu

} // namespace mmu
} // namespace Scu

#endif // ifndef _SCU_MMU_FE_HPP
//================================== EOF ======================================
