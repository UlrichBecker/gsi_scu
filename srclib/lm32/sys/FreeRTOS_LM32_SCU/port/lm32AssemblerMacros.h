/*!
 * @file lm32AssemblerMacros.h
 * @brief     Some macros for LM32 GNU-Assembler for handling LM32 exceptions.
 * >>> PvdS <<<
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      27.05.2020
 */
#ifndef _LM32ASSEMBLERMACROS_H
#define _LM32ASSEMBLERMACROS_H

#if !defined(__lm32__) && !defined(__CPPCHECK__)
  #error This headder file is for the target Latice Micro32 (LM32) only!
#endif
#if !defined(__ASSEMBLER__) && !defined(__CPPCHECK__)
  #error This headder file is for GNU-Assembler only!
#endif

#ifndef __CPPCHECK__

/*! --------------------------------------------------------------------------
 * @brief Loads the pointer of a global 32-bit C/C++ variable in a register.
 * @param reg Register name.
 * @param var Name of the global variable.
 */
#ifdef __DOXYGEN__
#define LOAD_ADDR( reg, var )
#else
.macro LOAD_ADDR reg, var
   mvhi  \reg,  hi(\var)
   ori   \reg,  \reg, lo(\var)
.endm
#endif

/*! ---------------------------------------------------------------------------
 * @brief Loads the value of a global 32-bit C/C++ variable in a register.
 * @param reg Register name.
 * @param var Name of the global variable.
 */
#ifdef __DOXYGEN__
#define LOAD_VAR( reg, var )
#else
.macro LOAD_VAR reg, var
   LOAD_ADDR \reg, \var
   lw        \reg, (\reg+0)
.endm
#endif

#endif /* ifndef __CPPCHECK__ */
#endif /* ifndef _LM32ASSEMBLERMACROS_H */
/*================================== EOF ====================================*/
