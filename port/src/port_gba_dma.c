#include "port_gba_dma.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "gba/types.h"
#include "gba/io_reg.h"

#define PORT_DMA_COUNT 4u
#define PORT_DMA_DEST_MASK 0x0060u
#define PORT_DMA_SRC_MASK  0x0180u

struct PortDmaChannel
{
    const uint8_t *src;
    uint8_t *dest;
    uint32_t control;
    bool active;
};

static struct PortDmaChannel sChannels[PORT_DMA_COUNT];

static volatile uint32_t *ControlRegister(unsigned dmaNum)
{
    if (dmaNum >= PORT_DMA_COUNT)
        return NULL;

    return (volatile uint32_t *)(gPortIoRegisters
        + REG_OFFSET_DMA0CNT
        + dmaNum * 12u);
}

static void AdvancePointer(const uint8_t **ptr, uint16_t mode, size_t unit)
{
    if (mode == DMA_SRC_INC)
        *ptr += unit;
    else if (mode == DMA_SRC_DEC)
        *ptr -= unit;
}

static void AdvanceDest(uint8_t **ptr, uint16_t mode, size_t unit)
{
    if (mode == DMA_DEST_INC || mode == DMA_DEST_RELOAD)
        *ptr += unit;
    else if (mode == DMA_DEST_DEC)
        *ptr -= unit;
}

static void ExecuteTransfer(unsigned dmaNum)
{
    struct PortDmaChannel *channel = &sChannels[dmaNum];
    const uint16_t flags = (uint16_t)(channel->control >> 16);
    uint32_t count = channel->control & 0xFFFFu;

    if (!channel->active || (flags & DMA_ENABLE) == 0)
        return;

    if (count == 0)
        count = (dmaNum == 3u) ? 0x10000u : 0x4000u;

    const size_t unit = (flags & DMA_32BIT) ? sizeof(uint32_t) : sizeof(uint16_t);
    const uint16_t srcMode = flags & PORT_DMA_SRC_MASK;
    const uint16_t destMode = flags & PORT_DMA_DEST_MASK;

    const uint8_t *src = channel->src;
    uint8_t *dest = channel->dest;
    const uint8_t *originalSrc = src;
    uint8_t *originalDest = dest;

    if (src == NULL || dest == NULL)
    {
        channel->active = false;
        return;
    }

    for (uint32_t i = 0; i < count; ++i)
    {
        if (unit == sizeof(uint32_t))
        {
            uint32_t value;
            memcpy(&value, src, sizeof(value));
            memcpy(dest, &value, sizeof(value));
        }
        else
        {
            uint16_t value;
            memcpy(&value, src, sizeof(value));
            memcpy(dest, &value, sizeof(value));
        }

        AdvancePointer(&src, srcMode, unit);
        AdvanceDest(&dest, destMode, unit);
    }

    if ((flags & DMA_REPEAT) != 0
        && (flags & DMA_START_MASK) != DMA_START_NOW)
    {
        channel->src = srcMode == DMA_SRC_FIXED ? originalSrc : src;
        channel->dest = destMode == DMA_DEST_RELOAD ? originalDest : dest;
    }
    else
    {
        channel->active = false;
        volatile uint32_t *reg = ControlRegister(dmaNum);
        if (reg != NULL)
            *reg &= ~((uint32_t)DMA_ENABLE << 16);
    }
}

void PortGbaDma_Init(void)
{
    memset(sChannels, 0, sizeof(sChannels));
}

void PortGbaDma_Set(unsigned dmaNum, const void *src, void *dest, uint32_t control)
{
    if (dmaNum >= PORT_DMA_COUNT)
        return;

    struct PortDmaChannel *channel = &sChannels[dmaNum];
    channel->src = (const uint8_t *)src;
    channel->dest = (uint8_t *)dest;
    channel->control = control;
    channel->active = ((control >> 16) & DMA_ENABLE) != 0;

    volatile uint32_t *reg = ControlRegister(dmaNum);
    if (reg != NULL)
        *reg = control;

    const uint16_t flags = (uint16_t)(control >> 16);
    if (channel->active && (flags & DMA_START_MASK) == DMA_START_NOW)
        ExecuteTransfer(dmaNum);
}

void PortGbaDma_Stop(unsigned dmaNum)
{
    if (dmaNum >= PORT_DMA_COUNT)
        return;

    sChannels[dmaNum].active = false;
    sChannels[dmaNum].control = 0;

    volatile uint32_t *reg = ControlRegister(dmaNum);
    if (reg != NULL)
        *reg = 0;
}

void PortGbaDma_Run(uint16_t startMode)
{
    for (unsigned dmaNum = 0; dmaNum < PORT_DMA_COUNT; ++dmaNum)
    {
        const uint16_t flags = (uint16_t)(sChannels[dmaNum].control >> 16);

        if (sChannels[dmaNum].active
            && (flags & DMA_START_MASK) == startMode)
        {
            ExecuteTransfer(dmaNum);
        }
    }
}

bool PortGbaDma_SelfTest(void)
{
    PortGbaDma_Init();

    uint32_t src32[4] = {1u, 2u, 3u, 4u};
    uint32_t dst32[4] = {0u, 0u, 0u, 0u};
    const uint32_t copyControl =
        ((uint32_t)(DMA_ENABLE | DMA_START_NOW | DMA_32BIT
            | DMA_SRC_INC | DMA_DEST_INC) << 16)
        | 4u;

    PortGbaDma_Set(3u, src32, dst32, copyControl);

    uint16_t fillValue = 0x55AAu;
    uint16_t dst16[4] = {0u, 0u, 0u, 0u};
    const uint32_t fillControl =
        ((uint32_t)(DMA_ENABLE | DMA_START_NOW | DMA_16BIT
            | DMA_SRC_FIXED | DMA_DEST_INC) << 16)
        | 4u;

    PortGbaDma_Set(3u, &fillValue, dst16, fillControl);

    return memcmp(src32, dst32, sizeof(src32)) == 0
        && dst16[0] == fillValue
        && dst16[1] == fillValue
        && dst16[2] == fillValue
        && dst16[3] == fillValue;
}
