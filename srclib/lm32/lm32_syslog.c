/*!
 * @file lm32_syslog.c
 * @brief LM32 version of syslog.
 *
 * @see       lm32_syslog.h
 * @see       lm32_syslog_common.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      22.04.2022
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
//#define CONFIG_DEBUG_LM32LOG
#ifndef __DOCFSM__
 #include <scu_wr_time.h>
 #include <lm32_syslog.h>
 #include <scu_mmu_tag.h>
 #include <dbg.h>
 #ifdef CONFIG_DEBUG_LM32LOG
  #include <mprintf.h>
 #endif
#endif

STATIC MMU_ADDR_T mg_adminOffset = 0;

#ifdef CONFIG_SCU_USE_DDR3 /***** DDR3-ACCESS *****/

#ifndef __DOCFSM__
 #include <scu_ddr3_lm32.h>
#endif
#ifdef CONFIG_RTOS
/*
 * When FreeRTOS is used then the DDR3-access functions are already within
 * a critical section, so that isn't necessary to put it here again.
 */
 #define DDR3_CRITICAL_SECTION_ENTER()
 #define DDR3_CRITICAL_SECTION_EXIT()
#else
  #ifndef __DOCFSM__
   #include <lm32Interrupts.h>
  #endif
 #define DDR3_CRITICAL_SECTION_ENTER() criticalSectionEnter()
 #define DDR3_CRITICAL_SECTION_EXIT()  criticalSectionExit()
#endif

/*! ---------------------------------------------------------------------------
 */
