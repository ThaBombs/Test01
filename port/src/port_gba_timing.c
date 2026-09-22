#include "port_gba_timing.h"

#include <errno.h>
#include <stdint.h>
#include <time.h>

#include "port_gba_dma.h"
#include "gba/types.h"
#include "gba/io_reg.h"

// One GBA video frame is ~280896 CPU cycles at 16.78 MHz.
// Keeping the original cadence avoids subtly accelerating timers/game logic.
#define GBA_FRAME_NS 16742706LL

static int64_t sNextFrameNs;

static int64_t MonotonicNanos(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

static void SleepNanos(int64_t nanoseconds)
{
    if (nanoseconds <= 0)
        return;

    struct timespec requested = {
        .tv_sec = (time_t)(nanoseconds / 1000000000LL),
        .tv_nsec = (long)(nanoseconds % 1000000000LL),
    };

    while (nanosleep(&requested, &requested) != 0 && errno == EINTR)
        ;
}

void PortGbaTiming_Init(void)
{
    sNextFrameNs = MonotonicNanos() + GBA_FRAME_NS;
    PortGbaTiming_BeginVisibleFrame();
}

void PortGbaTiming_BeginVisibleFrame(void)
{
    REG_VCOUNT = 0;
    REG_DISPSTAT &= (u16)~(DISPSTAT_VBLANK | DISPSTAT_HBLANK | DISPSTAT_VCOUNT);
}

void PortGbaTiming_EnterVBlank(void)
{
    REG_VCOUNT = 160;
    REG_DISPSTAT |= DISPSTAT_VBLANK;
    REG_DISPSTAT &= (u16)~DISPSTAT_HBLANK;

    const u16 compareLine = (REG_DISPSTAT >> 8) & 0xFFu;
    if (compareLine == REG_VCOUNT)
        REG_DISPSTAT |= DISPSTAT_VCOUNT;
    else
        REG_DISPSTAT &= (u16)~DISPSTAT_VCOUNT;

    PortGbaDma_Run(DMA_START_VBLANK);
    REG_IF |= INTR_FLAG_VBLANK;
}

void PortGbaTiming_LeaveVBlank(void)
{
    REG_DISPSTAT &= (u16)~(DISPSTAT_VBLANK | DISPSTAT_HBLANK | DISPSTAT_VCOUNT);
    REG_VCOUNT = 0;
}

void PortGbaTiming_WaitForNextFrame(void)
{
    const int64_t now = MonotonicNanos();

    if (sNextFrameNs == 0)
        sNextFrameNs = now + GBA_FRAME_NS;

    // If Android suspended us or debugging paused execution, don't attempt to
    // catch up by running a burst of old frames.
    if (now - sNextFrameNs > GBA_FRAME_NS * 4)
        sNextFrameNs = now + GBA_FRAME_NS;

    SleepNanos(sNextFrameNs - now);
    sNextFrameNs += GBA_FRAME_NS;
}

// This symbol replaces the GBA BIOS syscall. The real game frame loop will
// normally be driven directly by Android, but keeping this functional preserves
// compatibility with code paths that still use the traditional wait call.
void VBlankIntrWait(void)
{
    PortGbaTiming_WaitForNextFrame();
    PortGbaTiming_EnterVBlank();
    PortGbaTiming_LeaveVBlank();
}

bool PortGbaTiming_SelfTest(void)
{
    const u16 originalDispstat = REG_DISPSTAT;
    const u16 originalVcount = REG_VCOUNT;
    const u16 originalIf = REG_IF;

    REG_DISPSTAT = (160u << 8);
    REG_IF = 0;

    PortGbaTiming_BeginVisibleFrame();
    const bool visibleOk =
        REG_VCOUNT == 0
        && (REG_DISPSTAT & DISPSTAT_VBLANK) == 0;

    PortGbaTiming_EnterVBlank();
    const bool vblankOk =
        REG_VCOUNT == 160
        && (REG_DISPSTAT & DISPSTAT_VBLANK) != 0
        && (REG_DISPSTAT & DISPSTAT_VCOUNT) != 0
        && (REG_IF & INTR_FLAG_VBLANK) != 0;

    PortGbaTiming_LeaveVBlank();
    const bool leaveOk =
        REG_VCOUNT == 0
        && (REG_DISPSTAT & DISPSTAT_VBLANK) == 0;

    REG_DISPSTAT = originalDispstat;
    REG_VCOUNT = originalVcount;
    REG_IF = originalIf;

    return visibleOk && vblankOk && leaveOk;
}
