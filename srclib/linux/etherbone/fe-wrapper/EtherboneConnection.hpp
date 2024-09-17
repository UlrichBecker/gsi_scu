/*
 * EtherboneConnection.hpp
 *
 *  Created on: Feb 14, 2013
 *      Author: Vitaliy Rapp
 *      Revised: 2019 by Ulrich Becker
 */
#pragma once

#include <etherbone.h>
#include <utility>      // std::pair, std::make_pair
#include <string>

#define CONFIG_IMPLEMENT_DDR3_WRITE

#ifdef CONFIG_EB_USE_NORMAL_MUTEX
  #include <mutex>
#else
  #include <boost/interprocess/sync/named_mutex.hpp>
  #include <boost/interprocess/sync/scoped_lock.hpp>
#endif

#ifdef CONFIG_NO_INCLUDE_PATHS
  #include "Constants.hpp"
#else
  #include "feSupport/scu/etherbone/Constants.hpp"
#endif

#ifndef EB_DEFAULT_TIMEOUT
   #define EB_DEFAULT_TIMEOUT 5000
#endif

#ifndef EB_DEFAULT_CONNECTION
  #define EB_DEFAULT_CONNECTION "dev/wbm0"
#endif

namespace FeSupport {
  namespace Scu {
    /*!
     * Namespace used to organize all etherbone related
     * classes.
     */
    namespace Etherbone {
#ifndef CONFIG_EB_USE_NORMAL_MUTEX
      namespace IPC = boost::interprocess;
#endif
      /*!
       * \brief Connection abstraction for an etherbone bus.
       */
      class EtherboneConnection {
        public:

       #ifdef CONFIG_EB_USE_NORMAL_MUTEX
          using MUTEX_T        = std::mutex;
          using SCOPED_MUTEX_T = std::lock_guard<MUTEX_T>;
       #else
          using MUTEX_T        = IPC::named_mutex;
          using SCOPED_MUTEX_T = IPC::scoped_lock<MUTEX_T>;
       #endif

          /*!
           * @brief Central definition of the pionter type for
           *        whisbone/etherbone accesses.
           *
           * If necessary it can be changed here in a smart-pointer,
           * e.g. shared_ptr<>
           *
           * @author Ulrich Becker
           */
          using EBC_PTR_T  = EtherboneConnection*;

          /*!
           * @brief Object for administrating a singelton object of the type
           *        "EtherboneConnection".
           *
           * In the case that in the future there are more than one wishbone
           * interfaces on a FEC, so this object becomes to a item-type of
           * a linked list.
           * @author Ulrich Becker
           */
          struct OBJ_ADMIN_T
          {  /*!
              * @brief Pointer of the single object of type
              *        "EtherboneConnection*".
              */
             EBC_PTR_T ptr_;

             /*!
              * @brief Instances counter.
              */
             uint      count_;
          };

          /*!
           * @brief Static class-function creates a single instance if not already created
           *        and/or gives the pointer to this instance back.
           * @return Pointer to the object of type "EtherboneConnection".
           * @author Ulrich Becker
           */
          static EBC_PTR_T getInstance( const std::string& netaddress = EB_DEFAULT_CONNECTION,
                                        uint timeout = EB_DEFAULT_TIMEOUT );

          /*!
           * @brief Counterpart of getInstance(), invokes the destructor if the instance has existed.
           * @param ptr Return-value of getInstance(), at the moment only for debug purposes meaningful.
           * @author Ulrich Becker
           */
          static void releaseInstance( EBC_PTR_T ptr );

     private:
          /*!
           * \brief Basic constuctor, becomes invoked by the function getInstance() if
           *        it is the first instance.
           */
          EtherboneConnection( const std::string& netaddress = EB_DEFAULT_CONNECTION,
                               uint timeout = EB_DEFAULT_TIMEOUT );

          /*!
           * \brief Destructor, becomes invoked by the function releaseInstance() if
           *        it is the last instance.
           */
           virtual ~EtherboneConnection();
      public:
#ifndef CONFIG_EB_USE_NORMAL_MUTEX
          /*!
           * \brief check if named mutex is locked
           * @param true: unlock mutex, false: don't
           */
          bool checkMutex(bool unlock=false);
#endif
          /*!
           * \brief Connects to the etherbone bus
           */
          void connect();

          /*!
           * \brief Disconnects from the bus
           */
          void disconnect();
          /*!
           * \brief Searches for a particular device address
           */
          uint64_t findDeviceBaseAddress(Etherbone::VendorId vendorId,
                                         Etherbone::DeviceId deviceId,
                                         uint32_t ind=0);

          /*!
           * @brief Copies a data array in 1:1 manner form the bus.
           * @author Ulrich Becker
           * @param eb_address Address to read from
           * @param pData Destination address to store data
           * @param format Or-link of endian convention and data format
           *               (8, 16, 32 or 64) bit.
           * @param size Length of data array.
           * @param modWbAddrOfs Modulo value for increment the wb-address offset. \n
           *                     By default (value is zero) no modulo operation
           *                     will made (normal operation).\n
           *                     Meaningful are the values 1 (no increment)
           *                     for 32-bit register, or 2 for 64-bit register.
           */
          void read( const etherbone::address_t eb_address,
                     eb_user_data_t pData,
                     const etherbone::format_t format,
                     const uint size = 1,
                     uint modWbAddrOfs = 0 );

          /*!
           * \brief Reads a value from the bus in etherbone format.
           *
           * It wraps the asynchronous
           * etherbone call into a synchronous read method.
           *
           * @param eb_address Address to read from
           * @param data Array to store read data
           * @param format 32 or 16 bit
           * @param size Length of data array
           */
          void doRead(etherbone::address_t eb_address, etherbone::data_t* data,
                      etherbone::format_t format,
                      const uint16_t size = 1);

