#include "port_test_overworld.h"
#include "port_test_player.h"

#include "global.h"
#include "bg.h"
#include "field_camera.h"
#include "fieldmap.h"
#include "gpu_regs.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "constants/maps.h"


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
    PortTestPlayer_Update();
    FieldUpdateBgTilemapScroll();
    DoScheduledBgTilemapCopiesToVram();
    TransferPlttBuffer();
}

static void PortTestOverworld_LoadMapState(u16 mapGroup, u16 mapNum, s16 focusX, s16 focusY)
{
    const struct MapHeader *header = Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);
    gMapHeader = *header;

    gSaveBlock1Ptr->location.mapGroup = mapGroup;
    gSaveBlock1Ptr->location.mapNum = mapNum;
    gSaveBlock1Ptr->location.warpId = WARP_ID_NONE;
    gSaveBlock1Ptr->location.x = -1;
    gSaveBlock1Ptr->location.y = -1;

    gSaveBlock1Ptr->pos.x = focusX - MAP_OFFSET;
    gSaveBlock1Ptr->pos.y = focusY - MAP_OFFSET;

    InitMap();
    ResetFieldCamera();
    ResetCameraUpdateInfo();
    CopyMapTilesetsToVram(gMapHeader.mapLayout);
    LoadMapTilesetPalettes(gMapHeader.mapLayout);
    DrawWholeMapView();

    ChangeBgX(1, 0, BG_COORD_SET);
    ChangeBgY(1, 0, BG_COORD_SET);
    ChangeBgX(2, 0, BG_COORD_SET);
    ChangeBgY(2, 0, BG_COORD_SET);
    ChangeBgX(3, 0, BG_COORD_SET);
    ChangeBgY(3, 0, BG_COORD_SET);

    FieldUpdateBgTilemapScroll();
    DoScheduledBgTilemapCopiesToVram();
    TransferPlttBuffer();
}

void PortGame_LoadTestMap(u16 mapGroup, u16 mapNum, s16 focusX, s16 focusY)
{
    PortTestOverworld_LoadMapState(mapGroup, mapNum, focusX, focusY);
}

bool32 PortGame_TryTestWarpAt(s16 x, s16 y)
{
    const u16 group = gSaveBlock1Ptr->location.mapGroup;
    const u16 num = gSaveBlock1Ptr->location.mapNum;

    if (group == MAP_GROUP(MAP_LITTLEROOT_TOWN)
     && num == MAP_NUM(MAP_LITTLEROOT_TOWN))
    {
        if (x == 14 && y == 8)
            PortGame_LoadTestMap(MAP_GROUP(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F), MAP_NUM(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F), 2, 8);
        else if (x == 5 && y == 8)
            PortGame_LoadTestMap(MAP_GROUP(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F), MAP_NUM(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F), 8, 8);
        else if (x == 7 && y == 16)
            PortGame_LoadTestMap(MAP_GROUP(MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB), MAP_NUM(MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB), 6, 12);
        else
            return FALSE;
        return TRUE;
    }

    if (group == MAP_GROUP(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F)
     && num == MAP_NUM(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F)
     && ((x == 8 && y == 8) || (x == 9 && y == 8)))
    {
        PortGame_LoadTestMap(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 5, 8);
        return TRUE;
    }

    if (group == MAP_GROUP(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F)
     && num == MAP_NUM(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F)
     && ((x == 1 && y == 8) || (x == 2 && y == 8)))
    {
        PortGame_LoadTestMap(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 14, 8);
        return TRUE;
    }

    if (group == MAP_GROUP(MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB)
     && num == MAP_NUM(MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB)
     && ((x == 6 && y == 12) || (x == 7 && y == 12)))
    {
        PortGame_LoadTestMap(MAP_GROUP(MAP_LITTLEROOT_TOWN), MAP_NUM(MAP_LITTLEROOT_TOWN), 7, 16);
        return TRUE;
    }

    return FALSE;
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

    PortTestOverworld_LoadMapState(
        MAP_GROUP(MAP_LITTLEROOT_TOWN),
        MAP_NUM(MAP_LITTLEROOT_TOWN),
        10,
        10);

    ShowBg(1);
    ShowBg(2);
    ShowBg(3);

    PortTestPlayer_Init();

    SetMainCallback2(PortTestOverworld_Main);
}
