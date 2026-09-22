#ifndef POKEEMERALD_PORT_GBA_HOST_H
#define POKEEMERALD_PORT_GBA_HOST_H

#include <stdbool.h>
#include <stdint.h>

void PortGbaHost_Init(void);
bool PortGbaHost_SelfTest(void);
void PortGbaHost_SetButtons(uint32_t buttons);
uint16_t PortGbaHost_GetKeyInput(void);

#endif // POKEEMERALD_PORT_GBA_HOST_H