          void doVectorRead(const etherbone::address_t &eb_address, 
                            std::vector  <std::pair <etherbone::data_t, etherbone::data_t> >& v,
                            etherbone::format_t format);

          /*!
           * @brief Copies a data array in 1:1 manner to the bus.
           * @author Ulrich Becker
           * @param eb_address Address to write to
           * @param pData Array of data to write
           * @param format Or-link of endian convention and data format
           *               (8, 16, 32 or 64) bit.
           * @param size Length of data array.
           * @param modWbAddrOfs Modulo value for increment the wb-address offset. \n
           *                     By default (value is zero) no modulo operation
           *                     will made (normal operation).\n
           *                     Meaningful are the values 1 (no increment)
           *                     for 32-bit register, or 2 for 64-bit register.
           */
          void write( const etherbone::address_t eb_address,
                      const eb_user_data_t pData,
                      const etherbone::format_t format,
                      const uint size = 1,
                      uint modWbAddrOfs = 0 );

#ifdef CONFIG_IMPLEMENT_DDR3_WRITE
          /*!
           * @brief Copies a data array of 64 bit items on the DDR3-RAM
           *
           * Unfortunately this function is necessary because the writing of
           * the SCU DDR3-memory needs a special handling. \n
           * In contrast to other devices the upper 32 bit of the 64 bit
           * memory unit of DDR3 has to write first and than the lower 32 bit.
           * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/MacroF%C3%BCr1GbitDDR3MT41J64M16LADesSCUCarrierboards
           *
           * @author Ulrich Becker
           * @param eb_address Address to write to
           * @param pData Array of data to write
           * @param size Length in 64 bit items of data array.
           * @param modWbAddrOfs Modulo value for increment the wb-address offset. \n
           *                     By default (value is zero) no modulo operation
           *                     will made (normal operation).\n
           *                     Meaningful are the values 1 (no increment)
           *                     for 32-bit register, or 2 for 64-bit register.
           */
          void ddr3Write( const etherbone::address_t eb_address,
                          const uint64_t* pData,
                          const uint size = 1,
                          uint modWbAddrOfs = 0 );
#endif

          /*!
           * \brief Writes a single etherbone value to the bus.
           *
           * It wraps the asynchronous etherbone
           * call into a synchronous write method.
           *
           * @param eb_address Address to write to
           * @param data Array of data to write
           * @param format 32 or 16 bit
           * @param size Length of data array
           */
          void doWrite(const etherbone::address_t &eb_address,
                       const etherbone::data_t* data,
                       etherbone::format_t format,
                       const uint16_t size = 1);

          void doVectorWrite(const etherbone::address_t &eb_address, 
                             const std::vector <std::pair <etherbone::data_t, etherbone::data_t> >& v,
                             etherbone::format_t format);

          /*!
           * \brief Activate debugging outputs on console. 
           *
           * @param activate/deactivate debug output to console
           */
          void setDebug(const bool deb) { debug_ = deb; }

          uint32_t getSlaveMacroVersion(VendorId vendorId, DeviceId deviceId);

          /*!
           * @brief Returns true if connection has been successful established.
           * @author UB
           */
          bool isConnected() const
          {
             return connectionOpenCount_ > 0;
          }

          /*!
           * @brief Returns the number of requested connections.
           * @note For debug purposes only.
           * @author UB
           */
          uint getConnectionCounter() const
          {
             return connectionOpenCount_;
          }

          /*!
           * @brief Returns the net address given as first argument of
           *        the constructor.
           * @author UB
           */
          const std::string& getNetAddress()
          {
             return netaddress_;
          }

        protected:
          /*!
           * @brief Function will used from EtherboneConnection::write
           *        and EtherboneConnection::read
           * @note The real WB/EB hardware access will made within the
           *       function etherbone::Socket::run().\n
           *       If a real cycle callback function is given instead of
           *       "eb_block" then etherbone::Cycle::close() wont invoke
           *       this.\n
           *       The both functions mentioned above have to do this self.\n
           *
           * @author UB
           */
          int run()
          {
             return eb_socket_.run( timeout_ );
          }

          /*!
           * @brief Optional callback function becomes invoked during
           *        the final socked polling in the functions
           *        EtherboneConnection::write and EtherboneConnection::read.
           * @todo This class is not inheritable, since introducing
           *       getInstance() and releaseInstance(). Therefore replace
           *       this function by a function-pointer if they will need sometime.
           * @retval true Continue the polling loop
           * @retval false Polling loop will break.
           * @author UB
           */
          virtual bool onSockedPoll()
          {
             return true;
          }

        private:
          /*!
           * @brief System mutex
           */
          MUTEX_T _sysMu;
          // EB-Device to talk to
          //
          std::string netaddress_;

          // How long we want to wait for eb to answer [ms]
          // TODO Only used in function run() respectively in write()
          //      and read()
          unsigned int timeout_;

          // TODO Add RAII, initialize eb things in ctor
          // The socket, used for this connection
          etherbone::Socket eb_socket_;

          // device to talk too
          etherbone::Device eb_device_;

          /*!
           * @brief Becomes incremented by each calls of function connect()
           *        and decremented by each function calls of function disconnect()
           */
          uint connectionOpenCount_;

          // Contains the error Message in case the operation fails
          std::string errorMessage_;

          // debug flag, when set detailed infos for each etherbone access
          // will be printed to console output
          bool debug_;

          /*!
           * @brief Stores the pointer of an instance from itself.
           */
          static OBJ_ADMIN_T c_oAdmin;

          static std::string c_mutexName;
          static const char* __makeMutexName( const std::string& rName );
      };

    } // namespace Etherbone 
  } // namespace Scu
} // namepace FeSupport


