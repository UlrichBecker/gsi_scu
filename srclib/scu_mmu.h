/*!
 * @file scu_mmu.h
 * @brief Memory Management Unit of SCU
 * 
 * Administration of the shared memory (for SCU3 using DDR3) between 
 * Linux host and LM32 application.
 * 
 * @see https://www-acc.gsi.de/wiki/Frontend/Memory_Management_On_SCU
 * @note This source code is suitable for LM32 and Linux.
 * 
 * @see       scu_mmu.c
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      30.03.2022
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
#ifndef _SCU_MMU_H
#define _SCU_MMU_H

/*!
 * @defgroup SCU_MMU Memory Management Unit of SCU
 *
 * Memory organization in DDR3-RAM of SCU3 respectively in SRAM of SCU4.\n
 * Each payload descriptor takes a place of 2 * 8 = 16 bytes.
 * @see MMU_ITEM_T
 * E.g.: 4 payload segments with its payload descriptors:
 * @code
 *          63           47           31           15           0
 *          +------------+------------+------------+------------+
 *          |      Magic number       |      Start index 1      |\
 *          +-------------------------+-------------------------+ start descriptor
 *          |            Padding respectively RFU               |/
 *          +------------+------------+-------------------------+
 * index 1> |   Tag 1    |  Flags 1   |      Start index 2      |\
 *          +------------+------------+-------------------------+ payload descriptor 1
 *          |      Start-address 1    |   Length of payload 1   |/
 *          +-------------------------+-------------------------+
 * addr 1 > |///////////////////////////////////////////////////|
 *          |///////////////////////////////////////////////////|
 *          =//////////////////// Payload 1 ////////////////////=
 *          |///////////////////////////////////////////////////|
 *          |///////////////////////////////////////////////////|
 *          +------------+------------+-------------------------+
 * index 2> |   Tag 2    |  Flags 2   |      Start index 3      |\
 *          +------------+------------+-------------------------+ payload descriptor 2
 *          |      Start-address 2    |   Length of payload 2   |/
 *          +-------------------------+-------------------------+
 * addr 2 > |///////////////////////////////////////////////////|
 *          |///////////////////////////////////////////////////|
 *          =//////////////////// Payload 2 ////////////////////=
 *          |///////////////////////////////////////////////////|
 *          |///////////////////////////////////////////////////|
 *          +------------+------------+-------------------------+
 * index 3> |   Tag 3    |  Flags 3   |      Start index 4      |\
 *          +------------+------------+-------------------------+ payload descriptor 3
 *          |      Start-address 3    |   Length of payload 3   |/
 *          +-------------------------+-------------------------+
 * addr 3 > |///////////////////////////////////////////////////|
 *          |///////////////////////////////////////////////////|
 *          =//////////////////// Payload 3 ////////////////////=
 *          |///////////////////////////////////////////////////|
 *          |///////////////////////////////////////////////////|
 *          +------------+------------+-------------------------+
 * index 4> |   Tag 4    |  Flags 4   |           0             |\
 *          +------------+------------+-------------------------+ payload descriptor 4
 *          |      Start-address 4    |   Length of payload 4   |/
 *          +-------------------------+-------------------------+
 * addr 4 > |///////////////////////////////////////////////////|
 *          |///////////////////////////////////////////////////|
 *          =//////////////////// Payload 4 ////////////////////=
 *          |///////////////////////////////////////////////////|
 *          |///////////////////////////////////////////////////|
 *          +---------------------------------------------------+
 * @endcode
 * The start-index of descriptor 4 is in this example zero, that means that no
 * further descriptors respectively segments of payload will follow.
 * @note In the case of DDR3, indices and lengths are counted in 64-bit units.
 *       That means these numbers has to be dividable by 8, because
 *       a 64-bit type is the smallest addressable unit.
 */

#include <stdint.h>
#include <stdbool.h>
#include <helper_macros.h>
#include <access64_type.h>
#ifdef __lm32__
 #ifdef CONFIG_SCU_USE_DDR3
  #include <scu_ddr3.h>
 #else
  #include <scu_sram.h>
 #endif
#endif

