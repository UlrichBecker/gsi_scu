/*!
 * @file  lm32_port_common.h Common includefile for port.c and portasm.s
 * @brief 
 * @note Header only
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

#define CONFIG_SAVE_ASNC

/*
 * 31 registers will saved by context switch
 * whereby 30 of the 32 registers will relay saved:
 * "r0" is always 0 so on this place will contain a magic number,
 * and "sp" will not saved in stack.
 * Together with the variables __cscf and __asnc the total size
 * of stack is 33 words.
 */
#define TO_SAVE_REGS 31
#define STK_RA       28
#define STK_EA       29
#define STK_BA       30


#define CSCF_POS 0
#define STK_CSCF  (TO_SAVE_REGS + CSCF_POS)

#ifdef CONFIG_SAVE_ASNC
 #define ASNC_POS 1
 #define STK_ASNC (TO_SAVE_REGS + ASNC_POS)
#endif

/*!
 * @brief Storage offset in 32-bit values.
 * @note In this case a variable for the context switch flag will used,
 *       therefore the storage offset for register saving will be one.
 * @see __cscf
 */
#ifdef CONFIG_SAVE_ASNC
 #define ST_OFS 2
#else
 #define ST_OFS 1
#endif

/*!
 * @brief Number of reserved DWORDs (32-bit values) for the registers,
 *        the task switch cause flag and the atomic nesting counter.
 */
#define OS_STACK_DWORD_SIZE (ST_OFS + TO_SAVE_REGS)

#endif /* ifndef _LM32_PORT_COMMON_H */
/*================================== EOF ====================================*/
