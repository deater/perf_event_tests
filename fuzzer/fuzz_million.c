#include "fuzzer_determinism.h"

#include "../include/instructions_testcode.h"

void run_a_million_instructions(void) {

	if (ignore_but_dont_skip.million) return;

	instructions_million();

}
