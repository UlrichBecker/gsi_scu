/*!
 * @defgroup GENERATED_SOURCE
 * @{
 * @file ./generated/shared_mmap.h
 * @brief Location of Buildid and Shared Section in LM32 Memory, to be used by host
 * @note This file has been created automatically, do not modify it by hand!
 * @date Di 5. Jul 11:56:27 CEST 2022
 * @author makefile.scu
 */

#ifndef _SHARED_MMAP_H
#define _SHARED_MMAP_H

#define INT_BASE_ADR  0x10000000      /*!<@brief Address offset of LM32-RAM begin */
#define RAM_SIZE      147456      /*!<@brief Size of entire LM32-RAM in bytes */
#define STACK_SIZE    10240    /*!<@brief Size of LM32-stack in bytes */
#define BUILDID_OFFS  0x100 /*!<@brief Address offset of build ID text string */
#define SHARED_SIZE   0   /*!<@brief Maximum size of LM32- shared memory in bytes */
#define SHARED_OFFS   0x500  /*!<@brief Relative address offset of LM32- shared memory */

#endif
/*!@}*/
