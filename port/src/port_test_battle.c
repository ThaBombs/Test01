#include "port_test_battle.h"
#include "port_test_overworld.h"

#include "global.h"
#include "bg.h"
#include "gpu_regs.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "sprite.h"
#include "string_util.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/characters.h"
#include "constants/rgb.h"

#define PORT_BATTLE_FRAME_TILE 0x1E0
#define PORT_BATTLE_FRAME_PAL  14

#define PORT_BATTLE_PAL_TREECKO 0x7F31
#define PORT_BATTLE_PAL_TORCHIC 0x7F32
#define PORT_BATTLE_PAL_MUDKIP  0x7F33
#define PORT_BATTLE_PAL_ZIGZAGOON 0x7F34

#define PORT_BATTLE_PLAYER_MAX_HP 20
#define PORT_BATTLE_ENEMY_MAX_HP  16

enum
{
    PORT_BATTLE_WIN_ENEMY,
    PORT_BATTLE_WIN_PLAYER,
    PORT_BATTLE_WIN_COMMAND,
};

enum
{
    PORT_BATTLE_COMMAND_FIGHT,
    PORT_BATTLE_COMMAND_BAG,
    PORT_BATTLE_COMMAND_POKEMON,
    PORT_BATTLE_COMMAND_RUN,
    PORT_BATTLE_COMMAND_COUNT,
};

enum
{
    PORT_BATTLE_STATE_COMMAND,
    PORT_BATTLE_STATE_MOVES,
    PORT_BATTLE_STATE_PLAYER_MOVE_MESSAGE,
    PORT_BATTLE_STATE_PLAYER_ATTACK_ANIM,
    PORT_BATTLE_STATE_PLAYER_STATUS_MESSAGE,
    PORT_BATTLE_STATE_ENEMY_MOVE_MESSAGE,
    PORT_BATTLE_STATE_ENEMY_ATTACK_ANIM,
    PORT_BATTLE_STATE_BLOCKED_MESSAGE,
    PORT_BATTLE_STATE_WIN_FAINT_MESSAGE,
    PORT_BATTLE_STATE_WIN_MESSAGE,
    PORT_BATTLE_STATE_LOSS_MESSAGE,
};

static const u32 sTreeckoBackGfx[] =
    INCGFX_U32("graphics/pokemon/treecko/back_gba.png", ".4bpp");
static const u32 sTorchicBackGfx[] =
    INCGFX_U32("graphics/pokemon/torchic/back_gba.png", ".4bpp");
static const u32 sMudkipBackGfx[] =
    INCGFX_U32("graphics/pokemon/mudkip/back_gba.png", ".4bpp");
static const u32 sZigzagoonFrontGfx[] =
    INCGFX_U32("graphics/pokemon/zigzagoon/anim_front_gba.png", ".4bpp");

static const u16 sTreeckoPalette[] =
    INCGFX_U16("graphics/pokemon/treecko/normal_gba.pal", ".gbapal");
static const u16 sTorchicPalette[] =
    INCGFX_U16("graphics/pokemon/torchic/normal_gba.pal", ".gbapal");
static const u16 sMudkipPalette[] =
    INCGFX_U16("graphics/pokemon/mudkip/normal_gba.pal", ".gbapal");
static const u16 sZigzagoonPalette[] =
    INCGFX_U16("graphics/pokemon/zigzagoon/normal_gba.pal", ".gbapal");

static const struct SpriteFrameImage sTreeckoBackFrames[] =
{
    {.data = sTreeckoBackGfx, .size = 0x800},
};
static const struct SpriteFrameImage sTorchicBackFrames[] =
{
    {.data = sTorchicBackGfx, .size = 0x800},
};
static const struct SpriteFrameImage sMudkipBackFrames[] =
{
    {.data = sMudkipBackGfx, .size = 0x800},
};
static const struct SpriteFrameImage sZigzagoonFrontFrames[] =
{
    // anim_front_gba contains multiple animation frames. The first 64x64 frame
    // is the normal battle pose used by this reduced Android battle slice.
    {.data = sZigzagoonFrontGfx, .size = 0x800},
};

