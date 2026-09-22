#include "port_gba_flash.h"

#include <stdio.h>
#include <string.h>

#include "gba/types.h"
#include "gba/flash_internal.h"
#include "agb_flash.h"

u8 gPortFlashMemory[FLASH_ROM_SIZE_1M];

u8 gFlashTimeoutFlag;
u8 (*PollFlashStatus)(u8 *);
u16 (*WaitForFlashWrite)(u8 phase, u8 *addr, u8 lastData);
u16 (*ProgramFlashSector)(u16 sectorNum, u8 *src);
const struct FlashType *gFlash;
u16 (*ProgramFlashByte)(u16 sectorNum, u32 offset, u8 data);
u16 gFlashNumRemainingBytes;
u16 (*EraseFlashChip)(void);
u16 (*EraseFlashSector)(u16 sectorNum);
const u16 *gFlashMaxTime;

static char sSavePath[512];
static const u16 sHostFlashTimes[12] = {0};

static u8 HostPollFlashStatus(u8 *addr)
{
    return addr != NULL ? *addr : 0xFF;
}

static const struct FlashType sHostFlashType =
{
    .romSize = FLASH_ROM_SIZE_1M,
    .sector =
    {
        .size = 4096,
        .shift = 12,
        .count = 32,
        .top = 0,
    },
    .wait = {0, 0},
    .ids = { .separate = { .makerId = 0xC2, .deviceId = 0x09 } },
};

static void HostFlashTimerIntr(void)
{
}

static void SetupFunctionPointers(void)
{
    PollFlashStatus = HostPollFlashStatus;
    WaitForFlashWrite = WaitForFlashWrite_Common;
    ProgramFlashSector = ProgramFlashSector_MX;
    ProgramFlashByte = ProgramFlashByte_MX;
    EraseFlashChip = EraseFlashChip_MX;
    EraseFlashSector = EraseFlashSector_MX;
    gFlashMaxTime = sHostFlashTimes;
    gFlash = &sHostFlashType;
    gFlashTimeoutFlag = 0;
    gFlashNumRemainingBytes = 0;
}

void PortGbaFlash_Init(const char *dataDirectory)
{
    memset(gPortFlashMemory, 0xFF, sizeof(gPortFlashMemory));
    sSavePath[0] = '\0';
    SetupFunctionPointers();

    if (dataDirectory == NULL || dataDirectory[0] == '\0')
        return;

    const int written = snprintf(
        sSavePath,
        sizeof(sSavePath),
        "%s/game.sav",
        dataDirectory);

    if (written <= 0 || (size_t)written >= sizeof(sSavePath))
    {
        sSavePath[0] = '\0';
        return;
    }

    FILE *file = fopen(sSavePath, "rb");
    if (file == NULL)
        return;

    (void)fread(gPortFlashMemory, 1, sizeof(gPortFlashMemory), file);
    fclose(file);
}

bool PortGbaFlash_Flush(void)
{
    if (sSavePath[0] == '\0')
        return true;

    char tempPath[sizeof(sSavePath) + 8];
    const int written = snprintf(tempPath, sizeof(tempPath), "%s.tmp", sSavePath);
    if (written <= 0 || (size_t)written >= sizeof(tempPath))
        return false;

    FILE *file = fopen(tempPath, "wb");
    if (file == NULL)
        return false;

    const size_t bytesWritten =
        fwrite(gPortFlashMemory, 1, sizeof(gPortFlashMemory), file);
    const int flushResult = fflush(file);
    const int closeResult = fclose(file);

    if (bytesWritten != sizeof(gPortFlashMemory)
        || flushResult != 0
        || closeResult != 0)
    {
        remove(tempPath);
        return false;
    }

    remove(sSavePath);
    if (rename(tempPath, sSavePath) != 0)
    {
        remove(tempPath);
        return false;
    }

    return true;
}

void SwitchFlashBank(u8 bankNum)
{
    (void)bankNum;
}

u16 ReadFlashId(void)
{
    return 0x09C2u;
}

