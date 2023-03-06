/*!
 * @file scu_mutex.hpp
 * @brief Named mutex without using boost-library and other unnecessary ballast
 * @note Header only!
 * @date 21.02.2023
 * @copyright (C) 2023 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
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
#ifndef _SCU_MUTEX_HPP
#define _SCU_MUTEX_HPP

#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <message_macros.hpp>

namespace Scu
{

///////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
 * @brief Simple wrapper class of the POSIX- semaphore.
 */
class Mutex
{
   /*!
    * @brief Handle of POSIX semaphore.
    */
   ::sem_t* m_pSem;

public:
   /*!
    * @brief Constructor creates a named system-mutex.
    * @param name Name of the mutex.
    * @param oflag Specifies flags that control the operation of the call.
    * @param perm  Specifies the permissions when this mutex will created here and
    *              does not already exist.
    * @param count Specifies the initial value for  the  new  semaphore.
    */
   Mutex( std::string name, int oflag = O_CREAT, int perm = 0644, int count = 1 )
   {
      /*
       * Replacing possible prefix "tcp/" or "dev/" because slashes '/' are not
       * allowed in semaphore names.
       */
      name = name.substr( name.find_first_of( '/' ) + 1 );
      DEBUG_MESSAGE_M_FUNCTION( name );

      m_pSem = ::sem_open( name.c_str(), oflag, perm, count );
      if( m_pSem == SEM_FAILED )
      {
         std::string str = "Opening mutex: ";
         str += name;
         str += " error: ";
         str += ::strerror(errno);
         throw std::runtime_error( str );
      }

      /*
       * Ensures that semaphore will destroyed once all processes using
       * this semaphore has close this.
       */
      ::sem_unlink( name.c_str() );
   }

   /*!
    * @brief Destructor closes the mutex respectively the semaphore if they
    *        was successful opened.
    */
   ~Mutex( void )
   {
      DEBUG_MESSAGE_M_FUNCTION("");
      if( m_pSem != SEM_FAILED )
      {
         ::sem_close( m_pSem );
      }
   }

   /*!
    * @brief Locks a critical section at to protect it to concurrent accesses.
    * @see Mutex::unlock
    */
   void lock( void )
   {
      if( ::sem_wait( m_pSem ) != 0 )
      {
         std::string str = "Mutex::lock: ";
         str += ::strerror(errno);
         throw std::runtime_error( str );
      }
   }

   /*!
    * @brief Trying to lock a critical section.
    * @see Mutex::unlock
    * @retval true Lock of critical section was successful.
    * @retval false Lock not possible another process has already locked before.
    */
   bool tryLock( void )
   {
      if( ::sem_trywait( m_pSem ) != 0 )
      {
         if( errno == EAGAIN )
            return false;
         std::string str = "Mutex::tryLock: ";
         str += ::strerror(errno);
         throw std::runtime_error( str );
      }
      return true;
   }

#ifdef __USE_XOPEN2K
   /*!
    * @brief Try to lock within a defined time.
    * @see Mutex::unlock
    * @param pTimeout Maximum waiting time,
    * @retval false Timeout.
    * @retval true Lock was successful.
    */
   bool timedLock( ::timespec* pTimeout )
   {
      if( ::sem_timedwait( m_pSem, pTimeout ) != 0 )
      {
         if( errno == ETIMEDOUT )
            return false;
         std::string str = "Mutex::timedLock: ";
         str += ::strerror(errno);
         throw std::runtime_error( str );
      }
      return true;
   }

   /*!
    * @brief Try to lock within a defined time.
    * @see Mutex::unlock
    * @param nanosecs Maximum waiting time in nanoseconds,
    * @retval false Timeout.
    * @retval true Lock was successful.
    */
   bool timedLock( uint64_t nanosecs )
   {
      ::timespec tm =
      {
         .tv_sec  = static_cast<time_t>(nanosecs / 1000000000),
         .tv_nsec = static_cast<long>(  nanosecs % 1000000000)
      };
      return timedLock( &tm );
   }
#endif

   /*!
    * @brief Gives a locked section free.
    * Counterpart to lock(), () and timedLock();
    * @see Mutex::lock
    * @see Mutex::tryLock
    * @see Mutex::timedLock
    */
   void unlock( void )
   {
      if( ::sem_post( m_pSem ) != 0 )
      {
         std::string str = "Mutex::unlock: ";
         str += ::strerror(errno);
         throw std::runtime_error( str );
      }
   }
};

///////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
 * @brief Class can be used for critical sections which should be protect for
 *        concurrent accesses, and shall be freed automatically
 *        when the valid scope will left.
 * @see Scu::Mutex
 */
class AutoUnlock
{
   Mutex&  m_rMutex;

public:
   /*!
    * @brief Constructor locks a critical section.
    * @see Mutex::lock
    * @param rMutex Object of type Scu::Mutex.
    */
   AutoUnlock( Mutex& rMutex )
      :m_rMutex( rMutex )
   {
      m_rMutex.lock();
   }

   /*!
    * @brief Destructor makes a automatically freeing of a critical section
    *        locked by the constructor, when the valid scope will left.
    * @see Mutex::unlock
    */
   ~AutoUnlock( void )
   {
      m_rMutex.unlock();
   }

private:
   /*
    * Copy-constructors are not allowed for this class.
    */
   AutoUnlock( AutoUnlock& );
};

} // namespace Scu
#endif //ifndef _SCU_MUTEX_HPP
//================================== EOF ======================================