static const struct OamData sBattleMonOam =
{
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
    .priority = 1,
};

static const union AnimCmd sBattleMonAnim[] =
{
    ANIMCMD_FRAME(0, 16),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd *const sBattleMonAnims[] =
{
    sBattleMonAnim,
};

static const struct SpritePalette sBattlePalettes[] =
{
    {.data = sTreeckoPalette, .tag = PORT_BATTLE_PAL_TREECKO},
    {.data = sTorchicPalette, .tag = PORT_BATTLE_PAL_TORCHIC},
    {.data = sMudkipPalette, .tag = PORT_BATTLE_PAL_MUDKIP},
    {.data = sZigzagoonPalette, .tag = PORT_BATTLE_PAL_ZIGZAGOON},
};

static const struct SpriteTemplate sTreeckoBackTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_BATTLE_PAL_TREECKO,
    .oam = &sBattleMonOam,
    .anims = sBattleMonAnims,
    .images = sTreeckoBackFrames,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};
static const struct SpriteTemplate sTorchicBackTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_BATTLE_PAL_TORCHIC,
    .oam = &sBattleMonOam,
    .anims = sBattleMonAnims,
    .images = sTorchicBackFrames,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};
static const struct SpriteTemplate sMudkipBackTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_BATTLE_PAL_MUDKIP,
    .oam = &sBattleMonOam,
    .anims = sBattleMonAnims,
    .images = sMudkipBackFrames,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};
static const struct SpriteTemplate sZigzagoonFrontTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_BATTLE_PAL_ZIGZAGOON,
    .oam = &sBattleMonOam,
    .anims = sBattleMonAnims,
    .images = sZigzagoonFrontFrames,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct BgTemplate sBattleBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0,
    },
};

static u16 sBattleBg0[BG_SCREEN_SIZE / sizeof(u16)];

static const struct WindowTemplate sBattleWindows[] =
{
    [PORT_BATTLE_WIN_ENEMY] = {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 1,
        .width = 13,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 1,
    },
    [PORT_BATTLE_WIN_PLAYER] = {
        .bg = 0,
        .tilemapLeft = 16,
        .tilemapTop = 8,
        .width = 13,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 0x40,
    },
    [PORT_BATTLE_WIN_COMMAND] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 14,
        .width = 30,
        .height = 6,
        .paletteNum = 15,
        .baseBlock = 0x80,
    },
    DUMMY_WIN_TEMPLATE
};

static const u16 sBattleUiPalette[16] =
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

static const u16 sBattleBackdropPalette[16] =
{
    RGB(23, 29, 23),
};

static const u8 sTextColors[] =
{
    TEXT_COLOR_WHITE,
    TEXT_COLOR_DARK_GRAY,
    TEXT_COLOR_WHITE,
};

static const u8 sTextCursor[] = _("▶");
static const u8 sTextFight[] = _("FIGHT");
static const u8 sTextBag[] = _("BAG");
static const u8 sTextPokemon[] = _("POKéMON");
static const u8 sTextRun[] = _("RUN");
static const u8 *const sCommandNames[] =
{
    sTextFight,
    sTextBag,
    sTextPokemon,
    sTextRun,
};

static const u8 sTextTreecko[] = _("TREECKO");
static const u8 sTextTorchic[] = _("TORCHIC");
static const u8 sTextMudkip[] = _("MUDKIP");
static const u8 sTextZigzagoon[] = _("ZIGZAGOON");

static const u8 sTextPound[] = _("POUND");
static const u8 sTextLeer[] = _("LEER");
static const u8 sTextScratch[] = _("SCRATCH");
static const u8 sTextGrowl[] = _("GROWL");
static const u8 sTextTackle[] = _("TACKLE");

