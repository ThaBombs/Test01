#include "port_gba_host.h"

#include <stddef.h>
#include <string.h>

#include "gba/types.h"
#include "gba/io_reg.h"

struct SoundInfo *SOUND_INFO_PTR = NULL;
uint16_t INTR_CHECK = 0;
void *INTR_VECTOR = NULL;

unsigned char gPortIoRegisters[0x400] ALIGNED(4);
unsigned char gPortPaletteRam[PLTT_SIZE] ALIGNED(4);
unsigned char gPortVram[VRAM_SIZE] ALIGNED(4);
unsigned char gPortOam[OAM_SIZE] ALIGNED(4);

void PortGbaHost_Init(void)
{
    memset(gPortIoRegisters, 0, sizeof(gPortIoRegisters));
    memset(gPortPaletteRam, 0, sizeof(gPortPaletteRam));
    memset(gPortVram, 0, sizeof(gPortVram));
    memset(gPortOam, 0, sizeof(gPortOam));

    SOUND_INFO_PTR = NULL;
    INTR_CHECK = 0;
    INTR_VECTOR = NULL;

    // GBA KEYINPUT is active-low: 1 means released, 0 means pressed.
    REG_KEYINPUT = KEYS_MASK;
}

bool PortGbaHost_SelfTest(void)
{
    PortGbaHost_Init();

    REG_BG0CNT = 0x1234;
    REG_WAITCNT = 0x4321;
    *(volatile uint16_t *)BG_PLTT = 0x7FFF;
    *(volatile uint16_t *)(VRAM + 2u) = 0x55AA;
    *(volatile uint16_t *)OAM = 0x0F0F;

    const bool passed =
        REG_BG0CNT == 0x1234
        && REG_WAITCNT == 0x4321
        && *(volatile uint16_t *)(gPortIoRegisters + REG_OFFSET_BG0CNT) == 0x1234
        && *(volatile uint16_t *)gPortPaletteRam == 0x7FFF
        && *(volatile uint16_t *)(gPortVram + 2u) == 0x55AA
        && *(volatile uint16_t *)gPortOam == 0x0F0F;

    PortGbaHost_Init();
    return passed;
}

void PortGbaHost_SetButtons(uint32_t buttons)
{
    REG_KEYINPUT = (uint16_t)((~buttons) & KEYS_MASK);
}

uint16_t PortGbaHost_GetKeyInput(void)
{
    return REG_KEYINPUT;
}
