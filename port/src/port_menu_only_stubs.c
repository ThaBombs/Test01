#include "global.h"
#include "event_data.h"
#include "field_name_box.h"
#include "sound.h"
#include "text.h"

// Temporary menu-only compatibility shims.
//
// These keep the first native Android milestone focused on rendering and
// operating the real Emerald main menu. The corresponding full subsystems
// (save/event flags, m4a audio, Braille, and overworld dialogue nameboxes)
// are linked back in as their Android ports come online.

bool8 FlagGet(u16 id)
{
    (void)id;
    return FALSE;
}

void PlayBGM(u16 songNum)
{
    (void)songNum;
}

void PlaySE(u16 songNum)
{
    (void)songNum;
}

bool8 IsSEPlaying(void)
{
    return FALSE;
}

u16 FontFunc_Braille(struct TextPrinter *textPrinter)
{
    (void)textPrinter;
    return RENDER_FINISH;
}

const u8 *const gSpeakerNamesTable[] =
{
    0,
};

void TrySpawnAndShowNamebox(const u8 *speaker, u32 tileNum)
{
    (void)speaker;
    (void)tileNum;
}
