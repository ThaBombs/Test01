#include "port_gba_audio.h"

#include <string.h>

#include "global.h"
#include "m4a.h"

struct MusicPlayerInfo gMPlayInfo_BGM;
struct MusicPlayerInfo gMPlayInfo_SE1;
struct MusicPlayerInfo gMPlayInfo_SE2;
struct MusicPlayerInfo gMPlayInfo_SE3;
struct SoundInfo gSoundInfo;

static bool sVSyncEnabled;

static void ResetAudioState(void)
{
    memset(&gMPlayInfo_BGM, 0, sizeof(gMPlayInfo_BGM));
    memset(&gMPlayInfo_SE1, 0, sizeof(gMPlayInfo_SE1));
    memset(&gMPlayInfo_SE2, 0, sizeof(gMPlayInfo_SE2));
    memset(&gMPlayInfo_SE3, 0, sizeof(gMPlayInfo_SE3));
    memset(&gSoundInfo, 0, sizeof(gSoundInfo));
    SOUND_INFO_PTR = &gSoundInfo;
}

void m4aSoundInit(void)
{
    ResetAudioState();
    sVSyncEnabled = true;
}

void m4aSoundMain(void)
{
    // Bootstrap sink: game-side music/SFX state can run, but samples are not
    // emitted yet. This will be replaced by the Android audio backend.
}

void m4aSoundVSync(void)
{
}

void m4aSoundVSyncOn(void)
{
    sVSyncEnabled = true;
}

void m4aSoundVSyncOff(void)
{
    sVSyncEnabled = false;
}

void m4aSongNumStart(u16 n)
{
    (void)n;
}

void m4aSongNumStartOrChange(u16 n)
{
    (void)n;
}

void m4aSongNumStop(u16 n)
{
    (void)n;
}

void m4aMPlayAllStop(void)
{
}

void m4aMPlayContinue(struct MusicPlayerInfo *mplayInfo)
{
    (void)mplayInfo;
}

void m4aMPlayFadeOut(struct MusicPlayerInfo *mplayInfo, u16 speed)
{
    (void)mplayInfo;
    (void)speed;
}

void m4aMPlayFadeOutTemporarily(struct MusicPlayerInfo *mplayInfo, u16 speed)
{
    (void)mplayInfo;
    (void)speed;
}

void m4aMPlayFadeIn(struct MusicPlayerInfo *mplayInfo, u16 speed)
{
    (void)mplayInfo;
    (void)speed;
}

void m4aMPlayImmInit(struct MusicPlayerInfo *mplayInfo)
{
    (void)mplayInfo;
}

void m4aMPlayStop(struct MusicPlayerInfo *mplayInfo)
{
    (void)mplayInfo;
}

void m4aMPlayTempoControl(struct MusicPlayerInfo *mplayInfo, u16 tempo)
{
    (void)mplayInfo;
    (void)tempo;
}

void m4aMPlayVolumeControl(struct MusicPlayerInfo *mplayInfo, u16 trackBits, u16 volume)
{
    (void)mplayInfo;
    (void)trackBits;
    (void)volume;
}

void m4aMPlayPitchControl(struct MusicPlayerInfo *mplayInfo, u16 trackBits, s16 pitch)
{
    (void)mplayInfo;
    (void)trackBits;
    (void)pitch;
}

void m4aMPlayPanpotControl(struct MusicPlayerInfo *mplayInfo, u16 trackBits, s8 pan)
{
    (void)mplayInfo;
    (void)trackBits;
    (void)pan;
}

bool PortGbaAudio_SelfTest(void)
{
    m4aSoundInit();
    m4aSoundVSyncOff();
    const bool off = !sVSyncEnabled;
    m4aSoundVSyncOn();
    return off && sVSyncEnabled && SOUND_INFO_PTR == &gSoundInfo;
}
