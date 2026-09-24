#include "port_test_field_menu.h"
#include "port_test_overworld.h"
#include "port_test_dialogue.h"
#include "port_test_battle.h"

#include "global.h"
#include "bg.h"
#include "menu.h"
#include "main.h"
#include "palette.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/rgb.h"

#define PORT_FIELD_MENU_FRAME_TILE 0x1E0
#define PORT_FIELD_MENU_FRAME_PAL  14

enum
{
    PORT_FIELD_MENU_OPTION,
    PORT_FIELD_MENU_CLOSE,
    PORT_FIELD_MENU_COUNT,
};

enum
{
    PORT_FIELD_WINDOW_MENU,
    PORT_FIELD_WINDOW_STARTER,
};

enum
{
    PORT_STARTER_MENU_TREECKO,
    PORT_STARTER_MENU_TORCHIC,
    PORT_STARTER_MENU_MUDKIP,
    PORT_STARTER_MENU_COUNT,
};

// Common Emerald UI strings now come from the real strings.c translation unit.

static const u8 sTextOption[] = _("OPTION");
static const u8 sTextClose[] = _("CLOSE");
static const u8 sTextCursor[] = _("▶");
static const u8 sTextStarterHeader[] = _("CHOOSE A POKéMON");
static const u8 sTextTreecko[] = _("TREECKO");
static const u8 sTextTorchic[] = _("TORCHIC");
static const u8 sTextMudkip[] = _("MUDKIP");
static const u8 sTextTreeckoChosen[] =
    _("Go, TREECKO!\nThe wild ZIGZAGOON attacks!");
static const u8 sTextTorchicChosen[] =
    _("Go, TORCHIC!\nThe wild ZIGZAGOON attacks!");
static const u8 sTextMudkipChosen[] =
    _("Go, MUDKIP!\nThe wild ZIGZAGOON attacks!");
static const u8 sTextTreeckoReady[] =
    _("TREECKO is ready.\nThe wild ZIGZAGOON is attacking!");
static const u8 sTextTorchicReady[] =
    _("TORCHIC is ready.\nThe wild ZIGZAGOON is attacking!");
static const u8 sTextMudkipReady[] =
    _("MUDKIP is ready.\nThe wild ZIGZAGOON is attacking!");

static const u8 *const sStarterNames[PORT_STARTER_MENU_COUNT] =
{
    sTextTreecko,
    sTextTorchic,
    sTextMudkip,
};

static const u8 *const sStarterChosenText[PORT_STARTER_MENU_COUNT] =
{
    sTextTreeckoChosen,
    sTextTorchicChosen,
    sTextMudkipChosen,
};

static const u8 *const sStarterReadyText[PORT_STARTER_MENU_COUNT] =
{
    sTextTreeckoReady,
    sTextTorchicReady,
    sTextMudkipReady,
};
static const u8 sTextColors[] =
{
    TEXT_COLOR_WHITE,
    TEXT_COLOR_DARK_GRAY,
    TEXT_COLOR_WHITE,
};

static const u16 sFieldMenuPalette[16] =
{
    RGB_BLACK,
    RGB_WHITE,
    RGB_BLACK,
    RGB_WHITE,
    RGB_WHITE,
    RGB_WHITE,
    RGB_WHITE,
    RGB_WHITE,
    RGB_WHITE,
    RGB_WHITE,
    RGB_WHITE,
    RGB_WHITE,
    RGB_WHITE,
    RGB_WHITE,
    RGB_WHITE,
    RGB_WHITE,
};

static const struct WindowTemplate sFieldMenuWindows[] =
{
    [PORT_FIELD_WINDOW_MENU] = {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 1,
        .width = 8,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 1,
    },
    [PORT_FIELD_WINDOW_STARTER] = {
        .bg = 0,
        .tilemapLeft = 14,
        .tilemapTop = 4,
        .width = 15,
        .height = 8,
        .paletteNum = 15,
        .baseBlock = 0xD0,
    },
    DUMMY_WIN_TEMPLATE
};

static bool32 sOpen;
static bool32 sStarterOpen;
static u8 sSelection;
static u8 sStarterSelection;

static void DrawStarterChoiceContents(void)
{
    FillWindowPixelBuffer(
        PORT_FIELD_WINDOW_STARTER,
        PIXEL_FILL(TEXT_COLOR_WHITE));
    AddTextPrinterParameterized3(
        PORT_FIELD_WINDOW_STARTER,
        FONT_NORMAL,
        8,
        1,
        sTextColors,
        TEXT_SKIP_DRAW,
        sTextStarterHeader);

    for (u32 i = 0; i < PORT_STARTER_MENU_COUNT; ++i)
    {
        AddTextPrinterParameterized3(
            PORT_FIELD_WINDOW_STARTER,
            FONT_NORMAL,
            20,
            17 + i * 16,
            sTextColors,
            TEXT_SKIP_DRAW,
            sStarterNames[i]);
    }

    AddTextPrinterParameterized3(
        PORT_FIELD_WINDOW_STARTER,
        FONT_NORMAL,
        4,
        17 + sStarterSelection * 16,
        sTextColors,
        TEXT_SKIP_DRAW,
        sTextCursor);
}

static void DrawStarterChoice(void)
{
    DrawStarterChoiceContents();
    PutWindowTilemap(PORT_FIELD_WINDOW_STARTER);
    DrawTextBorderOuter(
        PORT_FIELD_WINDOW_STARTER,
        PORT_FIELD_MENU_FRAME_TILE,
        PORT_FIELD_MENU_FRAME_PAL);
    CopyWindowToVram(PORT_FIELD_WINDOW_STARTER, COPYWIN_FULL);
    ShowBg(0);
    ScheduleBgCopyTilemapToVram(0);
}

