/*!****************************************************************************
 * @file scu_mil.c MIL bus library
 * @author  Wolfgang Panschow
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/PerfOpt
 * This library works but has two issues:
 * 
 * 1. usage of platfrom dependent data types such as int
 *    --> conider using platfrom independent types such aa
 *    uint32_t instead
 * 2. register offsets are defined in units of integers,
 *    whereas common practice within ohwr is to use
 *    offsets in units of uint8_t
 *    --> consider using a different offset defintion
 * 
 * This file is split into two parts. The original code below 
 * is kept unchanged and is locacted directly below in the 
 * 1st part. 
 * The 2nd parts defines a new interface taking the into
 * account the suggestions above and extends the functionality
 * 
 * It shall be discussed, if the first part shall be deprecated
 * and using the definitions and routines of the 2nd part 
 * is encouraged.
 ****************************************************************************/
#ifdef CONFIG_RTOS
 #include <FreeRTOS.h>
 #include <task.h>
#endif
#include <scu_bus.h>
#include <sdb_lm32.h>
#include "scu_mil.h"

//#define CALC_OFFS(SLOT)   (((SLOT) * (1 << 16))) // from slot 1 to slot 12

#ifdef CONFIG_RTOS
 #define TRANSFER_DELAY    1
 #define RESET_DELAY     100
 #define READY_DELAY      10
#else
 #define TRANSFER_DELAY    1
 #define RESET_DELAY    1000
 #define READY_DELAY     100
#endif

/*! ---------------------------------------------------------------------------
 * @brief Makes some wait states during waiting for MIL-response.
 */
STATIC inline ALWAYS_INLINE void milWait( const unsigned int delay )
{
#ifdef CONFIG_RTOS
   vTaskDelay( delay );
#else
   usleep( delay );
#endif
}

#if 1
  #define milScuBusAtomicEnter() criticalSectionEnter()
  #define milScuBusAtomicExit()  criticalSectionExit()
#else
  #define milScuBusAtomicEnter() wbZycleEnter()
  #define milScuBusAtomicExit()  wbZycleExit()
#endif

#if 0
  #define milPiggyAtomicEnter() criticalSectionEnter()
  #define milPiggyAtomicExit()  criticalSectionExit()
#else
  #define milPiggyAtomicEnter() wbZycleEnter()
  #define milPiggyAtomicExit()  wbZycleExit()
#endif

/*!
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/PerfOpt
 */
/***********************************************************
 ***********************************************************
 *
 * 1st part: original MIL bus library
 *
 ***********************************************************
 ***********************************************************/