u16 SetFlashTimerIntr(u8 timerNum, void (**intrFunc)(void))
{
    if (timerNum >= 4 || intrFunc == NULL)
        return 1;

    *intrFunc = HostFlashTimerIntr;
    return 0;
}

void StartFlashTimer(u8 phase)
{
    (void)phase;
    gFlashTimeoutFlag = 0;
}

void StopFlashTimer(void)
{
}

void SetReadFlash1(u16 *dest)
{
    (void)dest;
    PollFlashStatus = HostPollFlashStatus;
}

u16 WaitForFlashWrite_Common(u8 phase, u8 *addr, u8 lastData)
{
    (void)phase;
    return (addr != NULL && *addr == lastData) ? 0 : 1;
}

void ReadFlash(u16 sectorNum, u32 offset, u8 *dest, u32 size)
{
    if (dest == NULL)
        return;

    if (sectorNum >= sHostFlashType.sector.count
        || offset > sHostFlashType.sector.size
        || size > sHostFlashType.sector.size - offset)
    {
        memset(dest, 0xFF, size);
        return;
    }

    const size_t base =
        ((size_t)sectorNum << sHostFlashType.sector.shift) + (size_t)offset;
    memcpy(dest, gPortFlashMemory + base, size);
}

u16 EraseFlashChip_MX(void)
{
    memset(gPortFlashMemory, 0xFF, sizeof(gPortFlashMemory));
    return PortGbaFlash_Flush() ? 0 : 1;
}

u16 EraseFlashSector_MX(u16 sectorNum)
{
    if (sectorNum >= sHostFlashType.sector.count)
        return 0x80FFu;

    const size_t base = (size_t)sectorNum << sHostFlashType.sector.shift;
    memset(gPortFlashMemory + base, 0xFF, sHostFlashType.sector.size);
    return PortGbaFlash_Flush() ? 0 : 1;
}

u16 ProgramFlashByte_MX(u16 sectorNum, u32 offset, u8 data)
{
    if (sectorNum >= sHostFlashType.sector.count
        || offset >= sHostFlashType.sector.size)
    {
        return 0x8000u;
    }

    const size_t address =
        ((size_t)sectorNum << sHostFlashType.sector.shift) + (size_t)offset;
    gPortFlashMemory[address] = data;
    return PortGbaFlash_Flush() ? 0 : 1;
}

u16 ProgramFlashSector_MX(u16 sectorNum, u8 *src)
{
    if (sectorNum >= sHostFlashType.sector.count || src == NULL)
        return 0x80FFu;

    const size_t base = (size_t)sectorNum << sHostFlashType.sector.shift;
    memcpy(gPortFlashMemory + base, src, sHostFlashType.sector.size);
    gFlashNumRemainingBytes = 0;
    return PortGbaFlash_Flush() ? 0 : 1;
}

u16 IdentifyFlash(void)
{
    SetupFunctionPointers();
    return 0;
}

u32 ProgramFlashSectorAndVerify(u16 sectorNum, u8 *src)
{
    if (ProgramFlashSector_MX(sectorNum, src) != 0)
        return 1;

    const size_t base = (size_t)sectorNum << sHostFlashType.sector.shift;
    return memcmp(gPortFlashMemory + base, src, sHostFlashType.sector.size) == 0
        ? 0u
        : 1u;
}

bool PortGbaFlash_SelfTest(void)
{
    PortGbaFlash_Init(NULL);

    u8 source[4096];
    u8 readback[4096];

    for (size_t i = 0; i < sizeof(source); ++i)
        source[i] = (u8)(i * 37u + 11u);

    if (ProgramFlashSectorAndVerify(2, source) != 0)
        return false;

    memset(readback, 0, sizeof(readback));
    ReadFlash(2, 0, readback, sizeof(readback));
    if (memcmp(source, readback, sizeof(source)) != 0)
        return false;

    if (EraseFlashSector_MX(2) != 0)
        return false;

    for (size_t i = 0; i < sizeof(source); ++i)
    {
        if (gPortFlashMemory[(2u << 12) + i] != 0xFF)
            return false;
    }

    return true;
}
