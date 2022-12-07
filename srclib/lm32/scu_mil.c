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
#include "scu_mil.h"

#define CALC_OFFS(SLOT)   (((SLOT) * (1 << 16))) // from slot 1 to slot 12

#ifdef CONFIG_RTOS
 #define TRANSFER_DELAY    1
 #define RESET_DELAY     100
 #define READY_DELAY      10
#else
 #define TRANSFER_DELAY    1
 #define RESET_DELAY    1000
 #define READY_DELAY     100
#endif

/*! ------------------------------------------------------------------------
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
int scub_write_mil_blk(volatile unsigned short *base, int slot, short *data, short fc_ifc_addr)
{
   criticalSectionEnter();
   base[CALC_OFFS(slot) + MIL_SIO3_TX_DATA] = data[0];
   base[CALC_OFFS(slot) + MIL_SIO3_TX_CMD] = fc_ifc_addr;

   for( unsigned int i = 1; i < MIL_BLOCK_SIZE; i++ )
   {
      base[CALC_OFFS(slot) + MIL_SIO3_TX_DATA] = data[i];
   }
   criticalSectionExit();
   return OKAY;
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int scub_write_mil(volatile unsigned short *base, int slot, short data, short fc_ifc_addr)
{
   criticalSectionEnter();
   base[CALC_OFFS(slot) + MIL_SIO3_TX_DATA ] = data;
   base[CALC_OFFS(slot) + MIL_SIO3_TX_CMD] = fc_ifc_addr;
   criticalSectionExit();
   return OKAY;
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 * non blocking write; uses the tx fifo
 */
