#include "test_utils.h"
#include "instructions_testcode.h"

   /* Test a simple loop of 1 million instructions             */
   /* Most implementations should count be correct within 1%   */
   /* This loop in in assembly language, as compiler generated */
   /* code varies too much.                                    */

int instructions_million(void) {

#if defined(__i386__) || (defined __x86_64__)   
   asm("\txor %%ecx,%%ecx\n"
       "\tmov $499999,%%ecx\n"
       "test_loop:\n"
       "\tdec %%ecx\n"
       "\tjnz test_loop\n"
       : /* no output registers */
       : /* no inputs */
       : "cc", "%ecx" /* clobbered */
    );
    return 0;
#elif defined(__PPC__)
   asm("\tnop                           # to give us an even million\n"
       "\tlis     15,499997@ha          # load high 16-bits of counter\n"
       "\taddi    15,15,499997@l        # load low 16-bits of counter\n"
       "55:\n"
       "\taddic.  15,15,-1              # decrement counter\n"
       "\tbne     0,55b                  # loop until zero\n"
       : /* no output registers */
       : /* no inputs */
       : "cc", "15" /* clobbered */
    );
    return 0;
#elif defined(__ia64__)
   asm("\tmov     loc6=166666           // below is 6 instr.\n"
       ";;                              // because of that we count 4 too few\n"
       "55:\n"
       "\tadd     loc6=-1,loc6          // decrement count\n"
       ";;\n"
       "\tcmp.ne  p2,p3=0,loc6\n"
       "(p2)    br.cond.dptk    55b     // if not zero, loop\n"
       : /* no output registers */
       : /* no inputs */
       : "p2", "loc6" /* clobbered */
    );
    return 0;
#elif defined(__sparc__)
   asm("\tsethi     %%hi(333333), %%l0\n"
       "\tor        %%l0,%%lo(333333),%%l0\n"
       "test_loop:\n"
       "\tdeccc   %%l0             ! decrement count\n"
       "\tbnz     test_loop        ! repeat until zero\n"
       "\tnop                      ! branch delay slot\n"
       : /* no output registers */
       : /* no inputs */
       : "cc", "l0" /* clobbered */
    );
    return 0;
#endif
   
    return CODE_UNIMPLEMENTED;

}


int instructions_fldcw(void) {

#if defined(__i386__) || (defined __x86_64__)   
   int saved_cw,result,cw;
   double three=3.0;
   
   asm( "mov     $100000,%%ecx\n"
        "big_loop:\n"
        "\tfldl    %1                   # load value onto fp stack\n"
        "\tfnstcw  %0                # store control word to mem\n"
        "\tmovzwl  %0, %%eax          # load cw from mem, zero extending\n"
        "\tmovb    $12, %%ah                # set cw for \"round to zero\"\n"
        "\tmovw    %%ax, %3                 # store back to memory\n"
        "\tfldcw   %3                      # save new rounding mode\n"
        "\tfistpl  %2                  # save stack value as integer to mem\n"
        "\tfldcw   %0                # restore old cw\n"
        "\tloop    big_loop                # loop to make the count more obvious\n"
       : /* no output registers */
       : "m"(saved_cw), "m"(three), "m"(result), "m"(cw) /* inputs */
       : "cc", "%ecx","%eax" /* clobbered */
    );
    return 0;
#endif
    return CODE_UNIMPLEMENTED;
}


int instructions_rep(void) {

   char buffer_out[16384];
   
#if defined(__i386__) 
   asm("\tmov       $1000,%%edx\n"
       "\tcld\n"
       "loadstore:                       # test 8-bit store\n"
       "\tmov     $0xd, %%al             # set eax to d\n"
       "\tmov     $16384, %%ecx\n"
       "\tmov     %0, %%edi              # set destination\n"
       "\trep     stosb                  # store d 16384 times, auto-increment\n"
       "\tdec     %%edx\n"
       "\tjnz     loadstore\n"
       : /* outputs */
       : "rm" (buffer_out) /* inputs */
       : "cc", "%esi","%edi","%edx","%ecx","%eax","memory" /* clobbered */
    );
    return 0;
#elif defined (__x86_64__)   
   asm("\tmov       $1000,%%edx\n"
       "\tcld\n"
       "loadstore:                       # test 8-bit store\n"
       "\tmov     $0xd, %%al             # set eax to d\n"
       "\tmov     $16384, %%ecx\n"
       "\tmov     %0, %%rdi              # set destination\n"
       "\trep     stosb                  # store d 16384 times, auto-increment\n"
       "\tdec     %%edx\n"
       "\tjnz     loadstore\n"
       : /* outputs */
       : "rm" (buffer_out) /* inputs */
       : "cc", "%esi","%edi","%edx","%ecx","%eax","memory" /* clobbered */
    );
    return 0;
#endif
    return CODE_UNIMPLEMENTED;

}
