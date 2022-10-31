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

/*
 * 31 registers will saved by context switch
 * whereby 30 of the 32 registers will relay saved:
 * "r0" is always 0 so on this place will contain a magic number,
 * and "sp" will not saved in stack.
 * Together with the variables __cscf and __asnc the total size
 * of stack is 33 words.
 */
#ifdef MICO32_FULL_CONTEXT_SAVE_RESTORE
  #define TO_SAVE_REGS  31
  #define STK_RA        28
  #define STK_EA        29
  #define STK_BA        30
  #define STK_CSCF      (TO_SAVE_REGS + 0)
  #define STK_ASNC      (TO_SAVE_REGS + 1)
#else
  #define TO_SAVE_REGS 13
  #define STK_RA  11
  #define STK_EA  12
  #define STK_BA   0
#endif

/*!
 * @brief Storage offset in 32-bit values.
 * @note In this case a variable for the context switch flag will used,
 *       therefore the storage offset for register saving will be one.
 * @see __cscf
 */
#define ST_OFS 2 //(STK_ASNC - TO_SAVE_REGS)



#define CSCF_POS   0

#endif /* ifndef _LM32_PORT_COMMON_H */
/*================================== EOF ====================================*/
