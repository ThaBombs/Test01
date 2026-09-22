#ifndef POKEEMERALD_PORT_GBA_FLASH_H
#define POKEEMERALD_PORT_GBA_FLASH_H

#include <stdbool.h>

void PortGbaFlash_Init(const char *dataDirectory);
bool PortGbaFlash_Flush(void);
bool PortGbaFlash_SelfTest(void);

#endif // POKEEMERALD_PORT_GBA_FLASH_H
