#include "port_test_dialogue.h"

#include "global.h"
#include "bg.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/rgb.h"

#define PORT_DIALOGUE_FRAME_TILE 0x1E0
#define PORT_DIALOGUE_FRAME_PAL  14
#define PORT_DIALOGUE_TEXT_PAL   15

static const struct WindowTemplate sDialogueWindow =
{
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 14,
    .width = 28,
    .height = 5,
    .paletteNum = 15,
    .baseBlock = 0x40,
};

static const u16 sPortDialogueTextPalette[16] =
{
    [TEXT_COLOR_WHITE] = RGB_WHITE,
    [TEXT_COLOR_DARK_GRAY] = RGB_BLACK,
    [TEXT_COLOR_LIGHT_GRAY] = RGB_WHITE,
};

static const u8 sTextColors[] =
{
    TEXT_COLOR_WHITE,
    TEXT_COLOR_DARK_GRAY,
    TEXT_COLOR_WHITE,
};

static u8 sDialogueWindowId = WINDOW_NONE;
static bool32 sDialogueOpen;

void PortTestDialogue_Init(void)
{
    sDialogueWindowId = AddWindow(&sDialogueWindow);
    if (sDialogueWindowId != WINDOW_NONE)
    {
        LoadWindowGfx(
            sDialogueWindowId,
            gSaveBlock2Ptr->optionsWindowFrameType,
            PORT_DIALOGUE_FRAME_TILE,
            BG_PLTT_ID(PORT_DIALOGUE_FRAME_PAL));
        LoadPalette(
            sPortDialogueTextPalette,
            BG_PLTT_ID(PORT_DIALOGUE_TEXT_PAL),
            PLTT_SIZE_4BPP);
    }
    sDialogueOpen = FALSE;
}

bool32 PortTestDialogue_Open(const u8 *text)
{
    if (sDialogueOpen || sDialogueWindowId == WINDOW_NONE || text == NULL)
        return FALSE;

    FillWindowPixelBuffer(sDialogueWindowId, PIXEL_FILL(TEXT_COLOR_WHITE));
    AddTextPrinterParameterized3(
        sDialogueWindowId,
        FONT_NORMAL,
        8,
        1,
        sTextColors,
        TEXT_SKIP_DRAW,
        text);
    PutWindowTilemap(sDialogueWindowId);
    DrawTextBorderOuter(sDialogueWindowId, PORT_DIALOGUE_FRAME_TILE, PORT_DIALOGUE_FRAME_PAL);
    CopyWindowToVram(sDialogueWindowId, COPYWIN_FULL);
    ShowBg(0);
    ScheduleBgCopyTilemapToVram(0);
    sDialogueOpen = TRUE;
    return TRUE;
}

bool32 PortTestDialogue_Update(void)
{
    if (!sDialogueOpen)
        return FALSE;

    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        ClearStdWindowAndFrameToTransparent(sDialogueWindowId, TRUE);
        ScheduleBgCopyTilemapToVram(0);
        HideBg(0);
        sDialogueOpen = FALSE;
    }

    return TRUE;
}

bool32 PortTestDialogue_IsOpen(void)
{
    return sDialogueOpen;
}