#ifdef __cplusplus
extern "C" {
namespace Scu
{
namespace mmu
{
#endif

/*!
 * @ingroup SCU_MMU
 * @brief Datatype for memory block identification.
 */
typedef uint16_t       MMU_TAG_T;

/*!
 * @ingroup SCU_MMU
 * @brief datatype for memory offset respectively index for the smallest
 *        addressable memory unit.
 */
typedef unsigned int   MMU_ADDR_T;

/*!
 * @ingroup SCU_MMU
 * @brief Datatype of the smallest addressable unit of the using memory.
 */
typedef ACCESS64_T     RAM_PAYLOAD_T;

/*!
 * @ingroup SCU_MMU
 * @brief Return values of the memory management unit. 
 */
typedef enum
{
   /*!
    * @brief Action was successful.
    */
   OK              =  0,

   /*!
    * @brief Wishbone device of RAM not found. 
    */
   MEM_NOT_PRESENT = -1,

   LIST_NOT_FOUND  = -2,

   /*!
    * @brief Memory block not found.
    */
   TAG_NOT_FOUND   = -3,

   /*!
    * @brief Requested memory block already present.
    */
   ALREADY_PRESENT = -4,

   /*!
    * @brief Requested memory block doesn't fit in physical memory.
    */
   OUT_OF_MEM      = -5
} MMU_STATUS_T;

/*!
 * @ingroup SCU_MMU
 * @brief Type of list item of memory partition list
 * @note Because of the different byteorder between x86 and LM32,
 *       the Linux- library "libetherbone" will made a byte-swap
 *       for all 32-bit types. \n Therefore has the order of the
 *       member-variables of "tag" and "flags" to be different
 *       between x86 and LM32.
 */
typedef struct PACKED_SIZE
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
   /*!
    * @brief Tag respectively identification (ID) of memory block.
    *        For LM32.
    */
   MMU_TAG_T tag;
#endif
   /*!
    * @brief Access flags of memory block. (rfu).
    */
   uint16_t  flags;

#if (__BYTE_ORDER__ != __ORDER_BIG_ENDIAN__)
   /*!
    * @brief Tag respectively identification (ID) of memory block.
    *        For Linux x86_64.
    */
   MMU_TAG_T tag;
#endif
   /*!
    * @brief Index of next item.
    * @note In the case of the last item then it has to be zero.
    */
   uint32_t  iNext;

   /*!
    * @brief Start index of memory block.
    */
   uint32_t  iStart;

   /*!
    * @brief Data size in RAM_PAYLOAD_T units of memory block.
    */
   uint32_t  length;
} MMU_ITEM_T;

STATIC_ASSERT( sizeof( uint16_t ) + sizeof( MMU_TAG_T ) == sizeof( uint32_t ) );
STATIC_ASSERT( sizeof( MMU_ITEM_T ) == 2 * sizeof( RAM_PAYLOAD_T ) );

/*!
 * @ingroup SCU_MMU
 * @brief Access adapter for MMU_ITEM_T.
 */
typedef union
{
   MMU_ITEM_T     mmu;
   RAM_PAYLOAD_T  item[sizeof(MMU_ITEM_T)/sizeof(RAM_PAYLOAD_T)];
} MMU_ACCESS_T;

STATIC_ASSERT( sizeof( MMU_ACCESS_T ) == sizeof( MMU_ITEM_T ) );

/*!
 * @ingroup SCU_MMU
 * @brief Size in addressable units of a single item of the partition list.
 */
STATIC const unsigned int MMU_ITEMSIZE = (sizeof( MMU_ITEM_T ) / sizeof( RAM_PAYLOAD_T ));

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_MMU
 * @brief Converts the status which returns the function mmuAlloc() in a
 *        ASCII-string.
 * @see mmuAlloc
 */
const char* mmuStatus2String( const MMU_STATUS_T status );

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_MMU
 * @brief Evaluates the status (return value of mmuAlloc()) and
 *        returns true if mmuAlloc was successful.
 */
STATIC inline bool mmuIsOkay( const MMU_STATUS_T status )
{
   return (status == OK) || (status == ALREADY_PRESENT);
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_MMU
 * @brief Returns "true" when the partition table is present. 
 */
bool mmuIsPresent( void );

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_MMU
 * @brief Returns the number of items of the memory partition table.
 */
unsigned int mmuGetNumberOfBlocks( void );

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_MMU
 * @brief Deletes a possible existing partition table.
 */
void mmuDelete( void );

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_MMU
 * @brief Reads a single item.
 */
void mmuReadItem( const MMU_ADDR_T index, MMU_ITEM_T* pItem );

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_MMU
 * @brief Reads the next item of the given item.
 */
STATIC inline void mmuReadNextItem( MMU_ITEM_T* pItem )
{
   mmuReadItem( pItem->iNext, pItem );
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_MMU
 * @brief Allocates a memory area in the shared memory.
 * @param tag Unique tag respectively identifier for this memory area which
 *            has to be reserved.
 * @param pStartAddr Points on the value of the start address respectively
 *                   start index in smallest addressable memory items.
 * @param pLen Requested number of items to allocate in in smallest addressable
 *            memory items. 
 * @param create If true then a new memory block will created,
 *               else a existing block will found only.
 * @return @see MMU_STATUS_T
 */
MMU_STATUS_T mmuAlloc( const MMU_TAG_T tag, MMU_ADDR_T* pStartAddr,
                       size_t* pLen, const bool create );

/*! ---------------------------------------------------------------------------
 * @brief Returns the total physical memory space in 64-bit units.
 */
#ifdef __lm32__
STATIC inline ALWAYS_INLINE
MMU_ADDR_T mmuGetMaxCapacity64( void )
{
 #ifdef CONFIG_SCU_USE_DDR3
   return DDR3_MAX_INDEX64;
 #else
   return SRAM_MAX_INDEX64;
 #endif
}
#else
/*!
 * @see scu_mmu_fe.cpp
 */
MMU_ADDR_T mmuGetMaxCapacity64( void );
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_MMU
 * @brief Writes the smallest addressable unit of the using memory. 
 *        In the case of SCU3 it's a 64-bit value.
 * @note This function depends on the platform (Linux or LM32), therefore it's
 *       NOT implemented in module scu_mmu.c! They has to be implemented
 *       separately for Linux or LM32.
 * @param index Start offset in addressable memory units.
 * @param pItem Pointer to memory area which has to be write.
 * @param len Number of items to write.
 */
void mmuWrite( MMU_ADDR_T index, const RAM_PAYLOAD_T* pItem, size_t len );

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_MMU
 * @brief Reads the smallest addressable unit of the using memory. 
 *        In the case of SCU3 it's a 64-bit value.
 * @note This function depends on the platform (Linux or LM32), therefore it's
 *       NOT implemented in module scu_mmu.c! They has to be implemented
 *       separately for Linux or LM32.
 * @param index Start offset in addressable memory units.
 * @param pItem Target pointer for the items to read.
 * @param len Number of items to read.
 */
void mmuRead( MMU_ADDR_T index, RAM_PAYLOAD_T* pItem, size_t len  );

#ifdef __cplusplus
} /* namespace mmu */
} /* namespace Scu */
} /* extern "C"    */
#endif

#endif /* ifndef _SCU_MMU_H */
/*================================== EOF ====================================*/