static const u8 sTextWildPrefix[] = _("Wild ");
static const u8 sTextLv2[] = _(" Lv2");
static const u8 sTextLv5[] = _(" Lv5");
static const u8 sTextHp[] = _("HP ");
static const u8 sTextSlash[] = _("/");
static const u8 sTextQuestionSuffix[] = _(" do?");
static const u8 sTextUsed[] = _(" used ");
static const u8 sTextBang[] = _("!");
static const u8 sTextEnemyTackle[] = _("Wild ZIGZAGOON used TACKLE!");
static const u8 sTextEnemyFainted[] = _("Wild ZIGZAGOON fainted!");
static const u8 sTextBattleWon[] = _("You won the battle!");
static const u8 sTextBattleLost[] = _("Your POKéMON fainted!");
static const u8 sTextFirstBattleWon[] =
    _("PROF. BIRCH: Whew...\nYou saved me. Thanks a lot!\p"
      "Come by my POKéMON LAB later, okay?");
static const u8 sTextFirstBattleLost[] =
    _("PROF. BIRCH: Try again!\nUse the POKéMON in my BAG!");
static const u8 sTextBlocked[] = _("That option isn't available\nin this first battle.");
static const u8 sTextDefenseFell[] = _("Wild ZIGZAGOON's DEFENSE fell!");
static const u8 sTextAttackFell[] = _("Wild ZIGZAGOON's ATTACK fell!");

static bool32 sBattlePending;
static u8 sBattleState;
static u8 sCommandSelection;
static u8 sMoveSelection;
static u8 sAnimTimer;
static s16 sPlayerHp;
static s16 sEnemyHp;
static bool32 sEnemyDefenseLowered;
static bool32 sEnemyAttackLowered;
static u8 sPlayerSpriteId = MAX_SPRITES;
static u8 sEnemySpriteId = MAX_SPRITES;

static const u8 *GetStarterName(void)
{
    switch (PortGame_GetChosenStarter())
    {
    case PORT_STARTER_TORCHIC:
        return sTextTorchic;
    case PORT_STARTER_MUDKIP:
        return sTextMudkip;
    case PORT_STARTER_TREECKO:
    default:
        return sTextTreecko;
    }
}

static const struct SpriteTemplate *GetStarterTemplate(void)
{
    switch (PortGame_GetChosenStarter())
    {
    case PORT_STARTER_TORCHIC:
        return &sTorchicBackTemplate;
    case PORT_STARTER_MUDKIP:
        return &sMudkipBackTemplate;
    case PORT_STARTER_TREECKO:
    default:
        return &sTreeckoBackTemplate;
    }
}

static const u8 *GetStarterMoveName(u8 slot)
{
    switch (PortGame_GetChosenStarter())
    {
    case PORT_STARTER_TORCHIC:
        return slot == 0 ? sTextScratch : sTextGrowl;
    case PORT_STARTER_MUDKIP:
        return slot == 0 ? sTextTackle : sTextGrowl;
    case PORT_STARTER_TREECKO:
    default:
        return slot == 0 ? sTextPound : sTextLeer;
    }
}

static bool32 StarterSecondMoveLowersDefense(void)
{
    return PortGame_GetChosenStarter() == PORT_STARTER_TREECKO;
}

static void PrintText(u8 windowId, u8 x, u8 y, const u8 *text)
{
    AddTextPrinterParameterized3(
        windowId,
        FONT_NORMAL,
        x,
        y,
        sTextColors,
        TEXT_SKIP_DRAW,
        text);
}

static void DrawHpLine(u8 windowId, s16 hp, s16 maxHp)
{
    u8 buffer[32];
    u8 *dst = StringCopy(buffer, sTextHp);
    dst = ConvertIntToDecimalStringN(
        dst,
        hp,
        STR_CONV_MODE_LEFT_ALIGN,
        2);
    dst = StringCopy(dst, sTextSlash);
    ConvertIntToDecimalStringN(
        dst,
        maxHp,
        STR_CONV_MODE_LEFT_ALIGN,
        2);
    PrintText(windowId, 8, 17, buffer);
}

