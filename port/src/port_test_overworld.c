#include "port_test_overworld.h"

#include "global.h"
#include "bg.h"
#include "field_camera.h"
#include "fieldmap.h"
#include "gpu_regs.h"
#include "main.h"
#include "overworld.h"
#include "palette.h"
#include "constants/maps.h"

extern const struct MapHeader gPortLittlerootHeader;

u16 *gOverworldTilemapBuffer_Bg1 = NULL;
u16 *gOverworldTilemapBuffer_Bg2 = NULL;
u16 *gOverworldTilemapBuffer_Bg3 = NULL;

static u16 sPortBg1[BG_SCREEN_SIZE / sizeof(u16)];
static u16 sPortBg2[BG_SCREEN_SIZE / sizeof(u16)];
static u16 sPortBg3[BG_SCREEN_SIZE / sizeof(u16)];

static const struct BgTemplate sPortOverworldBgTemplates[] =
{
    { .bg = 0, .charBaseIndex = 2, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0 },
    { .bg = 1, .charBaseIndex = 0, .mapBaseIndex = 29, .screenSize = 0, .paletteMode = 0, .priority = 1, .baseTile = 0 },
    { .bg = 2, .charBaseIndex = 0, .mapBaseIndex = 28, .screenSize = 0, .paletteMode = 0, .priority = 2, .baseTile = 0 },
    { .bg = 3, .charBaseIndex = 0, .mapBaseIndex = 30, .screenSize = 0, .paletteMode = 0, .priority = 3, .baseTile = 0 },
};

static void PortTestOverworld_Main(void)
{
    FieldUpdateBgTilemapScroll();
    DoScheduledBgTilemapCopiesToVram();
    TransferPlttBuffer();
}

void PortGame_StartTestOverworld(void)
{
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    ResetBgsAndClearDma3BusyFlags(FALSE);
    ResetPaletteFade();

    InitBgsFromTemplates(0, sPortOverworldBgTemplates, ARRAY_COUNT(sPortOverworldBgTemplates));

    memset(sPortBg1, 0, sizeof(sPortBg1));
    memset(sPortBg2, 0, sizeof(sPortBg2));
    memset(sPortBg3, 0, sizeof(sPortBg3));

    gOverworldTilemapBuffer_Bg1 = sPortBg1;
    gOverworldTilemapBuffer_Bg2 = sPortBg2;
    gOverworldTilemapBuffer_Bg3 = sPortBg3;
    SetBgTilemapBuffer(1, gOverworldTilemapBuffer_Bg1);
    SetBgTilemapBuffer(2, gOverworldTilemapBuffer_Bg2);
    SetBgTilemapBuffer(3, gOverworldTilemapBuffer_Bg3);

    gMapHeader = gPortLittlerootHeader;
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_LITTLEROOT_TOWN);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_LITTLEROOT_TOWN);
    gSaveBlock1Ptr->location.warpId = WARP_ID_NONE;
    gSaveBlock1Ptr->location.x = -1;
    gSaveBlock1Ptr->location.y = -1;

    // Camera focus at the middle of the 20x20 Littleroot map.
    gSaveBlock1Ptr->pos.x = 10;
    gSaveBlock1Ptr->pos.y = 10;

    InitMap();
    ResetFieldCamera();
    CopyMapTilesetsToVram(gMapHeader.mapLayout);
    LoadMapTilesetPalettes(gMapHeader.mapLayout);
    DrawWholeMapView();

    ChangeBgX(1, 0, BG_COORD_SET);
    ChangeBgY(1, 0, BG_COORD_SET);
    ChangeBgX(2, 0, BG_COORD_SET);
    ChangeBgY(2, 0, BG_COORD_SET);
    ChangeBgX(3, 0, BG_COORD_SET);
    ChangeBgY(3, 0, BG_COORD_SET);

    ShowBg(1);
    ShowBg(2);
    ShowBg(3);

    FieldUpdateBgTilemapScroll();
    DoScheduledBgTilemapCopiesToVram();
    TransferPlttBuffer();

    SetMainCallback2(PortTestOverworld_Main);
}
