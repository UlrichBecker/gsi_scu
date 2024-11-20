/*!
 * @file eca_queue_type.c
 * @brief Initialization of  ECA register object type for Wishbone interface of VHDL entity
 * @author Ulrich Becker <u.becker@gsi.de>
 * @copyright   2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @date 30.01.2020
 * @see https://www-acc.gsi.de/wiki/Timing/TimingSystemHowSoftCPUHandleECAMSIs
 */
#if !defined(__lm32__) && !defined(__CPPCHECK__)
  #error This module is for LM32 only!
#endif

#include <lm32Interrupts.h>
#include <eca_queue_type.h>

#ifndef ECAQMAX
  #define ECAQMAX  4  /*!<@brief  max number of ECA queues */
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup ECA
 * @see eca_queue_type.h
 */
ECA_QUEUE_ITEM_T* ecaGetQueue( const unsigned int id )
{
   SDB_LOCATION_T ecaQeueBase[ECAQMAX];
   uint32_t queueIndex = 0;

   find_device_multi( ecaQeueBase, &queueIndex, ARRAY_SIZE(ecaQeueBase),
                      ECA_QUEUE_SDB_VENDOR_ID, ECA_QUEUE_SDB_DEVICE_ID );

   ECA_QUEUE_ITEM_T* pEcaQueue = NULL;
   ECA_QUEUE_ITEM_T* pEcaTemp;
   for( uint32_t i = 0; i < queueIndex; i++ )
   {
      pEcaTemp = (ECA_QUEUE_ITEM_T*) getSdbAdr( &ecaQeueBase[i] );
      if( (pEcaTemp != NULL) && (pEcaTemp->id == id) )
         pEcaQueue = pEcaTemp;
   }

   return pEcaQueue;
}

/*! ---------------------------------------------------------------------------
 * @ingroup ECA
 * @see eca_queue_type.h
 */
unsigned int ecaClearQueue( ECA_QUEUE_ITEM_T* pThis, const unsigned int cnt )
{
   unsigned int ret = 0;

   for( unsigned int i = 0; i < cnt; i++ )
   {
      if( ecaIsValid( pThis ) )
      {
         ecaPop( pThis );
         ret++;
      }
   }

   return ret;
}

/*! ---------------------------------------------------------------------------
 * @ingroup ECA
 * @see eca_queue_type.h
 */
void ecaSendEvent( volatile uint32_t* const pSendReg,
                   uint64_t eventId, uint64_t param, uint64_t wbTime )
{

   wbZycleEnter();

   /*
    * CAUTION: Don't change the order of the following code lines!
    */

   *pSendReg = (volatile uint32_t)GET_UPPER_HALF( eventId );
   BARRIER();
   *pSendReg = (volatile uint32_t)GET_LOWER_HALF( eventId );
   BARRIER();

   *pSendReg = (volatile uint32_t)GET_UPPER_HALF( param );
   BARRIER();
   *pSendReg = (volatile uint32_t)GET_LOWER_HALF( param );
   BARRIER();

   *pSendReg = 0;
   BARRIER();
   *pSendReg = 0;
   BARRIER();

   *pSendReg = (volatile uint32_t)GET_UPPER_HALF( wbTime );
   BARRIER();
   *pSendReg = (volatile uint32_t)GET_LOWER_HALF( wbTime );
   BARRIER();

   wbZycleExit();
}

/*================================== EOF ====================================*/
