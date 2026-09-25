#include "port_test_dialogue.h"

#include "global.h"
#include "bg.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/characters.h"
#include "constants/rgb.h"

#define PORT_DIALOGUE_FRAME_TILE 0x1E0
#define PORT_DIALOGUE_FRAME_PAL  14
#define PORT_DIALOGUE_TEXT_PAL   15

// The window is 28 tiles (224 px) wide. Leave padding on both sides so
// proportional FONT_NORMAL glyphs never touch the frame.
#define PORT_DIALOGUE_TEXT_WIDTH       208
#define PORT_DIALOGUE_LINES_PER_PAGE   2
#define PORT_DIALOGUE_PAGE_BUFFER_SIZE 256

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
static const u8 *sDialogueCursor;
static bool32 sDialogueHasMore;
static u8 sDialoguePage[PORT_DIALOGUE_PAGE_BUFFER_SIZE];

static s32 GetPortTextWidth(const u8 *start, const u8 *end)
{
    const s32 letterSpacing =
        GetFontAttribute(FONT_NORMAL, FONTATTR_LETTER_SPACING);
    s32 width = 0;
    bool32 first = TRUE;

    while (start < end)
    {
        if (!first)
            width += letterSpacing;
        width += GetGlyphWidth(*start, FALSE, FONT_NORMAL);
        first = FALSE;
        ++start;
    }

    return width;
}

static bool32 BuildDialoguePage(void)
{
    const u8 *src = sDialogueCursor;
    const s32 letterSpacing =
        GetFontAttribute(FONT_NORMAL, FONTATTR_LETTER_SPACING);
    const s32 spaceWidth =
        GetGlyphWidth(CHAR_SPACE, FALSE, FONT_NORMAL) + letterSpacing;
    u32 out = 0;
    u32 line = 0;
    s32 lineWidth = 0;

    if (src == NULL)
        return FALSE;

    while (*src != EOS && out + 1 < PORT_DIALOGUE_PAGE_BUFFER_SIZE)
    {
        const u8 *wordStart;
        const u8 *wordEnd;
        s32 wordWidth;
        bool32 addSpace;

        // Emerald's prompt control characters are explicit page breaks. The
        // simplified Android dialogue layer handles the button press itself.
        if (*src == CHAR_PROMPT_CLEAR || *src == CHAR_PROMPT_SCROLL)
        {
            ++src;
            break;
        }

        // Preserve authored line breaks. A third line starts a new page,
        // because this window deliberately displays two text rows at a time.
        if (*src == CHAR_NEWLINE)
        {
            ++src;
            if (line + 1 >= PORT_DIALOGUE_LINES_PER_PAGE)
                break;

            sDialoguePage[out++] = CHAR_NEWLINE;
            ++line;
            lineWidth = 0;
            continue;
        }

        // Collapse separator spaces at the start of a line/page. A normal
        // single space is reinserted between words below.
        if (*src == CHAR_SPACE)
        {
            ++src;
            continue;
        }

        wordStart = src;
        while (*src != EOS
            && *src != CHAR_SPACE
            && *src != CHAR_NEWLINE
            && *src != CHAR_PROMPT_CLEAR
            && *src != CHAR_PROMPT_SCROLL)
        {
            ++src;
        }
        wordEnd = src;
        wordWidth = GetPortTextWidth(wordStart, wordEnd);
        addSpace = lineWidth != 0;

        if (addSpace
         && lineWidth + spaceWidth + wordWidth > PORT_DIALOGUE_TEXT_WIDTH)
        {
            if (line + 1 >= PORT_DIALOGUE_LINES_PER_PAGE)
            {
                // Re-read this word on the next page.
                src = wordStart;
                break;
            }

            sDialoguePage[out++] = CHAR_NEWLINE;
            ++line;
            lineWidth = 0;
            addSpace = FALSE;
        }

        if (addSpace && out + 1 < PORT_DIALOGUE_PAGE_BUFFER_SIZE)
        {
            sDialoguePage[out++] = CHAR_SPACE;
            lineWidth += spaceWidth;
        }

        while (wordStart < wordEnd
            && out + 1 < PORT_DIALOGUE_PAGE_BUFFER_SIZE)
        {
            sDialoguePage[out++] = *wordStart++;
        }
        lineWidth += wordWidth;
    }

    sDialoguePage[out] = EOS;
    sDialogueCursor = src;

    while (*sDialogueCursor == CHAR_SPACE)
        ++sDialogueCursor;

    sDialogueHasMore = *sDialogueCursor != EOS;
    return out != 0;
}

static bool32 DrawDialoguePage(void)
{
    if (!BuildDialoguePage())
        return FALSE;

    FillWindowPixelBuffer(
        sDialogueWindowId,
        PIXEL_FILL(TEXT_COLOR_WHITE));
    AddTextPrinterParameterized3(
        sDialogueWindowId,
        FONT_NORMAL,
        8,
        1,
        sTextColors,
        TEXT_SKIP_DRAW,
        sDialoguePage);
    PutWindowTilemap(sDialogueWindowId);
    DrawTextBorderOuter(
        sDialogueWindowId,
        PORT_DIALOGUE_FRAME_TILE,
        PORT_DIALOGUE_FRAME_PAL);
    CopyWindowToVram(sDialogueWindowId, COPYWIN_FULL);
    ShowBg(0);
    ScheduleBgCopyTilemapToVram(0);
    return TRUE;
}

static void CloseDialogue(void)
{
    ClearStdWindowAndFrameToTransparent(sDialogueWindowId, TRUE);
    ScheduleBgCopyTilemapToVram(0);
    HideBg(0);
    sDialogueOpen = FALSE;
    sDialogueCursor = NULL;
    sDialogueHasMore = FALSE;
}

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
    sDialogueCursor = NULL;
    sDialogueHasMore = FALSE;
}

bool32 PortTestDialogue_Open(const u8 *text)
{
    if (sDialogueOpen || sDialogueWindowId == WINDOW_NONE || text == NULL)
        return FALSE;

    sDialogueCursor = text;
    if (!DrawDialoguePage())
    {
        sDialogueCursor = NULL;
        return FALSE;
    }

    sDialogueOpen = TRUE;
    return TRUE;
}

bool32 PortTestDialogue_Update(void)
{
    if (!sDialogueOpen)
        return FALSE;

    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        if (sDialogueHasMore)
        {
            if (!DrawDialoguePage())
                CloseDialogue();
        }
        else
        {
            CloseDialogue();
        }
    }

    return TRUE;
}

bool32 PortTestDialogue_IsOpen(void)
{
    return sDialogueOpen;
}