/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
//OPTIMIZE( "-O1"  ) //TODO Necessary if LTO activated. I don't know why yet!
int scub_write_mil_blk( void* pBase,
                        const unsigned int slot,
                        const uint16_t* pData,
                        const unsigned int fc_ifc_addr )
{
   void* pSlave = scuBusGetAbsSlaveAddr( pBase, slot );

   milScuBusAtomicEnter();

   scuBusSetSlaveValue16( pSlave, MIL_SIO3_TX_DATA, pData[0] );
   BARRIER(); /* CAUTION: Don't remove this macro it's really necessary! */
   scuBusSetSlaveValue16( pSlave, MIL_SIO3_TX_CMD, fc_ifc_addr );

   for( unsigned int i = 1; i < MIL_BLOCK_SIZE; i++ )
   {
      scuBusSetSlaveValue16( pSlave, MIL_SIO3_TX_DATA, pData[i] );
   }

   milScuBusAtomicExit();
   return OKAY;
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int scub_write_mil( void* pBase, const unsigned int slot,
                    const unsigned int data, const unsigned int fc_ifc_addr)
{
   void* pSlave = scuBusGetAbsSlaveAddr( pBase, slot );

   milScuBusAtomicEnter();
   scuBusSetSlaveValue16( pSlave, MIL_SIO3_TX_DATA, data );
   BARRIER(); /* CAUTION: Don't remove this macro it's really necessary! */
   scuBusSetSlaveValue16( pSlave, MIL_SIO3_TX_CMD, fc_ifc_addr );
   milScuBusAtomicExit();

   return OKAY;
}


/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int scub_status_mil( const void* pBase, const unsigned int slot, uint16_t* pStatus )
{
   if( slot >= SCUBUS_START_SLOT && slot <= MAX_SCU_SLAVES )
   {
      *pStatus = scuBusGetSlaveValue16( scuBusGetAbsSlaveAddr( pBase, slot ), MIL_SIO3_STAT );
      return OKAY;
   }

   return ERROR;
}

#define TR_BIT_MASK (1 << 2)

#ifdef CONFIG_MIL_PIGGY
/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int write_mil_blk(volatile unsigned int *base, uint16_t* data, short fc_ifc_addr)
{
   milPiggyAtomicEnter();
   base[MIL_SIO3_TX_DATA] = data[0];
   BARRIER();
   base[MIL_SIO3_TX_CMD]  = fc_ifc_addr;
   for( unsigned int i = 1; i < MIL_BLOCK_SIZE; i++ )
   {
      base[MIL_SIO3_TX_DATA] = data[i];
   }
   milPiggyAtomicExit();
   return OKAY;
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 * non blocking write; uses the tx fifo
 */
int write_mil( void* base, const unsigned int data, const unsigned int fc_ifc_addr)
{
   milPiggyAtomicEnter();
   milPiggySet( base, MIL_SIO3_TX_DATA, data );
   BARRIER();
   milPiggySet( base, MIL_SIO3_TX_CMD, fc_ifc_addr );
   milPiggyAtomicExit();
   return OKAY;
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int status_mil(volatile unsigned int *base, unsigned short *status )
{
   *status = base[MIL_SIO3_STAT];
   return OKAY;
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 * blocking read; uses task slot 2
 */
int read_mil( void* pBase, uint16_t* pData, const unsigned int fc_ifc_addr )
{
   unsigned int timeout = 0;

   /*
    * write fc and addr to taskram
    */
   milPiggySet( pBase, MIL_SIO3_TX_TASK2, fc_ifc_addr );

   /*
    * wait for task to start (tx fifo full or other tasks running)
    */
   while( (milPiggyGet( pBase, MIL_SIO3_TX_REQ ) & TR_BIT_MASK) == 0 )
   {
      if( timeout > BLOCK_TIMEOUT )
         return RCV_TIMEOUT;
      timeout++;
      milWait( TRANSFER_DELAY );
   }

   /*
    * wait for task to finish, a read over the dev bus needs at least 40us
    */
   while( (milPiggyGet( pBase, MIL_SIO3_D_RCVD ) & TR_BIT_MASK) == 0 )
   {
      if( timeout > BLOCK_TIMEOUT )
         return RCV_TIMEOUT;
      timeout++;
      milWait( TRANSFER_DELAY );
   }

   /*
    * task finished
    */
   *pData = milPiggyGet( pBase, MIL_SIO3_RX_TASK2 );

   /*
    * Checking whether an error has been occurred.
    */
   if( (milPiggyGet( pBase, MIL_SIO3_D_ERR ) & TR_BIT_MASK) != 0 )
      return RCV_TIMEOUT;

   return OKAY;
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 * non-blocking
 */
int set_task_mil( void* pBase, const unsigned int task, const unsigned int fc_ifc_addr )
{
   if( (task < TASKMIN) || (task > TASKMAX) )
      return RCV_TASK_ERR;

   /*
    * write fc and addr to taskram
    */
   milPiggySet( pBase, MIL_SIO3_TX_TASK1 + task - TASKMIN, fc_ifc_addr );
   return OKAY;
}

// blocks until data is available or timeout occurs
/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int get_task_mil( void* pBase, const unsigned int task, int16_t* pData )
{
   if( (task < TASKMIN) || (task > TASKMAX) )
      return RCV_TASK_ERR;

   /*
    * fetch avail and err bits
    */
   const unsigned int regOffset = task / 16;
   const unsigned int bitMask   = (1 << (task % 16));

   /*
    * Return by RCV_TASK_BSY if data is not available yet
    */
   if( (milPiggyGet( pBase, MIL_SIO3_D_RCVD + regOffset ) & bitMask) == 0 )
      return RCV_TASK_BSY;


   *pData = milPiggyGet( pBase, MIL_SIO3_RX_TASK1 + task - TASKMIN );

   /*
    * Return by OKAY if no error.
    */
   if( (milPiggyGet( pBase, MIL_SIO3_D_ERR + regOffset) & bitMask) == 0 )
      return OKAY;

   if( *pData == 0xDEAD )
      return RCV_TIMEOUT;

   if( *pData == 0xBABE )
      return RCV_PARITY;

   return RCV_ERROR;
}
#endif /* CONFIG_MIL_PIGGY */

// non-blocking
/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int scub_set_task_mil( void* pBase, const unsigned int slot,
                       const unsigned int task, const unsigned int fc_ifc_addr )
{
   if( (task < TASKMIN) || (task > TASKMAX) )
      return RCV_TASK_ERR;

  /*
   * write fc and addr to taskram
   */
   scuBusSetSlaveValue16( scuBusGetAbsSlaveAddr( pBase, slot ),
                          MIL_SIO3_TX_TASK1 + task - TASKMIN, fc_ifc_addr );
   return OKAY;
}

// blocks until data is available or timeout occurs
/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int scub_get_task_mil( const void* pBase, const unsigned int slot, const unsigned int task, int16_t* pData )
{
   if( (task < TASKMIN) || (task > TASKMAX) )
      return RCV_TASK_ERR;

   const unsigned int regOffset = task / 16;
   const unsigned int bitMask   = (1 << (task % 16));

   const void* pSlave = scuBusGetAbsSlaveAddr( pBase, slot );

   /*
    * Return by RCV_TASK_BSY if data is not available yet
    */
   if( (scuBusGetSlaveValue16( pSlave, MIL_SIO3_D_RCVD + regOffset ) & bitMask) == 0 )
      return RCV_TASK_BSY;

   *pData = scuBusGetSlaveValue16( pSlave, MIL_SIO3_RX_TASK1 + task - TASKMIN );

   /*
    * Return by OKAY if no error.
    */
   if( (scuBusGetSlaveValue16( pSlave, MIL_SIO3_D_ERR + regOffset) & bitMask) == 0 )
      return OKAY;

   if( *pData == 0xDEAD )
      return RCV_TIMEOUT;

   if( *pData == 0xBABE )
      return RCV_PARITY;

   return RCV_ERROR;
}


/* blocking dev bus read over scu bus using task slot 2*/
/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int scuBusSlaveReadMil( void* pSlave, uint16_t* pData, const unsigned int fc_ifc_addr )
{
   /*
    * write fc and addr to taskram
    */
   scuBusSetSlaveValue16( pSlave, MIL_SIO3_TX_TASK2, fc_ifc_addr );

   unsigned int timeout = 0;
   /*
    * wait for task to start (tx fifo full or other tasks running)
    */
   while( (scuBusGetSlaveValue16( pSlave, MIL_SIO3_TX_REQ ) & TR_BIT_MASK) == 0 )
   {
      if( timeout > BLOCK_TIMEOUT )
         return RCV_TIMEOUT;
      timeout++;
      milWait( TRANSFER_DELAY );
   }

   /*
    * wait for task to finish, a read over the dev bus needs at least 40us
    */
   while( (scuBusGetSlaveValue16( pSlave, MIL_SIO3_D_RCVD ) & TR_BIT_MASK) == 0 )
   {
      if( timeout > BLOCK_TIMEOUT )
         return RCV_TIMEOUT;
      timeout++;
      milWait( TRANSFER_DELAY );
   }

   /*
    * task finished
    */
   *pData = scuBusGetSlaveValue16( pSlave, MIL_SIO3_RX_TASK2 );

   /*
    * Checking whether an error has been occurred.
    */
   if( (scuBusGetSlaveValue16( pSlave, MIL_SIO3_D_ERR ) & TR_BIT_MASK) != 0 )
      return RCV_TIMEOUT;

   return OKAY;
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int scub_read_mil( void* base, const unsigned int slot, uint16_t* pData, const unsigned int fc_ifc_addr )
{
   return scuBusSlaveReadMil( scuBusGetAbsSlaveAddr( base, slot ), pData, fc_ifc_addr );
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 * reset all task slots
 */
void scuBusSlaveResetMil( void* pSlave )
{
   scuBusSetSlaveValue16( pSlave, MIL_SIO3_RST, 0 );
   milWait(RESET_DELAY);
   scuBusSetSlaveValue16( pSlave, MIL_SIO3_RST, 0xFF );
   /*
    * added by db; if not, an subsequent write/read results in an error -3
    */
   milWait(READY_DELAY);
}

/* reset all task slots */
/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int scub_reset_mil( uint16_t* base, int slot )
{
   scuBusSlaveResetMil( scuBusGetAbsSlaveAddr( base, slot ) );
   return OKAY; 
}

#ifdef CONFIG_MIL_PIGGY
/* reset all task slots */
/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
void reset_mil( void* base )
{
   milPiggySet( base, MIL_SIO3_RST, 0x00 );
   milWait( RESET_DELAY );
   milPiggySet( base, MIL_SIO3_RST, 0xFF );
   /*
    * added by db; if not, an subsequent write/read results in an error -3
    */
   milWait( READY_DELAY );
}
#endif /* ifdef CONFIG_MIL_PIGGY */

/***********************************************************
 ***********************************************************
 * 
 * 2nd part:  (new) MIL bus library
 *
 ***********************************************************
 ***********************************************************/
#ifdef CONFIG_MIL_PIGGY
/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int writeDevMil( void* base, uint16_t ifbAddr, uint16_t fctCode, const unsigned int data)
{
  // just a wrapper for the function of the original library
  // replace code once original library becomes deprecated
  
  uint16_t fc_ifb_addr;

  fc_ifb_addr = ifbAddr | (fctCode << 8);

  return write_mil( base, data, fc_ifb_addr);
} // writeDevMil

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t readDevMil( void* base, uint16_t  ifbAddr, uint16_t  fctCode, uint16_t* pData )
{
  // just a wrapper for the function of the original library
  // replace code once original library becomes deprecated

  uint16_t fc_ifb_addr;

  fc_ifb_addr = ifbAddr | (fctCode << 8);

  return (int16_t)read_mil( base, pData, fc_ifb_addr);
} //writeDevMil
#endif /* ifdef CONFIG_MIL_PIGGY */

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t echoTestDevMil( void* base, uint16_t  ifbAddr, uint16_t data)
{
  int32_t  busStatus;
  uint16_t rData = 0x0;

  busStatus = writeDevMil( base, ifbAddr, FC_WR_IFC_ECHO, data);
  if (busStatus != MIL_STAT_OK)
     return busStatus;

  busStatus = readDevMil(base, ifbAddr, FC_RD_IFC_ECHO, &rData);
  if (busStatus != MIL_STAT_OK)
     return busStatus;

  if (data != rData)
     return MIL_STAT_ERROR;
  else
     return MIL_STAT_OK;
} //echoTestDevMil

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int resetPiggyDevMil( void* base)
{
  // just a wrapper for the function of the original library
  // replace code once original library becomes deprecated

  reset_mil( base );
  return MIL_STAT_OK;
} //resetPiggyDevMil

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t clearFilterEvtMil(volatile uint32_t *base)
{
  uint32_t filterSize;         // size of filter RAM     
  uint32_t *pFilterRAM;        // RAM for event filters
  uint32_t i;

  filterSize = (MIL_REG_EV_FILT_LAST >> 2) - (MIL_REG_EV_FILT_FIRST >> 2) + 1;
  // mprintf("filtersize: %d, base 0x%08x\n", filterSize, base);

  pFilterRAM = (uint32_t *)(base + (MIL_REG_EV_FILT_FIRST >> 2));      // address to filter RAM 
  for (i=0; i < filterSize; i++)
     pFilterRAM[i] = 0x0;

  // mprintf("&pFilterRAM[0]: 0x%08x, &pFilterRAM[filterSize-1]: 0x%08x\n", &(pFilterRAM[0]), &(pFilterRAM[filterSize-1]));

  return MIL_STAT_OK;
} //clearFiterEvtMil

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t setFilterEvtMil(volatile uint32_t *base, uint16_t evtCode, uint16_t virtAcc, uint32_t filter)
{
  uint32_t *pFilterRAM;        // RAM for event filters

  if (virtAcc > 15) return MIL_STAT_OUT_OF_RANGE;

  pFilterRAM = (uint32_t*)(base + (uint32_t)(MIL_REG_EV_FILT_FIRST >> 2));  // address to filter RAM 

  pFilterRAM[virtAcc*256+evtCode] = filter;

  // mprintf("pFilter: 0x%08x, &pFilter[evt_code*16+acc_number]: 0x%08x\n", pFilterRAM, &(pFilterRAM[evtCode*16+virtAcc]));

  return MIL_STAT_OK;
} //setFilterEvtMil

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t enableFilterEvtMil(volatile uint32_t *base)
{
  uint32_t regValue;

  readCtrlStatRegEvtMil(base, &regValue);
  regValue = regValue | MIL_CTRL_STAT_EV_FILTER_ON;
  writeCtrlStatRegEvtMil(base, regValue);
  
  return MIL_STAT_OK;
} //enableFilterEvtMil

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t disableFilterEvtMil(volatile uint32_t *base)
{
  uint32_t regValue;

  readCtrlStatRegEvtMil(base, &regValue);
  regValue = regValue & (MIL_CTRL_STAT_EV_FILTER_ON);
  writeCtrlStatRegEvtMil(base, regValue);
    
  return MIL_STAT_OK;
} // disableFilterEvtMil

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t writeCtrlStatRegEvtMil(volatile uint32_t *base, uint32_t value)
{
  uint32_t *pControlRegister;  // control register of event filter

  pControlRegister  = (uint32_t *)(base + (MIL_REG_WR_RD_STATUS >> 2));
  *pControlRegister = value;

  return MIL_STAT_OK;
} // writeCtrlStatRegMil

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t readCtrlStatRegEvtMil(volatile uint32_t *base, uint32_t *value)
{
  uint32_t *pControlRegister;  // control register of event filter

  pControlRegister  = (uint32_t *)(base + (MIL_REG_WR_RD_STATUS >> 2));
  *value = *pControlRegister;

  return MIL_STAT_OK;
} //readCtrlStatRegMil

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
uint16_t fifoNotemptyEvtMil(volatile uint32_t *base)
{
  uint32_t regValue;
  uint16_t fifoNotEmpty;

  readCtrlStatRegEvtMil(base, &regValue);
  fifoNotEmpty = (uint16_t)(regValue & MIL_CTRL_STAT_EV_FIFO_NE);
  
  return (fifoNotEmpty);
} // fifoNotemptyEvtMil

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t clearFifoEvtMil(volatile uint32_t *base)
{
  uint32_t *pFIFO;

  pFIFO = (uint32_t *)(base + (MIL_REG_RD_CLR_EV_FIFO >> 2));
  *pFIFO = 0x1; // check value!!!

  return MIL_STAT_OK;
} // clearFifoEvtMil

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t popFifoEvtMil(volatile uint32_t *base, uint32_t *evtData)
{
  uint32_t *pFIFO;

  pFIFO = (uint32_t *)(base + (MIL_REG_RD_CLR_EV_FIFO >> 2));

  *evtData = *pFIFO;
  
  return MIL_STAT_OK;
} // popFifoEvtMil

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t configLemoPulseEvtMil(volatile uint32_t *base, uint32_t lemo)
{
  uint32_t *pConfigRegister;

  uint32_t statRegValue;
  uint32_t confRegValue;
  
  if (lemo > 4) return MIL_STAT_OUT_OF_RANGE;

  // disable gate mode 
  readCtrlStatRegEvtMil(base, &statRegValue);
  if (lemo == 1) statRegValue = statRegValue & ~MIL_CTRL_STAT_PULS1_FRAME;
  if (lemo == 2) statRegValue = statRegValue & ~MIL_CTRL_STAT_PULS2_FRAME;
  writeCtrlStatRegEvtMil(base, statRegValue);

  // enable output
  pConfigRegister = (uint32_t *)(base + (MIL_REG_WR_RF_LEMO_CONF >> 2));
  confRegValue = *pConfigRegister;
  if (lemo == 1) confRegValue = confRegValue | MIL_LEMO_OUT_EN1 | MIL_LEMO_EVENT_EN1;
  if (lemo == 2) confRegValue = confRegValue | MIL_LEMO_OUT_EN2 | MIL_LEMO_EVENT_EN2;
  if (lemo == 3) confRegValue = confRegValue | MIL_LEMO_OUT_EN3 | MIL_LEMO_EVENT_EN3;
  if (lemo == 4) confRegValue = confRegValue | MIL_LEMO_OUT_EN4 | MIL_LEMO_EVENT_EN4;
  *pConfigRegister = confRegValue;

  return MIL_STAT_OK;
} // configLemoPulseEvtMil


/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t configLemoGateEvtMil(volatile uint32_t *base, uint32_t lemo)
{
  uint32_t *pConfigRegister;

  uint32_t statRegValue;
  uint32_t confRegValue;

  if (lemo > 2) return MIL_STAT_OUT_OF_RANGE;
  
  // enable gate mode 
  readCtrlStatRegEvtMil(base, &statRegValue);
  if (lemo == 1) statRegValue = statRegValue | MIL_CTRL_STAT_PULS1_FRAME;
  if (lemo == 2) statRegValue = statRegValue | MIL_CTRL_STAT_PULS2_FRAME;
  writeCtrlStatRegEvtMil(base, statRegValue);

  // enable output
  pConfigRegister = (uint32_t *)(base + (MIL_REG_WR_RF_LEMO_CONF >> 2));
  confRegValue = *pConfigRegister;
  if (lemo == 1) confRegValue = confRegValue | MIL_LEMO_EVENT_EN1;
  if (lemo == 2) confRegValue = confRegValue | MIL_LEMO_EVENT_EN2;
  *pConfigRegister = confRegValue;
  
  return MIL_STAT_OK;  
} //enableLemoGateEvtMil

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t configLemoOutputEvtMil(volatile uint32_t *base, uint32_t lemo)
{
  uint32_t *pConfigRegister;

  uint32_t statRegValue;
  uint32_t confRegValue;
  
  if (lemo > 4) return MIL_STAT_OUT_OF_RANGE;

  // disable gate mode 
  readCtrlStatRegEvtMil(base, &statRegValue);
  if (lemo == 1) statRegValue = statRegValue & ~MIL_CTRL_STAT_PULS1_FRAME;
  if (lemo == 2) statRegValue = statRegValue & ~MIL_CTRL_STAT_PULS2_FRAME;
  writeCtrlStatRegEvtMil(base, statRegValue);

  // enable output for programable operation
  pConfigRegister = (uint32_t *)(base + (MIL_REG_WR_RF_LEMO_CONF >> 2));
  confRegValue = *pConfigRegister;
  if (lemo == 1) confRegValue = confRegValue | MIL_LEMO_OUT_EN1;
  if (lemo == 2) confRegValue = confRegValue | MIL_LEMO_OUT_EN2;
  if (lemo == 3) confRegValue = confRegValue | MIL_LEMO_OUT_EN3;
  if (lemo == 4) confRegValue = confRegValue | MIL_LEMO_OUT_EN4;
  *pConfigRegister = confRegValue;

  return MIL_STAT_OK; 
} //configLemoOutputEvtMil

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t setLemoOutputEvtMil(volatile uint32_t *base, uint32_t lemo, uint32_t on)
{
  uint32_t *pLemoDataRegister;

  uint32_t dataRegValue;

  if (lemo > 4) return MIL_STAT_OUT_OF_RANGE;
  if (on > 1)   return MIL_STAT_OUT_OF_RANGE;

  // read current value of register
  pLemoDataRegister = (uint32_t *)(base + (MIL_REG_WR_RD_LEMO_DAT >> 2));
  dataRegValue = *pLemoDataRegister;

  // modify value for register
  if (on)
  {
    if (lemo == 1) dataRegValue = dataRegValue | MIL_LEMO_OUT_EN1;
    if (lemo == 2) dataRegValue = dataRegValue | MIL_LEMO_OUT_EN2;
    if (lemo == 3) dataRegValue = dataRegValue | MIL_LEMO_OUT_EN3;
    if (lemo == 4) dataRegValue = dataRegValue | MIL_LEMO_OUT_EN4;
  } // if on
  else
  {
    if (lemo == 1) dataRegValue = dataRegValue & ~MIL_LEMO_OUT_EN1;
    if (lemo == 2) dataRegValue = dataRegValue & ~MIL_LEMO_OUT_EN2;
    if (lemo == 3) dataRegValue = dataRegValue & ~MIL_LEMO_OUT_EN3;
    if (lemo == 4) dataRegValue = dataRegValue & ~MIL_LEMO_OUT_EN4;
  } //else if on

  //write new value to register
  *pLemoDataRegister = dataRegValue;

  return MIL_STAT_OK;
} //setLemoOutputEvtMil

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t disableLemoEvtMil(volatile uint32_t *base, uint32_t lemo)
{
  uint32_t *pConfigRegister;

  uint32_t statRegValue;
  uint32_t confRegValue;

  if (lemo > 4) return MIL_STAT_OUT_OF_RANGE;

  // disable gate mode 
  readCtrlStatRegEvtMil(base, &statRegValue);
  if (lemo == 1) statRegValue = statRegValue & ~MIL_CTRL_STAT_PULS1_FRAME;
  if (lemo == 2) statRegValue = statRegValue & ~MIL_CTRL_STAT_PULS2_FRAME;
  writeCtrlStatRegEvtMil(base, statRegValue);

  // disable output
  pConfigRegister = (uint32_t *)(base + (MIL_REG_WR_RF_LEMO_CONF >> 2));
  confRegValue = *pConfigRegister;
  if (lemo == 1) confRegValue = confRegValue & ~MIL_LEMO_OUT_EN1 & ~MIL_LEMO_EVENT_EN1;
  if (lemo == 2) confRegValue = confRegValue & ~MIL_LEMO_OUT_EN2 & ~MIL_LEMO_EVENT_EN2;
  if (lemo == 3) confRegValue = confRegValue & ~MIL_LEMO_OUT_EN3 & ~MIL_LEMO_EVENT_EN3;
  if (lemo == 4) confRegValue = confRegValue & ~MIL_LEMO_OUT_EN4 & ~MIL_LEMO_EVENT_EN4;
  *pConfigRegister = confRegValue;

  return MIL_STAT_OK;
} // disableLemoEvtMil

/*================================== EOF ====================================*/
