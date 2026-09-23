#include "port_test_field_menu.h"
#include "port_test_overworld.h"

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

// menu.c expects this common Emerald cursor string from strings.c. Pulling the
// entire strings translation unit would retain a large unrelated data set.
const u8 gText_SelectorArrow3[] = _("▶");

static const u8 sTextOption[] = _("OPTION");
static const u8 sTextClose[] = _("CLOSE");
static const u8 sTextColors[] =
{
    TEXT_COLOR_WHITE,
    TEXT_COLOR_DARK_GRAY,
    TEXT_COLOR_WHITE,
};
static const u8 sSelectedTextColors[] =
{
    4,
    TEXT_COLOR_DARK_GRAY,
    4,
};

static const u16 sFieldMenuPalette[16] =
{
    RGB_BLACK,
    RGB_WHITE,
    RGB_BLACK,
    RGB_WHITE,
    RGB(24, 24, 24),
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
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 1,
        .width = 8,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 1,
    },
    DUMMY_WIN_TEMPLATE
};

static bool32 sOpen;
static u8 sSelection;

static void DrawFieldMenuContents(void)
{
    FillWindowPixelBuffer(0, PIXEL_FILL(TEXT_COLOR_WHITE));

    FillWindowPixelRect(
        0,
        PIXEL_FILL(4),
        4,
        sSelection * 16,
        56,
        16);

    AddTextPrinterParameterized3(
        0,
        FONT_NORMAL,
        12,
        1,
        sSelection == PORT_FIELD_MENU_OPTION ? sSelectedTextColors : sTextColors,
        TEXT_SKIP_DRAW,
        sTextOption);
    AddTextPrinterParameterized3(
        0,
        FONT_NORMAL,
        12,
        17,
        sSelection == PORT_FIELD_MENU_CLOSE ? sSelectedTextColors : sTextColors,
        TEXT_SKIP_DRAW,
        sTextClose);
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
    sSelection = PORT_FIELD_MENU_OPTION;
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
