
#include <mprintf.h>
#include <stdint.h>

volatile static uint32_t counter = 0;

void sig( uint32_t x )
{
   mprintf( "x = %u\n" );
}

#if 1
void incCounter( void )
{
#if 0
   asm volatile ( "orhi r1, r0, hi(counter) \n\t"
                  "ori  r1, r1, lo(counter) \n\t"
                  "lw       r2, (r1+0)     \n\t"
                  "lw       r1, (r2+0)     \n\t"
                  "addi     r1, r1, 1      \n\t"
                  "sw       (r2+0), r1     \n\t"
                  :"=r"(counter)
                  :"r"(counter)
                  : "r1", "r2"
                );
#else
   counter++;
#endif
}

void decCounter( void )
{
   asm volatile ( "orhi r1, r0, hi(counter) \n\t"
                  "ori  r1, r1, lo(counter) \n\t"
                  "lw       r2, (r1+0)     \n\t"
                  "lw       r1, (r2+0)     \n\t"
                  "addi     r1, r1, -1     \n\t"
                  "sw       (r2+0), r1     \n\t"
                  ://"=r"(counter)
                  ://"r"(counter)
                  : "r1", "r2" );
}
#endif
void main( void )
{
 //  counter = 0;
   mprintf( "Inline assembler test\n" );
   mprintf( "Noch was...\n" );

   mprintf( "counter = %d\n", counter );

   incCounter();
#if 1
   mprintf( "counter = %u\n", counter );
   decCounter();
   mprintf( "counter = %u\n", counter );
   
#endif
   while( 1 );
}
