#include "random.h"

void * get_address(void)
{
  return (void *)rand64();
}