int write_mil(volatile unsigned int *base, short data, short fc_ifc_addr)
{
   criticalSectionEnter();
   base[MIL_SIO3_TX_DATA] = data;
   base[MIL_SIO3_TX_CMD]  = fc_ifc_addr;
   criticalSectionExit();
   return OKAY;
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int write_mil_blk(volatile unsigned int *base, short *data, short fc_ifc_addr)
{
   criticalSectionEnter();
   base[MIL_SIO3_TX_DATA] = data[0];
   base[MIL_SIO3_TX_CMD]  = fc_ifc_addr;
   for( unsigned int i = 1; i < MIL_BLOCK_SIZE; i++ )
   {
      base[MIL_SIO3_TX_DATA] = data[i];
   }
   criticalSectionExit();
   return OKAY;
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int status_mil(volatile unsigned int *base, unsigned short *status)
{
   *status = base[MIL_SIO3_STAT];
   return OKAY;
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int scub_status_mil(volatile unsigned short *base, int slot, unsigned short *status)
{
   if( slot >= SCUBUS_START_SLOT && slot <= MAX_SCU_SLAVES )
   {
      *status = base[CALC_OFFS(slot) + MIL_SIO3_STAT];
      return OKAY;
   }

   return ERROR;
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 * blocking read; uses task slot 2
 */
int read_mil(volatile unsigned int *base, short *data, short fc_ifc_addr)
{
  unsigned short rx_data_avail;
  unsigned short rx_err;
  unsigned short rx_req;
  int timeout = 0;

  // write fc and addr to taskram
  base[MIL_SIO3_TX_TASK2] = fc_ifc_addr;

  // wait for task to start (tx fifo full or other tasks running)
  rx_req = base[MIL_SIO3_TX_REQ];
  while(!(rx_req & 0x4) && (timeout < BLOCK_TIMEOUT))
  {
    milWait(TRANSFER_DELAY);
    rx_req = base[MIL_SIO3_TX_REQ];
    timeout++;
  }
  if (timeout > BLOCK_TIMEOUT)
    return RCV_TIMEOUT;

  // wait for task to finish, a read over the dev bus needs at least 40us
  rx_data_avail = base[MIL_SIO3_D_RCVD];
  while(!(rx_data_avail & 0x4) && (timeout < BLOCK_TIMEOUT))
  {
    milWait(TRANSFER_DELAY);
    rx_data_avail = base[MIL_SIO3_D_RCVD];
    timeout++;
  }
  if (timeout > BLOCK_TIMEOUT)
    return RCV_TIMEOUT;

  // task finished
  rx_err = base[MIL_SIO3_D_ERR];
  if ((rx_data_avail & 0x4) && !(rx_err & 0x4))
  {
    // copy received value
    *data = 0xffff & base[MIL_SIO3_RX_TASK2];
    return OKAY;
  }
  else
  {
    // dummy read resets available and error bits
    *data = base[MIL_SIO3_RX_TASK2];
    return RCV_TIMEOUT;
  }
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 * non-blocking
 */
int set_task_mil(volatile unsigned int *base, unsigned char task, short fc_ifc_addr)
{
  if ((task < TASKMIN) || (task > TASKMAX))
    return RCV_TASK_ERR;
  
  // write fc and addr to taskram
  base[MIL_SIO3_TX_TASK1 + task - 1] = fc_ifc_addr;
   
  return OKAY;
}

// blocks until data is available or timeout occurs
/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int get_task_mil(volatile unsigned int *base, unsigned char task, short *data)
{
  unsigned short rx_data_avail;
  unsigned short rx_err;
  unsigned int reg_offset;
  unsigned int bit_offset;

  if ((task < TASKMIN) || (task > TASKMAX))
    return RCV_TASK_ERR;

  // fetch avail and err bits
  reg_offset = task / 16;
  bit_offset = task % 16;
  rx_data_avail = base[MIL_SIO3_D_RCVD + reg_offset];
  // return if data is not available yet
  if(!(rx_data_avail & (1 << bit_offset)))
    return RCV_TASK_BSY;

  rx_err   = base[MIL_SIO3_D_ERR + reg_offset];
  if ((rx_data_avail & (1 << bit_offset)) && !(rx_err & (1 << bit_offset)))
  {
    // copy received value
    *data = 0xffff & base[MIL_SIO3_RX_TASK1 + task - 1];
    return OKAY;
  }
  else
  {
    // dummy read resets available and error bits
    *data = 0xffff & base[MIL_SIO3_RX_TASK1 + task - 1];
    if ((*data & 0xffff) == 0xdead)
      return RCV_TIMEOUT;
    else if ((*data & 0xffff) == 0xbabe)
      return RCV_PARITY;
    else
      return RCV_ERROR;
  }
}

// non-blocking
/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int scub_set_task_mil(volatile unsigned short int *base, int slot, unsigned char task, short fc_ifc_addr)
{
  if ((task < TASKMIN) || (task > TASKMAX))
    return RCV_TASK_ERR;

  // write fc and addr to taskram
  base[CALC_OFFS(slot) + MIL_SIO3_TX_TASK1 + task - 1] = fc_ifc_addr;

  return OKAY;
}

// blocks until data is available or timeout occurs
/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int scub_get_task_mil(volatile unsigned short int *base, int slot, unsigned char task, short *data)
{
  unsigned short rx_data_avail;
  unsigned short rx_err;
  unsigned int reg_offset;
  unsigned int bit_offset;

  if ((task < TASKMIN) || (task > TASKMAX))
    return RCV_TASK_ERR;

  // fetch avail and err bits
  reg_offset = task / 16;
  bit_offset = task % 16;
  rx_data_avail = base[CALC_OFFS(slot) + MIL_SIO3_D_RCVD + reg_offset];
  // return if data is not available yet
  if(!(rx_data_avail & (1 << bit_offset)))
    return RCV_TASK_BSY;
  
  rx_err  = base[CALC_OFFS(slot) + MIL_SIO3_D_ERR + reg_offset];
  if ((rx_data_avail & (1 << bit_offset)) && !(rx_err & (1 << bit_offset)))
  {
    // copy received value
    *data = 0xffff & base[CALC_OFFS(slot) + MIL_SIO3_RX_TASK1 + task - 1];
    return OKAY;
  }
  else
  {
    // dummy read resets available and error bits
    *data = 0xffff & base[CALC_OFFS(slot) + MIL_SIO3_RX_TASK1 + task - 1];
    if ((*data & 0xffff) == 0xdead)
      return RCV_TIMEOUT;
    else if ((*data & 0xffff) == 0xbabe)
      return RCV_PARITY;
    else
      return RCV_ERROR;
  }
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
   while( (scuBusGetSlaveValue16( pSlave, MIL_SIO3_TX_REQ ) & 0x4) == 0 )
   {
      if( timeout > BLOCK_TIMEOUT )
         return RCV_TIMEOUT;
      timeout++;
      milWait( TRANSFER_DELAY );
   }

   /*
    * wait for task to finish, a read over the dev bus needs at least 40us
    */
   while( (scuBusGetSlaveValue16( pSlave, MIL_SIO3_D_RCVD ) & 0x4) == 0 )
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

   if( (scuBusGetSlaveValue16( pSlave, MIL_SIO3_D_ERR ) & 0x4) != 0 )
      return RCV_TIMEOUT;

   return OKAY;
}

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int scub_read_mil( uint16_t *base, const unsigned int slot, uint16_t* pData, const unsigned int fc_ifc_addr )
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

/* reset all task slots */
/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int reset_mil(volatile unsigned *base)
{
//  unsigned short data;
//  int i;
  base[MIL_SIO3_RST] = 0x0;
  milWait(RESET_DELAY);
  base[MIL_SIO3_RST] = 0xff;
  milWait(READY_DELAY);      // added by db; if not, an subsequent write/read results in an error -3

  return OKAY;
  //for (i = TASKMIN; i <= TASKMAX; i++) {
    //data = 0xffff & base[MIL_SIO3_RX_TASK1 + i - 1];
  //}
}

/***********************************************************
 ***********************************************************
 * 
 * 2nd part:  (new) MIL bus library
 *
 ***********************************************************
 ***********************************************************/
/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t writeDevMil(volatile uint32_t *base, uint16_t  ifbAddr, uint16_t  fctCode, uint16_t  data)
{
  // just a wrapper for the function of the original library
  // replace code once original library becomes deprecated
  
  uint16_t fc_ifb_addr;

  fc_ifb_addr = ifbAddr | (fctCode << 8);

  return (int16_t)write_mil((unsigned int *)base, (short)data, (short)fc_ifb_addr);
} // writeDevMil

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t readDevMil(volatile uint32_t *base, uint16_t  ifbAddr, uint16_t  fctCode, uint16_t  *data)
{
  // just a wrapper for the function of the original library
  // replace code once original library becomes deprecated

  uint16_t fc_ifb_addr;

  fc_ifb_addr = ifbAddr | (fctCode << 8);

  return (int16_t)read_mil((unsigned int *)base, (short *)data, (short)fc_ifb_addr);
} //writeDevMil

/*! ---------------------------------------------------------------------------
 * @see scu_mil.h
 */
int16_t echoTestDevMil(volatile uint32_t *base, uint16_t  ifbAddr, uint16_t data)
{
  int32_t  busStatus;
  uint16_t rData = 0x0;

  busStatus = writeDevMil(base, ifbAddr, FC_WR_IFC_ECHO, data);
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
int16_t resetPiggyDevMil(volatile uint32_t *base)
{
  int32_t  busStatus;
  
  // just a wrapper for the function of the original library
  // replace code once original library becomes deprecated

  busStatus = reset_mil((unsigned int *)base);
  if (busStatus != OKAY)
     return MIL_STAT_ERROR;
  else 
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
