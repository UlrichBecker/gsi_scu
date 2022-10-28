/*!
 * @file  lm32_port_common.h Common includefile for port.c and portasm.s
 * @brief 
 *
 * @date 28.10.2022
 * @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see port.c
 * @see portasm.S
 */
#ifndef _LM32_PORT_COMMON_H
#define _LM32_PORT_COMMON_H

#define MICO32_FULL_CONTEXT_SAVE_RESTORE

/*!
 * @brief Storage offset in 32-bit values.
 * @note In this case a variable for the context switch flag will used,
 *       therefore the storage offset for register saving will be one.
 * @see __cscf
 */
#define ST_OFS 1

#ifdef MICO32_FULL_CONTEXT_SAVE_RESTORE
  #define TO_SAVE 30
  #define STK_RA  28
  #define STK_EA  29
  #define STK_BA   0
#else
  #define TO_SAVE 13
  #define STK_RA  11
  #define STK_EA  12
  #define STK_BA   0
#endif

#define CSCF_POS   0

#endif /* ifndef _LM32_PORT_COMMON_H */
/*================================== EOF ====================================*/
