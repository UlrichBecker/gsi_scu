#ifndef _SDB_ADD_H
#define _SDB_ADD_H

#ifndef __lm32__
 #error "This module is for LM32 only!"
#endif

/*!
 * @ingroup SDB
 * @brief Returns the root address of the self described bis (SDB)
 */
unsigned int sdb_add( void );

#endif
/*================================== EOF ====================================*/
