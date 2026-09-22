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


void SetPokemonCryStereo(u32 val)
{
    // Audio output is not connected yet. Keep the real Emerald option menu
    // functional and preserve the selected preference in gSaveBlock2.
    (void)val;
}


u32 GetGlyphWidth_Braille(u16 glyphId, bool32 isJapanese)
{
    // Emerald's Braille font is fixed-width at 16 pixels.
    (void)glyphId;
    (void)isJapanese;
    return 16;
}


u8 gStringVar1[0x100] = {0};
u8 gStringVar2[0x100] = {0};
u8 gStringVar3[0x100] = {0};

const u8 *DynamicPlaceholderTextUtil_GetPlaceholderPtr(u8 idx)
{
    static const u8 sEmptyPlaceholder[] = { EOS };
    (void)idx;
    return sEmptyPlaceholder;
}

u32 GetGlyphWidth_Braille(u16 glyphId, bool32 isJapanese)
{
    (void)glyphId;
    (void)isJapanese;
    return 16;
}

int GetStringRightAlignXOffset(int fontId, const u8 *str, int totalWidth)
{
    const int stringWidth = GetStringWidth(fontId, str, 0);
    return totalWidth > stringWidth ? totalWidth - stringWidth : 0;
}