static void DrawBattleStatus(void)
{
    u8 buffer[64];
    u8 *dst;

    FillWindowPixelBuffer(
        PORT_BATTLE_WIN_ENEMY,
        PIXEL_FILL(TEXT_COLOR_WHITE));
    dst = StringCopy(buffer, sTextWildPrefix);
    dst = StringCopy(dst, sTextZigzagoon);
    StringCopy(dst, sTextLv2);
    PrintText(PORT_BATTLE_WIN_ENEMY, 8, 1, buffer);
    DrawHpLine(
        PORT_BATTLE_WIN_ENEMY,
        sEnemyHp,
        PORT_BATTLE_ENEMY_MAX_HP);
    PutWindowTilemap(PORT_BATTLE_WIN_ENEMY);
    DrawTextBorderOuter(
        PORT_BATTLE_WIN_ENEMY,
        PORT_BATTLE_FRAME_TILE,
        PORT_BATTLE_FRAME_PAL);
    CopyWindowToVram(PORT_BATTLE_WIN_ENEMY, COPYWIN_FULL);

    FillWindowPixelBuffer(
        PORT_BATTLE_WIN_PLAYER,
        PIXEL_FILL(TEXT_COLOR_WHITE));
    dst = StringCopy(buffer, GetStarterName());
    StringCopy(dst, sTextLv5);
    PrintText(PORT_BATTLE_WIN_PLAYER, 8, 1, buffer);
    DrawHpLine(
        PORT_BATTLE_WIN_PLAYER,
        sPlayerHp,
        PORT_BATTLE_PLAYER_MAX_HP);
    PutWindowTilemap(PORT_BATTLE_WIN_PLAYER);
    DrawTextBorderOuter(
        PORT_BATTLE_WIN_PLAYER,
        PORT_BATTLE_FRAME_TILE,
        PORT_BATTLE_FRAME_PAL);
    CopyWindowToVram(PORT_BATTLE_WIN_PLAYER, COPYWIN_FULL);
}

