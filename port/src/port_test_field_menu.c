#include "port_test_field_menu.h"
#include "port_test_overworld.h"

#include "global.h"
#include "bg.h"
#include "menu.h"
#include "main.h"
#include "palette.h"
#include "text.h"
#include "window.h"
#include "constants/rgb.h"

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
static const u8 sTextColors[] = {2, 1, 3};

static const u16 sFieldMenuPalette[16] =
{
    RGB_BLACK,
    RGB_WHITE,
    RGB_BLACK,
    RGB(18, 18, 18),
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

static void DrawFieldMenu(void)
{
    FillWindowPixelBuffer(0, PIXEL_FILL(1));
    AddTextPrinterParameterized3(0, FONT_NORMAL, 8, 1, sTextColors, TEXT_SKIP_DRAW, sTextOption);
    AddTextPrinterParameterized3(0, FONT_NORMAL, 8, 17, sTextColors, TEXT_SKIP_DRAW, sTextClose);
    PutWindowTilemap(0);
    InitMenuNormal(0, FONT_NORMAL, 0, 1, 16, PORT_FIELD_MENU_COUNT, 0);
    CopyWindowToVram(0, COPYWIN_FULL);
    ShowBg(0);
    ScheduleBgCopyTilemapToVram(0);
}

void PortTestFieldMenu_Init(void)
{
    InitWindows(sFieldMenuWindows);
    DeactivateAllTextPrinters();
    LoadPalette(sFieldMenuPalette, BG_PLTT_ID(15), sizeof(sFieldMenuPalette));
    sOpen = FALSE;
    HideBg(0);
}

void PortTestFieldMenu_Close(void)
{
    if (!sOpen)
        return;

    ClearWindowTilemap(0);
    CopyWindowToVram(0, COPYWIN_MAP);
    HideBg(0);
    ScheduleBgCopyTilemapToVram(0);
    sOpen = FALSE;
}

bool32 PortTestFieldMenu_Update(void)
{
    if (!sOpen)
    {
        if (JOY_NEW(START_BUTTON))
        {
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

    const s8 choice = Menu_ProcessInputNoWrap();
    if (choice == PORT_FIELD_MENU_OPTION)
    {
        sOpen = FALSE;
        PortGame_OpenTestOptions();
        return TRUE;
    }
    if (choice == PORT_FIELD_MENU_CLOSE)
    {
        PortTestFieldMenu_Close();
        return TRUE;
    }

    return TRUE;
}
