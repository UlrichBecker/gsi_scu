/*!
 * @file TMubu.hpp
 * @brief Template managing circular thread-safe buffers (FiFos) which
 *        has one data source but could have one or more data sinks.
 *        Based on boost::circular_buffer.
 *
 * @note  Header only.
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      01.02.2025
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
#ifndef _TMUBU_HPP
#define _TMUBU_HPP

#include <boost/circular_buffer.hpp>
#include <list>
#include <mutex>
#include <string>
#include <exception>

namespace mubu
{
///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Template-class managing circular thread-safe buffers (FiFos)
 *        which has one data source but could have one or more data sinks.
 *        Based on boost::circular_buffer.
 * @param PL_T Payload type.
 */
template <typename PL_T>
class TMultiBuffer
{
public:
   /*!
    * @brief Class for a single circular buffer of multi- buffer.
    */
   class Buffer: public boost::circular_buffer<PL_T>
   {
      TMultiBuffer* m_pParent;
      std::mutex    m_mutex;
      const uint    m_id;

   public:
      Buffer( TMultiBuffer* pParent, uint id, std::size_t size ):
         boost::circular_buffer<PL_T>( size ),
         m_pParent( pParent ),
         m_id( id )
      {}

      void push( const PL_T& rPl )
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         boost::circular_buffer<PL_T>::push_back( rPl );
      }

      std::size_t copy( PL_T* pPl, const std::size_t max )
      {
         std::lock_guard<std::mutex> lock(m_mutex);

         auto array1 = boost::circular_buffer<PL_T>::array_one();
         std::size_t toCopy = std::min( array1.second, max );
         if( toCopy > 0 )
            std::copy( array1.first, array1.first + toCopy, pPl );

         if( toCopy == max )
            return max;

         std::size_t copied = toCopy;
         auto array2 = boost::circular_buffer<PL_T>::array_two();
         toCopy = std::min( array2.second, max - copied );
         if( toCopy > 0 )
            std::copy( array2.first, array2.first + toCopy, pPl + copied );

         return copied + toCopy;
      }

      std::size_t erase( std::size_t max )
      {
         std::lock_guard<std::mutex> lock(m_mutex);

         max = std::min( boost::circular_buffer<PL_T>::size(), max );

         boost::circular_buffer<PL_T>::erase( boost::circular_buffer<PL_T>::begin(),
                                              boost::circular_buffer<PL_T>::begin() + max );

         return max;
      }

      std::size_t pull( PL_T* pPl, const std::size_t max )
      {
         return erase( copy( pPl, max ) );
      }

      uint getId()
      {
         return m_id;
      }

      TMultiBuffer* getParent()
      {
         return m_pParent;
      }
   }; /* class Buffer */

private:
   using LIST_T = std::list<Buffer*>;


   std::mutex  m_mutex;
   LIST_T      m_bufferList;
   const std::size_t m_capacity;

public:
   TMultiBuffer( std::size_t capacity ):
      m_capacity( capacity ) {}

   ~TMultiBuffer()
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      for( auto& pLe: m_bufferList )
      {
         delete pLe;
      }
   }

   uint push( const PL_T& rPl )
   {
      uint ret = 0;
      std::lock_guard<std::mutex> lock(m_mutex);
      for( auto& pLe: m_bufferList )
      {
         pLe->push( rPl );
         ret++;
      }
      return ret;
   }

   Buffer* findBuffer( uint id )
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      for( auto& pLe: m_bufferList )
      {
         if( pLe->getId() == id )
            return pLe;
      }
      return nullptr;
   }

   Buffer* getBuffer( uint id )
   {
      Buffer* pBuffer = findBuffer( id );
      if( pBuffer == nullptr )
      {
         throw std::runtime_error( "Buffer-ID not found!" );
      }
      return pBuffer;
   }

   Buffer* createBuffer( uint id, std::size_t capacity = 0 )
   {
      Buffer* pBuffer = findBuffer( id );
      if( pBuffer == nullptr )
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         pBuffer = new Buffer( this, id, (capacity==0)? m_capacity : capacity );
         m_bufferList.push_front( pBuffer );
      }
      return pBuffer;
   }

   std::size_t getCapacity()
   {
      return m_capacity;
   }

   std::size_t getCapacity( uint id )
   {
      return getBuffer( id )->capacity();
   }

   std::size_t getMaxSize()
   {
      std::size_t size = 0;
      for( auto& pLe: m_bufferList )
      {
         size = std::max( size, pLe->size() );
      }
      return size;
   }

   std::size_t getSize( uint id )
   {
      return getBuffer( id )->size();
   }

   std::size_t copy( uint id, PL_T* pPl, const std::size_t max )
   {
      return getBuffer( id )->copy( pPl, max );
   }

   std::size_t erase( uint id, const std::size_t max )
   {
      return getBuffer( id )->erase( max );
   }

   std::size_t pull( uint id, PL_T* pPl, const std::size_t max )
   {
      return getBuffer( id )->pull( pPl, max );
   }

}; /* class TMultiBuffer */
} /* End namespace mubu */
#endif /* ifndef _TMUBU_HPP */
//================================== EOF ======================================