static void DrawCommandWindow(void)
{
    u8 prompt[64];
    u8 *dst = StringCopy(prompt, GetStarterName());
    StringCopy(dst, sTextQuestionSuffix);

    FillWindowPixelBuffer(
        PORT_BATTLE_WIN_COMMAND,
        PIXEL_FILL(TEXT_COLOR_WHITE));
    PrintText(PORT_BATTLE_WIN_COMMAND, 8, 1, prompt);

    for (u32 i = 0; i < PORT_BATTLE_COMMAND_COUNT; ++i)
    {
        const u8 col = i & 1;
        const u8 row = i >> 1;
        PrintText(
            PORT_BATTLE_WIN_COMMAND,
            24 + col * 104,
            17 + row * 16,
            sCommandNames[i]);
    }

    PrintText(
        PORT_BATTLE_WIN_COMMAND,
        8 + (sCommandSelection & 1) * 104,
        17 + (sCommandSelection >> 1) * 16,
        sTextCursor);

    PutWindowTilemap(PORT_BATTLE_WIN_COMMAND);
    DrawTextBorderOuter(
        PORT_BATTLE_WIN_COMMAND,
        PORT_BATTLE_FRAME_TILE,
        PORT_BATTLE_FRAME_PAL);
    CopyWindowToVram(PORT_BATTLE_WIN_COMMAND, COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(0);
}

static void DrawMoveWindow(void)
{
    FillWindowPixelBuffer(
        PORT_BATTLE_WIN_COMMAND,
        PIXEL_FILL(TEXT_COLOR_WHITE));

    PrintText(
        PORT_BATTLE_WIN_COMMAND,
        24,
        9,
        GetStarterMoveName(0));
    PrintText(
        PORT_BATTLE_WIN_COMMAND,
        24,
        29,
        GetStarterMoveName(1));
    PrintText(
        PORT_BATTLE_WIN_COMMAND,
        8,
        9 + sMoveSelection * 20,
        sTextCursor);

    PutWindowTilemap(PORT_BATTLE_WIN_COMMAND);
    DrawTextBorderOuter(
        PORT_BATTLE_WIN_COMMAND,
        PORT_BATTLE_FRAME_TILE,
        PORT_BATTLE_FRAME_PAL);
    CopyWindowToVram(PORT_BATTLE_WIN_COMMAND, COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(0);
}

static void DrawMessage(const u8 *text)
{
    FillWindowPixelBuffer(
        PORT_BATTLE_WIN_COMMAND,
        PIXEL_FILL(TEXT_COLOR_WHITE));
    PrintText(PORT_BATTLE_WIN_COMMAND, 8, 9, text);
    PutWindowTilemap(PORT_BATTLE_WIN_COMMAND);
    DrawTextBorderOuter(
        PORT_BATTLE_WIN_COMMAND,
        PORT_BATTLE_FRAME_TILE,
        PORT_BATTLE_FRAME_PAL);
    CopyWindowToVram(PORT_BATTLE_WIN_COMMAND, COPYWIN_FULL);
    ScheduleBgCopyTilemapToVram(0);
}

static void DrawPlayerMoveMessage(void)
{
    u8 buffer[80];
    u8 *dst = StringCopy(buffer, GetStarterName());
    dst = StringCopy(dst, sTextUsed);
    dst = StringCopy(dst, GetStarterMoveName(sMoveSelection));
    StringCopy(dst, sTextBang);
    DrawMessage(buffer);
}

static void ReturnToCommand(void)
{
    sBattleState = PORT_BATTLE_STATE_COMMAND;
    sCommandSelection = PORT_BATTLE_COMMAND_FIGHT;
    DrawCommandWindow();
}

static void BeginEnemyTurn(void)
{
    DrawMessage(sTextEnemyTackle);
    sBattleState = PORT_BATTLE_STATE_ENEMY_MOVE_MESSAGE;
}

static void ApplyPlayerMove(void)
{
    if (sMoveSelection == 0)
    {
        s16 damage = 6;
        if (sEnemyDefenseLowered)
            damage += 2;

        sEnemyHp -= damage;
        if (sEnemyHp < 0)
            sEnemyHp = 0;
        DrawBattleStatus();

        if (sEnemyHp == 0)
        {
            if (sEnemySpriteId < MAX_SPRITES)
                gSprites[sEnemySpriteId].invisible = TRUE;
            DrawMessage(sTextEnemyFainted);
            sBattleState = PORT_BATTLE_STATE_WIN_FAINT_MESSAGE;
        }
        else
        {
            BeginEnemyTurn();
        }
        return;
    }

    if (StarterSecondMoveLowersDefense())
    {
        sEnemyDefenseLowered = TRUE;
        DrawMessage(sTextDefenseFell);
    }
    else
    {
        sEnemyAttackLowered = TRUE;
        DrawMessage(sTextAttackFell);
    }
    sBattleState = PORT_BATTLE_STATE_PLAYER_STATUS_MESSAGE;
}

static void ApplyEnemyMove(void)
{
    const s16 damage = sEnemyAttackLowered ? 2 : 4;
    sPlayerHp -= damage;
    if (sPlayerHp < 0)
        sPlayerHp = 0;
    DrawBattleStatus();

    if (sPlayerHp == 0)
    {
        if (sPlayerSpriteId < MAX_SPRITES)
            gSprites[sPlayerSpriteId].invisible = TRUE;
        DrawMessage(sTextBattleLost);
        sBattleState = PORT_BATTLE_STATE_LOSS_MESSAGE;
    }
    else
    {
        ReturnToCommand();
    }
}

static void UpdateAttackAnimation(bool32 playerAttacking)
{
    struct Sprite *sprite;
    if (playerAttacking)
    {
        if (sPlayerSpriteId >= MAX_SPRITES)
            return;
        sprite = &gSprites[sPlayerSpriteId];
    }
    else
    {
        if (sEnemySpriteId >= MAX_SPRITES)
            return;
        sprite = &gSprites[sEnemySpriteId];
    }

    ++sAnimTimer;
    if (sAnimTimer <= 4)
        sprite->x2 = playerAttacking ? sAnimTimer * 3 : -(s16)sAnimTimer * 3;
    else if (sAnimTimer < 8)
        sprite->x2 = playerAttacking
            ? (8 - sAnimTimer) * 3
            : -(s16)(8 - sAnimTimer) * 3;
    else
    {
        sprite->x2 = 0;
        sAnimTimer = 0;
        if (playerAttacking)
            ApplyPlayerMove();
        else
            ApplyEnemyMove();
    }
}

static void HandleCommandInput(void)
{
    const u8 oldSelection = sCommandSelection;

    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
        sCommandSelection ^= 1;
    if (JOY_NEW(DPAD_UP | DPAD_DOWN))
        sCommandSelection ^= 2;

    if (oldSelection != sCommandSelection)
    {
        DrawCommandWindow();
        return;
    }

    if (!JOY_NEW(A_BUTTON))
        return;

    if (sCommandSelection == PORT_BATTLE_COMMAND_FIGHT)
    {
        sMoveSelection = 0;
        sBattleState = PORT_BATTLE_STATE_MOVES;
        DrawMoveWindow();
    }
    else
    {
        DrawMessage(sTextBlocked);
        sBattleState = PORT_BATTLE_STATE_BLOCKED_MESSAGE;
    }
}

static void HandleMoveInput(void)
{
    if (JOY_NEW(B_BUTTON))
    {
        ReturnToCommand();
        return;
    }

    if (JOY_NEW(DPAD_UP | DPAD_DOWN))
    {
        sMoveSelection ^= 1;
        DrawMoveWindow();
        return;
    }

    if (JOY_NEW(A_BUTTON))
    {
        DrawPlayerMoveMessage();
        sBattleState = PORT_BATTLE_STATE_PLAYER_MOVE_MESSAGE;
    }
}

static void PortTestBattle_Main(void)
{
    switch (sBattleState)
    {
    case PORT_BATTLE_STATE_COMMAND:
        HandleCommandInput();
        break;
    case PORT_BATTLE_STATE_MOVES:
        HandleMoveInput();
        break;
    case PORT_BATTLE_STATE_PLAYER_MOVE_MESSAGE:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            if (sMoveSelection == 0)
            {
                sAnimTimer = 0;
                sBattleState = PORT_BATTLE_STATE_PLAYER_ATTACK_ANIM;
            }
            else
            {
                ApplyPlayerMove();
            }
        }
        break;
    case PORT_BATTLE_STATE_PLAYER_ATTACK_ANIM:
        UpdateAttackAnimation(TRUE);
        break;
    case PORT_BATTLE_STATE_PLAYER_STATUS_MESSAGE:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
            BeginEnemyTurn();
        break;
    case PORT_BATTLE_STATE_ENEMY_MOVE_MESSAGE:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            sAnimTimer = 0;
            sBattleState = PORT_BATTLE_STATE_ENEMY_ATTACK_ANIM;
        }
        break;
    case PORT_BATTLE_STATE_ENEMY_ATTACK_ANIM:
        UpdateAttackAnimation(FALSE);
        break;
    case PORT_BATTLE_STATE_BLOCKED_MESSAGE:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
            ReturnToCommand();
        break;
    case PORT_BATTLE_STATE_WIN_FAINT_MESSAGE:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            DrawMessage(sTextBattleWon);
            sBattleState = PORT_BATTLE_STATE_WIN_MESSAGE;
        }
        break;
    case PORT_BATTLE_STATE_WIN_MESSAGE:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            PortGame_ReturnFromFirstBattle(TRUE);
            return;
        }
        break;
    case PORT_BATTLE_STATE_LOSS_MESSAGE:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            PortGame_ReturnFromFirstBattle(FALSE);
            return;
        }
        break;
    }

    AnimateSprites();
    BuildOamBuffer();
    LoadOam();
    ProcessSpriteCopyRequests();
    DoScheduledBgTilemapCopiesToVram();
    TransferPlttBuffer();
}

