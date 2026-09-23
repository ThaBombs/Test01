#include "port_test_dialogue.h"

#include "global.h"
#include "bg.h"
#include "main.h"
#include "menu.h"
#include "text.h"
#include "text_window.h"
#include "window.h"

#define PORT_DIALOGUE_FRAME_TILE 0x1E0
#define PORT_DIALOGUE_FRAME_PAL  14

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

static const u8 sTextColors[] = {2, 1, 3};

static u8 sDialogueWindowId = WINDOW_NONE;
static bool32 sDialogueOpen;

void PortTestDialogue_Init(void)
{
    sDialogueWindowId = AddWindow(&sDialogueWindow);
    sDialogueOpen = FALSE;
}

bool32 PortTestDialogue_Open(const u8 *text)
{
    if (sDialogueOpen || sDialogueWindowId == WINDOW_NONE || text == NULL)
        return FALSE;

    FillWindowPixelBuffer(sDialogueWindowId, PIXEL_FILL(1));
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
        ClearWindowTilemap(sDialogueWindowId);
        CopyWindowToVram(sDialogueWindowId, COPYWIN_MAP);
        HideBg(0);
        ScheduleBgCopyTilemapToVram(0);
        sDialogueOpen = FALSE;
    }

    return TRUE;
}

bool32 PortTestDialogue_IsOpen(void)
{
    return sDialogueOpen;
}
