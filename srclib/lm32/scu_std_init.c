/*!
 * @file scu_std_init.c
 * @brief Module makes some standard initializations before the function
 *        "main()" becomes called.
 * @note Provided the startup module crt0ScuLm32.S will used!
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      20.05.2020
 */
#include <sdb_lm32.h>
#include <mprintf.h>

/*! ---------------------------------------------------------------------------
 * @brief Function becomes invoked immediately before the function main() by
 *        the startup module crt0ScuLm32.S.
 *
 * In this case it becomes possible to use mprintf() immediately in
 * function main();
 * @see crt0ScuLm32.S
 */
OPTIMIZE( "-O0"  ) /*! @todo It seems to be necessary, but I don't know why yet... */
void __init( void )
{
   /*
    * Initialization of the UART, required for mprintf...
    */
   initMprintf();

   /*
    * Get info on important Wishbone infrastructure by module sdb_lm32.
    * Initialization of some global pointers.
    */
   discoverPeriphery();
}

/*================================== EOF ====================================*/
