
#include <mprintf.h>
#include <stdint.h>
#include <helper_macros.h>

volatile extern uint32_t __atomic_section_nesting_count;

#define _IRQ_IE 0x00000001
#define IRQ_IE ((uint32_t)_IRQ_IE)

OPTIMIZE( "O0" )
void print( uint32_t x )
{
   mprintf( "x = %u\n" );
}

STATIC inline ALWAYS_INLINE uint32_t irqGetEnableRegister( void )
{
   uint32_t ie;
   asm volatile ( "rcsr %0, ie" :"=r"(ie) );
   return ie;
}


#if 1
OPTIMIZE( "O1" )
void criticalSectionEnter( void )
{
   asm volatile
   (
      ".long    __atomic_section_nesting_count             \n\t"
   #ifndef CONFIG_DISABLE_CRITICAL_SECTION
      "wcsr     ie, r0                                     \n\t"
   #endif
      "orhi     r1, r0, hi(__atomic_section_nesting_count) \n\t"
      "ori      r1, r1, lo(__atomic_section_nesting_count) \n\t"
      "lw       r2, (r1+0)                                 \n\t"
      "addi     r2, r2, 1                                  \n\t"
      "sw       (r1+0), r2                                 \n\t"
      :
      :
      : "r1", "r2"
   );
}

#define CONFIG_IRQ_ALSO_ENABLE_IF_COUNTER_ALREADY_ZERO

OPTIMIZE( "O1" )
void criticalSectionExit( void )
{
   asm volatile
   (
      ".long    __atomic_section_nesting_count             \n\t"
      "orhi     r1, r0, hi(__atomic_section_nesting_count) \n\t"
      "ori      r1, r1, lo(__atomic_section_nesting_count) \n\t"
      "lw       r2, (r1+0)                                 \n\t"
   #ifdef CONFIG_IRQ_ALSO_ENABLE_IF_COUNTER_ALREADY_ZERO
      "be       r2, r0, L_ENABLE                           \n\t"
   #else
      "be       r2, r0, L_NO_ENABLE                        \n\t"
   #endif
      "addi     r2, r2, -1                                 \n\t"
      "sw       (r1+0), r2                                 \n\t"
      "bne      r2, r0, L_NO_ENABLE                        \n"
   "L_ENABLE:                                              \n\t"
      "ori      r1, r0, " TO_STRING( _IRQ_IE ) "           \n\t"
      "wcsr     ie, r1                                     \n"
   "L_NO_ENABLE:                                           \n\t"
      :
      :
      : "r1", "r2" 
   );
}
#endif



extern volatile uint32_t __reset_count;

void main( void )
{
   mprintf( "Inline assembler test no: %u\n", __reset_count );
   mprintf( "Address of __atomic_section_nesting_count: 0x%p\n", &__atomic_section_nesting_count );

   mprintf( "__atomic_section_nesting_count = %d, enable = 0x%08X, \n",
             __atomic_section_nesting_count, irqGetEnableRegister() );

   criticalSectionEnter();
   mprintf( "__atomic_section_nesting_count = %d, enable = 0x%08X, \n",
             __atomic_section_nesting_count, irqGetEnableRegister() );

   criticalSectionEnter();
#if 1
   mprintf( "__atomic_section_nesting_count = %d, enable = 0x%08X, \n",
             __atomic_section_nesting_count, irqGetEnableRegister() );

   criticalSectionExit();
   mprintf( "__atomic_section_nesting_count = %d, enable = 0x%08X, \n",
             __atomic_section_nesting_count, irqGetEnableRegister() );

   criticalSectionExit();
   mprintf( "__atomic_section_nesting_count = %d, enable = 0x%08X, \n",
             __atomic_section_nesting_count, irqGetEnableRegister() );

   criticalSectionExit();
   mprintf( "__atomic_section_nesting_count = %d, enable = 0x%08X, \n",
             __atomic_section_nesting_count, irqGetEnableRegister() );

   criticalSectionExit();
   mprintf( "__atomic_section_nesting_count = %d, enable = 0x%08X, \n",
             __atomic_section_nesting_count, irqGetEnableRegister() );

   criticalSectionEnter();
   mprintf( "__atomic_section_nesting_count = %d, enable = 0x%08X, \n",
             __atomic_section_nesting_count, irqGetEnableRegister() );

#endif
   while( 1 );
}
