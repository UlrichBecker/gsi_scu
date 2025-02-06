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
#include <vector>
#include <mutex>
#include <exception>

namespace mubu
{
///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Template-class managing circular thread-safe buffers (FiFos)
 *        which has one data source but could have one or more data sinks.
 *        Based on boost::circular_buffer.
 * @param PL_T Payload type.
 * @param ID_T Buffer-Identification type.
 */
template <typename PL_T, typename ID_T=uint>
class TMultiBuffer
{
public:
   using CB_T     = boost::circular_buffer<PL_T>;
   using VECTOR_T = std::vector<PL_T>;
   using MUTEX_T  = std::lock_guard<std::mutex>;

   /*!
    * @brief Class for a single circular buffer of multi- buffer.
    */
   class Buffer: public CB_T
   {
      TMultiBuffer* m_pParent;
      std::mutex    m_mutex;
      const ID_T    m_id;

   public:
      Buffer( TMultiBuffer* pParent, ID_T id, std::size_t size ):
         CB_T( size ),
         m_pParent( pParent ),
         m_id( id )
      {}

      std::size_t erase( std::size_t max )
      {
         MUTEX_T lock(m_mutex);

         max = std::min( CB_T::size(), max );
         CB_T::erase( CB_T::begin(), CB_T::begin() + max );

         return max;
      }

      void push( const PL_T& rPl )
      {
         MUTEX_T lock(m_mutex);
         CB_T::push_back( rPl );
      }

      void push( const VECTOR_T& rvPl )
      {
         MUTEX_T lock(m_mutex);
#if 0
         if( rvPl.size() > CB_T::capacity() )
         {
            CB_T::assign( rvPl.end() - CB_T::capacity(), rvPl.end() );
            return;
         }
         std::size_t remaining = (CB_T::capacity() - CB_T::size());
         if( rvPl.size() > remaining )
            CB_T::erase( CB_T::begin(), CB_T::begin() + rvPl.size() - remaining );

         CB_T::assign( rvPl.begin(), rvPl.end() );
#else
         /*!
          * @todo Workaround, find a cheaper algorithm.
          */
         for( const auto& rPl: rvPl )
         {
            CB_T::push_back( rPl );
         }
#endif
      }

      std::size_t getCapacity()
      {
         return CB_T::capacity();
      }

      std::size_t copy( PL_T* pPl, const std::size_t max )
      {
         MUTEX_T lock(m_mutex);

         auto array1 = CB_T::array_one();
         std::size_t toCopy = std::min( array1.second, max );
         if( toCopy > 0 )
            std::copy( array1.first, array1.first + toCopy, pPl );

         if( toCopy == max )
            return max;

         const std::size_t copied = toCopy;
         auto array2 = CB_T::array_two();
         toCopy = std::min( array2.second, max - copied );
         if( toCopy > 0 )
            std::copy( array2.first, array2.first + toCopy, pPl + copied );

         return copied + toCopy;
      }

      std::size_t copy( VECTOR_T& rvPl, const std::size_t max )
      {
         rvPl.reserve( max );

         MUTEX_T lock(m_mutex);

         auto array1 = CB_T::array_one();
         std::size_t toCopy = std::min( array1.second, max );
         if( toCopy > 0 )
            rvPl.insert( rvPl.end(), array1.first, array1.first + toCopy );

         if( toCopy == max )
            return max;

         const std::size_t copied = toCopy;
         auto array2 = CB_T::array_two();
         toCopy = std::min( array2.second, max - copied );
         if( toCopy > 0 )
            rvPl.insert( rvPl.end(), array2.first, array2.first + toCopy );

         return copied + toCopy;
      }


      void clear()
      {
         MUTEX_T lock(m_mutex);
         CB_T::clear();
      }

      std::size_t pull( PL_T* pPl, const std::size_t max )
      {
         return erase( copy( pPl, max ) );
      }

      std::size_t pull( VECTOR_T& rvPl, const std::size_t max )
      {
         return erase( copy( rvPl, max ) );
      }

      ID_T getId()
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
      MUTEX_T lock(m_mutex);
      for( auto& pLe: m_bufferList )
      {
         delete pLe;
      }
   }

   uint push( const PL_T& rPl )
   {
      uint ret = 0;
      MUTEX_T lock(m_mutex);
      for( auto& pLe: m_bufferList )
      {
         pLe->push( rPl );
         ret++;
      }
      return ret;
   }

   uint push( const VECTOR_T& rvPl )
   {
      uint ret = 0;
      MUTEX_T lock(m_mutex);
      for( auto& pLe: m_bufferList )
      {
         pLe->push( rvPl );
         ret++;
      }
      return ret;
   }

   Buffer* findBuffer( ID_T id )
   {
      MUTEX_T lock(m_mutex);
      for( auto& pLe: m_bufferList )
      {
         if( pLe->getId() == id )
            return pLe;
      }
      return nullptr;
   }

   Buffer* getBuffer( ID_T id )
   {
      Buffer* pBuffer = findBuffer( id );
      if( pBuffer == nullptr )
      {
         throw std::runtime_error( "Buffer-ID not found!" );
      }
      return pBuffer;
   }

   Buffer* createBuffer( ID_T id, std::size_t capacity = 0 )
   {
      Buffer* pBuffer = findBuffer( id );
      if( pBuffer == nullptr )
      {
         MUTEX_T lock(m_mutex);
         pBuffer = new Buffer( this, id, (capacity==0)? m_capacity : capacity );
         m_bufferList.push_front( pBuffer );
      }
      return pBuffer;
   }

   bool deleteBuffer( ID_T id )
   {
      MUTEX_T lock(m_mutex);
      for( auto it = m_bufferList.begin(); it != m_bufferList.end(); it++ )
      {
         if( (*it)->getId() == id )
         {
            m_bufferList.erase( it );
            delete *(it);
            return true;
         }
      }
      return false;
   }

   bool deleteAllBuffers()
   {
      MUTEX_T lock(m_mutex);
      if( m_bufferList.empty() )
         return false;
      for( auto& pLe: m_bufferList )
      {
         delete pLe;
      }
      m_bufferList.clear();
      return true;
   }

   void clear()
   {
      MUTEX_T lock(m_mutex);
      for( auto& pLe: m_bufferList )
      {
         pLe->clear();
      }
   }

   void clear( ID_T id )
   {
      getBuffer( id )->clear();
   }

   std::size_t getCapacity()
   {
      return m_capacity;
   }

   std::size_t getCapacity( ID_T id )
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

   std::size_t getSize( ID_T id )
   {
      return getBuffer( id )->size();
   }

   std::size_t copy( ID_T id, PL_T* pPl, const std::size_t max )
   {
      return getBuffer( id )->copy( pPl, max );
   }

   std::size_t copy( ID_T id, VECTOR_T& rvPl, const std::size_t max )
   {
      return getBuffer( id )->copy( rvPl, max );
   }

   std::size_t erase( ID_T id, const std::size_t max )
   {
      return getBuffer( id )->erase( max );
   }

   std::size_t pull( ID_T id, PL_T* pPl, const std::size_t max )
   {
      return getBuffer( id )->pull( pPl, max );
   }

   std::size_t pull( ID_T id, VECTOR_T& rvPl, const std::size_t max )
   {
      return getBuffer( id )->pull( rvPl, max );
   }

}; /* class TMultiBuffer */
} /* End namespace mubu */
#endif /* ifndef _TMUBU_HPP */
//================================== EOF ======================================

