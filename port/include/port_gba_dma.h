#ifndef POKEEMERALD_PORT_GBA_DMA_H
#define POKEEMERALD_PORT_GBA_DMA_H

#include <stdbool.h>
#include <stdint.h>

void PortGbaDma_Init(void);
void PortGbaDma_Set(unsigned dmaNum, const void *src, void *dest, uint32_t control);
void PortGbaDma_Stop(unsigned dmaNum);
void PortGbaDma_Run(uint16_t startMode);
bool PortGbaDma_SelfTest(void);

#endif // POKEEMERALD_PORT_GBA_DMA_H