STATIC inline void syslogWriteRam( unsigned int index, const RAM_PAYLOAD_T* pData )
{
   DDR3_CRITICAL_SECTION_ENTER();
   ddr3write64( index, pData );
   DDR3_CRITICAL_SECTION_EXIT();
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline void syslogReadRam( unsigned int index, RAM_PAYLOAD_T* pData )
{
   DDR3_CRITICAL_SECTION_ENTER();
   ddr3read64( pData, index );
   DDR3_CRITICAL_SECTION_EXIT();
}

#else /***** SRAM-ACCESS *****/

#ifndef __DOCFSM__
 #include <scu_sram_lm32.h>
#endif

/*! ---------------------------------------------------------------------------
 */
STATIC inline void syslogWriteRam( unsigned int index, const RAM_PAYLOAD_T* pData )
{
   sramWrite64( index, pData );
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline void syslogReadRam( unsigned int index, RAM_PAYLOAD_T* pData )
{
   sramRead64( pData, index );
}

#endif /* else ifdef CONFIG_SCU_USE_DDR3 */

/*! ---------------------------------------------------------------------------
 */
STATIC void syslogWriteFifoAdmin( const SYSLOG_FIFO_ADMIN_T* pAdmin )
{
   MMU_ADDR_T index = mg_adminOffset;
   for( size_t i = 0; i < SYSLOG_FIFO_ADMIN_SIZE; i++, index++ )
   {
      syslogWriteRam( index, &((RAM_PAYLOAD_T*)pAdmin)[i] );
   }
}

/*! ---------------------------------------------------------------------------
 */
STATIC void syslogReadFifoAdmin( SYSLOG_FIFO_ADMIN_T* pAdmin )
{
   MMU_ADDR_T index = mg_adminOffset;
   for( size_t i = 0; i < SYSLOG_FIFO_ADMIN_SIZE; i++, index++ )
   {
      syslogReadRam( index, &((RAM_PAYLOAD_T*)pAdmin)[i] );
   }
   ramRingDbgPrintIndexes( &pAdmin->admin.indexes, "read" );
}

/*! ---------------------------------------------------------------------------
 * @see lm32_syslog.h
 */
MMU_STATUS_T lm32LogInit( unsigned int numOfItems )
{
   MMU_STATUS_T status;

   if( (status = mmuInit()) != OK )
      return status;

#ifdef CONFIG_LOG_TEST
   mmuDelete();
#endif
   numOfItems *= SYSLOG_FIFO_ITEM_SIZE;
   numOfItems += SYSLOG_FIFO_ADMIN_SIZE;

   status = mmuAlloc( TAG_LM32_LOG, &mg_adminOffset, &numOfItems, true );
   if( !mmuIsOkay( status ) )
      return status;

   SYSLOG_FIFO_ADMIN_T fifoAdmin =
   {
      .admin.indexes.offset   = mg_adminOffset + SYSLOG_FIFO_ADMIN_SIZE,
      .admin.indexes.capacity = numOfItems - SYSLOG_FIFO_ADMIN_SIZE,
      .admin.indexes.start    = 0,
      .admin.indexes.end      = 0,
      .admin.wasRead          = 0
   };

   /*
    * In the case the memory segment was already allocated by the tool "mem-mon"
    * it could be, that the allocated memory capacity isn't dividable by the
    * size of a sys-log item. In this case it becomes dividable by sacrificing
    * some 64-bit words.
    */
   fifoAdmin.admin.indexes.capacity -= (fifoAdmin.admin.indexes.capacity % SYSLOG_FIFO_ITEM_SIZE);

   DBPRINT1( "offset: %u\ncapacity: %u\n",
             fifoAdmin.admin.indexes.offset,
             fifoAdmin.admin.indexes.capacity );

   syslogWriteFifoAdmin( &fifoAdmin );

   return status;
}

/*! ---------------------------------------------------------------------------
 * @ingroup LM32_LOG
 */
STATIC inline void syslogPushItem( const SYSLOG_FIFO_ITEM_T* pItem )
{
   SYSLOG_FIFO_ADMIN_T admin;

   criticalSectionEnter();

   syslogReadFifoAdmin( &admin );

   /*
    * Removing the items which has been probably read by the Linux daemon.
    */
   sysLogFifoSynchonizeReadIndex( &admin );

   /*
    * Is enough space for the new item?
    */
   if( sysLogFifoGetRemainingItemCapacity( &admin ) == 0 )
   { /*
      * No, deleting the oldest item to make space.
      */
      sysLogFifoAddToReadIndex( &admin, SYSLOG_FIFO_ITEM_SIZE );
   }

   for( size_t i = 0; i < SYSLOG_FIFO_ITEM_SIZE; i++ )
   {
      syslogWriteRam( sysLogFifoGetWriteIndex( &admin ), &((RAM_PAYLOAD_T*)pItem)[i] );
      sysLogFifoIncWriteIndex( &admin );
   }

   syslogWriteFifoAdmin( &admin );

   criticalSectionExit();
}


/*! ---------------------------------------------------------------------------
 * @ingroup LM32_LOG
 * @brief Returns true if the character is a decimal digit.
 */
STATIC inline bool isDecDigit( const char c )
{
   return (c >= '0') && (c <= '9');
}

/*! ---------------------------------------------------------------------------
 * @ingroup LM32_LOG
 * @brief Returns true if the character can be used as a fill-character.
 */
STATIC inline bool isPaddingChar( const char c )
{
   switch( c )
   {
      case '0': /* No break here! */
      case ' ': /* No break here! */
      case '.': /* No break here! */
      case '_':
      {
         return true;
      }
   }
   return false;
}

#define FSM_DECLARE_STATE( newState, attr... ) newState

typedef enum
{
   FSM_DECLARE_STATE( NORMAL, color=blue ),
   FSM_DECLARE_STATE( PADDING_CHAR, color=green ),
   FSM_DECLARE_STATE( PADDING_SIZE, color=cyan ),
   FSM_DECLARE_STATE( PARAM, color=magenta )
} STATE_T;

#ifdef CONFIG_DEBUG_LM32LOG
 /*! ------------------------------------------------------------------------
  * @ingroup LM32_LOG
  * @brief Prints the actual state of the FSM within vsyslog.
  * @note For debug purposes only!
  */
 STATIC const char* state2String( const STATE_T state )
 {
    #define CASE_STATE( s ) case s: return #s;

    switch( state )
    {
       CASE_STATE( NORMAL )
       CASE_STATE( PADDING_CHAR )
       CASE_STATE( PADDING_SIZE )
       CASE_STATE( PARAM )
    }
    return "unknown";
 }

 #define FSM_TRANSITION_NEXT( target, attr... ) \
 {                                              \
    mprintf( "C = '%c' %s -> %s\n",             \
             *format,                           \
             state2String( state ),             \
             state2String( target ) );          \
    state = target;                             \
    next = true;                                \
    break;                                      \
 }
 
 #define FSM_TRANSITION( target, attr... )      \
 {                                              \
    mprintf( "C = '%c' %s -> %s\n",             \
             *format,                           \
             state2String( state ),             \
             state2String( target ) );          \
    state = target;                             \
    break;                                      \
 }

#else
 #define FSM_TRANSITION( target, attr... ) { state = target; break; }
 #define FSM_TRANSITION_NEXT( target, attr... ) { state = target; next = true; break; }
#endif

#define FSM_INIT_FSM( initState, attr... ) STATE_T state = initState
#define FSM_TRANSITION_SELF( attr...) break

STATIC_ASSERT( sizeof(char*) == sizeof(uint32_t) );

/*! ---------------------------------------------------------------------------
 * @see lm32_syslog.h
 * @todo Why -O0 or -O1 only?
 */
OPTIMIZE( "-O1"  )
void vLm32log( const unsigned int filter, const char* format, va_list ap )
{
#ifdef CONFIG_DEBUG_LM32LOG
   mprintf( "%s( %u, %s )\n",  __func__, filter, format );
#endif

   const uint64_t timestamp = getWrSysTimeSafe();

   SYSLOG_FIFO_ITEM_T item =
   {
   #if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
      .timestamp = SWAP_HALVES_OF_64( timestamp ),
   #else
      .timestamp = timestamp,
   #endif
      .filter = filter,
      .format = (uint32_t)format
   };

   FSM_INIT_FSM( NORMAL, color=blue );

   /*
    * Parsing the format-string whether including additional arguments.
    */
   for( unsigned int i = 0; (*format != '\0') && (i < ARRAY_SIZE(item.param)); format++ )
   {
      bool next;
      do
      {  /*
          * Becones 'true' within macro FSM_TRANSITION_NEXT.
          */
         next = false;
         switch( state )
         {
            case NORMAL:
            {
               if( *format == '%' )
                  FSM_TRANSITION( PADDING_CHAR, label='%' );
               FSM_TRANSITION_SELF();
            }

            case PADDING_CHAR:
            {
               if( *format == '%' )
                  FSM_TRANSITION( NORMAL, label='%' );

               if( isPaddingChar( *format ) )
                  FSM_TRANSITION( PADDING_SIZE, label='padding' );

               if( isDecDigit( *format ) )
                  FSM_TRANSITION_NEXT( PADDING_SIZE, label='or dec digit' );

               FSM_TRANSITION_NEXT( PARAM );
            }

            case PADDING_SIZE:
            {
               if( isDecDigit( *format ) )
                  FSM_TRANSITION_SELF( label='dec digit');
               FSM_TRANSITION_NEXT( PARAM );
            }

            case PARAM:
            {
               switch( *format )
               {
                  case 'S': /* No break here! */
                  case 's': /* No break here! */
                  case 'c': /* No break here! */
                  case 'X': /* No break here! */
                  case 'x': /* No break here! */
                  case 'p': /* No break here! */
                  case 'i': /* No break here! */
                  case 'd': /* No break here! */
                  case 'u': /* No break here! */
                  case 'o': /* No break here! */
               #ifndef CONFIG_NO_BINARY_PRINTF_FORMAT
                  case 'b':
               #endif
                  {
                     item.param[i++] = va_arg( ap, typeof(item.param[0]) );
                     break;
                  }
               }
              #ifdef CONFIG_DEBUG_LM32LOG
               mprintf( "Param[%u] = 0x%08X, %d\n", i-1, item.param[i-1], item.param[i-1] );
              #endif
  
               FSM_TRANSITION( NORMAL );
            }
         } /* switch( state ) */
      }
      while( next );
   } /* for() */

   syslogPushItem( &item );

#ifdef CONFIG_DEBUG_LM32LOG
   mprintf( "\n" );
#endif
}

/*! ---------------------------------------------------------------------------
 * @see lm32_syslog.h
 */
void lm32Log( const unsigned int filter, const char* format, ... )
{
#ifdef CONFIG_DEBUG_LM32LOG
   mprintf( "%s( %u, %s )\n",  __func__, filter, format );
#endif
   va_list ap;

   va_start( ap, format );
   vLm32log( filter, format, ap );
   va_end( ap );
}

/*================================== EOF ====================================*/
