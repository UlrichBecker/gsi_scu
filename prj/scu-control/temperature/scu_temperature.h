/*!
 * @file scu_temperature.h
 * @file Updates the temperatur information in the shared section
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 03.02.2020
 * Outsourced from scu_main.c
 */
#ifndef _SCU_TEMPERATURE_H
#define _SCU_TEMPERATURE_H

#include <scu_lm32_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @defgroup W1 Funktions and macros for handling the one whire bus. 
 */
   
/*!
 * @defgroup TEMPERATURE Functions and macros for handling the temperature sensors.
 */

/*!
 * @ingroup TEMPERATURE
 * @brief Pre-initializing value for recognizing a faulty temperature sensor.
 */
#define INVALID_TEMPERATURE ((uint32_t)~0)

/*! ---------------------------------------------------------------------------
 * @ingroup W1
 * @brief Pointers for one wire connections.
 */
typedef struct
{
   /*!
    * @brief Base pointer of one-wire controller in the WRC
    */
   uint8_t* pWr;

   /*!
    * @brief Base pointer of one-wire controller on dev crossbar
    */
   uint8_t* pUser;
} ONE_WIRE_T;

/*! ----------------------------------------------------------------------------
 * @ingroup W1
 * @brief Inizializing of the one wire base pointers used by
 * updateTemperature()
 */
bool initOneWire( void );

/*! ---------------------------------------------------------------------------
 * @ingroup TEMPERATURE
 * @ingroup W1
 * @brief updates the temperature information in the shared section
 */
void updateTemperature( void );


extern ONE_WIRE_T g_oneWireBase;
#ifdef __cplusplus
}
#endif

#endif
/*================================== EOF ====================================*/