static void CloseStarterChoice(void)
{
    if (!sStarterOpen)
        return;

    ClearStdWindowAndFrameToTransparent(
        PORT_FIELD_WINDOW_STARTER,
        TRUE);
    ScheduleBgCopyTilemapToVram(0);
    HideBg(0);
    sStarterOpen = FALSE;
}

bool32 PortTestFieldMenu_OpenStarterChoice(void)
{
    if (sOpen || sStarterOpen)
        return FALSE;

    const u8 chosenStarter = PortGame_GetChosenStarter();
    if (chosenStarter < PORT_STARTER_MENU_COUNT)
    {
        PortTestBattle_RequestFirstBattle();
        return PortTestDialogue_Open(sStarterReadyText[chosenStarter]);
    }

    sStarterSelection = PORT_STARTER_MENU_TREECKO;
    sStarterOpen = TRUE;
    DrawStarterChoice();
    return TRUE;
}

static void DrawFieldMenuContents(void)
{
    FillWindowPixelBuffer(0, PIXEL_FILL(TEXT_COLOR_WHITE));
    AddTextPrinterParameterized3(0, FONT_NORMAL, 16, 1, sTextColors, TEXT_SKIP_DRAW, sTextOption);
    AddTextPrinterParameterized3(0, FONT_NORMAL, 16, 17, sTextColors, TEXT_SKIP_DRAW, sTextClose);
    AddTextPrinterParameterized3(
        0,
        FONT_NORMAL,
        2,
        sSelection * 16 + 1,
        sTextColors,
        TEXT_SKIP_DRAW,
        sTextCursor);
}

static void DrawFieldMenu(void)
{
    DrawFieldMenuContents();
    PutWindowTilemap(0);
    DrawTextBorderOuter(0, PORT_FIELD_MENU_FRAME_TILE, PORT_FIELD_MENU_FRAME_PAL);
    CopyWindowToVram(0, COPYWIN_FULL);
    ShowBg(0);
    ScheduleBgCopyTilemapToVram(0);
}

void PortTestFieldMenu_Init(void)
{
    InitWindows(sFieldMenuWindows);
    DeactivateAllTextPrinters();
    LoadPalette(sFieldMenuPalette, BG_PLTT_ID(15), sizeof(sFieldMenuPalette));
    LoadUserWindowBorderGfx(0, PORT_FIELD_MENU_FRAME_TILE, BG_PLTT_ID(PORT_FIELD_MENU_FRAME_PAL));
    sOpen = FALSE;
    sStarterOpen = FALSE;
    sSelection = PORT_FIELD_MENU_OPTION;
    sStarterSelection = PORT_STARTER_MENU_TREECKO;
    HideBg(0);
}

void PortTestFieldMenu_Close(void)
{
    if (!sOpen)
        return;

    ClearStdWindowAndFrameToTransparent(0, TRUE);
    ScheduleBgCopyTilemapToVram(0);
    HideBg(0);
    sOpen = FALSE;
}

bool32 PortTestFieldMenu_Update(void)
{
    if (sStarterOpen)
    {
        if (JOY_NEW(B_BUTTON))
        {
            CloseStarterChoice();
            return TRUE;
        }

        if (JOY_NEW(DPAD_UP))
        {
            if (sStarterSelection == 0)
                sStarterSelection = PORT_STARTER_MENU_COUNT - 1;
            else
                --sStarterSelection;
            DrawStarterChoiceContents();
            CopyWindowToVram(PORT_FIELD_WINDOW_STARTER, COPYWIN_GFX);
            return TRUE;
        }

        if (JOY_NEW(DPAD_DOWN))
        {
            ++sStarterSelection;
            if (sStarterSelection >= PORT_STARTER_MENU_COUNT)
                sStarterSelection = 0;
            DrawStarterChoiceContents();
            CopyWindowToVram(PORT_FIELD_WINDOW_STARTER, COPYWIN_GFX);
            return TRUE;
        }

        if (JOY_NEW(A_BUTTON))
        {
            const u8 chosenStarter = sStarterSelection;
            CloseStarterChoice();
            PortGame_SetChosenStarter(chosenStarter);
            PortTestBattle_RequestFirstBattle();
            PortTestDialogue_Open(sStarterChosenText[chosenStarter]);
            return TRUE;
        }

        return TRUE;
    }

    if (!sOpen)
    {
        if (JOY_NEW(START_BUTTON))
        {
            sSelection = PORT_FIELD_MENU_OPTION;
            sOpen = TRUE;
            DrawFieldMenu();
            return TRUE;
        }
        return FALSE;
    }

    if (JOY_NEW(START_BUTTON | B_BUTTON))
    {
        PortTestFieldMenu_Close();
        return TRUE;
    }

    if (JOY_NEW(DPAD_UP | DPAD_DOWN))
    {
        sSelection ^= 1;
        DrawFieldMenuContents();
        CopyWindowToVram(0, COPYWIN_GFX);
        return TRUE;
    }

    if (JOY_NEW(A_BUTTON))
    {
        if (sSelection == PORT_FIELD_MENU_OPTION)
        {
            PortTestFieldMenu_Close();
            PortGame_OpenTestOptions();
        }
        else
        {
            PortTestFieldMenu_Close();
        }
        return TRUE;
    }

    return TRUE;
}