static void StartFirstBattle(void)
{
    const struct SpriteTemplate *starterTemplate = GetStarterTemplate();

    sBattlePending = FALSE;
    sBattleState = PORT_BATTLE_STATE_COMMAND;
    sCommandSelection = PORT_BATTLE_COMMAND_FIGHT;
    sMoveSelection = 0;
    sAnimTimer = 0;
    sPlayerHp = PORT_BATTLE_PLAYER_MAX_HP;
    sEnemyHp = PORT_BATTLE_ENEMY_MAX_HP;
    sEnemyDefenseLowered = FALSE;
    sEnemyAttackLowered = FALSE;

    FreeAllWindowBuffers();
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    ResetBgsAndClearDma3BusyFlags(FALSE);
    ResetPaletteFade();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ClearSpriteCopyRequests();

    InitBgsFromTemplates(
        0,
        sBattleBgTemplates,
        ARRAY_COUNT(sBattleBgTemplates));
    memset(sBattleBg0, 0, sizeof(sBattleBg0));
    SetBgTilemapBuffer(0, sBattleBg0);

    InitWindows(sBattleWindows);
    DeactivateAllTextPrinters();

    LoadPalette(
        sBattleBackdropPalette,
        BG_PLTT_ID(0),
        PLTT_SIZE_4BPP);
    LoadPalette(
        sBattleUiPalette,
        BG_PLTT_ID(15),
        sizeof(sBattleUiPalette));
    LoadUserWindowBorderGfx(
        PORT_BATTLE_WIN_COMMAND,
        PORT_BATTLE_FRAME_TILE,
        BG_PLTT_ID(PORT_BATTLE_FRAME_PAL));

    for (u32 i = 0; i < ARRAY_COUNT(sBattlePalettes); ++i)
        LoadSpritePalette(&sBattlePalettes[i]);

    sPlayerSpriteId = CreateSprite(
        starterTemplate,
        56,
        99,
        0);
    sEnemySpriteId = CreateSprite(
        &sZigzagoonFrontTemplate,
        182,
        51,
        0);

    ShowBg(0);
    SetGpuRegBits(
        REG_OFFSET_DISPCNT,
        DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);

    DrawBattleStatus();
    DrawCommandWindow();

    AnimateSprites();
    BuildOamBuffer();
    LoadOam();
    ProcessSpriteCopyRequests();
    ScheduleBgCopyTilemapToVram(0);
    DoScheduledBgTilemapCopiesToVram();
    TransferPlttBuffer();

    SetMainCallback2(PortTestBattle_Main);
}

void PortTestBattle_RequestFirstBattle(void)
{
    if (PortGame_GetChosenStarter() <= PORT_STARTER_MUDKIP)
        sBattlePending = TRUE;
}

bool32 PortTestBattle_TryStartPending(void)
{
    if (!sBattlePending)
        return FALSE;

    StartFirstBattle();
    return TRUE;
}

bool32 PortTestBattle_IsPending(void)
{
    return sBattlePending;
}

const u8 *PortTestBattle_GetResultText(bool32 won)
{
    return won ? sTextFirstBattleWon : sTextFirstBattleLost;
}
