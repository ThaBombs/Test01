#include "port_game_bootstrap.h"

#include <android/log.h>
#include <stdarg.h>
#include <stdlib.h>

#include "global.h"
#include "agb_flash.h"
#include "bg.h"
#include "gpu_regs.h"
#include "load_save.h"
#include "play_time.h"
#include "scanline_effect.h"
#include "text.h"
#include "test_runner.h"
#include "trainer_hill.h"

#define LOG_TAG "PokeemeraldEngine"

static u64 sBootstrapFrameCount;

struct SaveBlock3 gSaveblock3 = {0};
struct SaveBlock2ASLR gSaveblock2 = {0};
struct SaveBlock1ASLR gSaveblock1 = {0};
struct PokemonStorageASLR gPokemonStorage = {0};

bool32 gFlashMemoryPresent = TRUE;
struct SaveBlock1 *gSaveBlock1Ptr = &gSaveblock1.block;
struct SaveBlock2 *gSaveBlock2Ptr = &gSaveblock2.block;
struct SaveBlock3 *gSaveBlock3Ptr = &gSaveblock3;
struct PokemonStorage *gPokemonStoragePtr = &gPokemonStorage.block;

u32 *gTrainerHillVBlankCounter = NULL;
const bool8 gTestRunnerEnabled = FALSE;
const bool8 gTestRunnerSkipIsFail = FALSE;
u32 gBattleTypeFlags = 0;

void CheckForFlashMemory(void)
{
    gFlashMemoryPresent = (IdentifyFlash() == 0);
}

void PlayTimeCounter_Update(void)
{
    // The proper play-time module will replace this once the new-game/save
    // path is linked. Keeping it inert during bootstrap avoids advancing a
    // save before one has been initialized.
}

void SetDefaultFontsPointer(void)
{
    // Font tables are initialized when the text/UI stack is brought online.
}

void ScanlineEffect_Stop(void)
{
    // Host rendering has no GBA scanline DMA running during bootstrap.
}

void PortGame_InitialCallback(void)
{
    if (sBootstrapFrameCount++ == 0)
    {
        __android_log_print(
            ANDROID_LOG_INFO,
            LOG_TAG,
            "Emerald main callback reached from Android frame loop.");
    }

    // Intentionally minimal first native callback. Replacing this with the
    // original intro/title callback is the next engine-integration milestone.
}

void AssertfCrashScreen(const void *return0, const char *fmt, ...)
{
    (void)return0;
    va_list args;
    va_start(args, fmt);
    __android_log_vprint(ANDROID_LOG_ERROR, LOG_TAG, fmt, args);
    va_end(args);
}

_Noreturn void FatalfCrashScreen(const void *return0, const char *fmt, ...)
{
    (void)return0;
    va_list args;
    va_start(args, fmt);
    __android_log_vprint(ANDROID_LOG_FATAL, LOG_TAG, fmt, args);
    va_end(args);
    abort();
}
