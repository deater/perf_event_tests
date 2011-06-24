#include <stdio.h>
#include <stdlib.h>

#include "perf_event.h"
#include "nops_testcode.h"


/* This code has 1,000,000,000 total nops       */

int nops_testcode(void) {
   
#if defined(__i386__) || (defined __x86_64__)   
   asm("\txor %%ecx,%%ecx\n"
       "\tmov $20000000,%%ecx\n"
       "test_loop:\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "nop\n"
       "\tdec %%ecx\n"
       "\tjnz test_loop\n"
       : /* no output registers */
       : /* no inputs */
       : "cc", "%ecx", "%eax" /* clobbered */
    );
    return 0;
#endif
    return -1;

}



