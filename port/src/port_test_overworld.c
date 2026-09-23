#include "port_test_overworld.h"
#include "port_test_player.h"
#include "port_test_field_menu.h"
#include "port_test_dialogue.h"
#include "port_test_npc.h"

#include "global.h"
#include "bg.h"
#include "field_camera.h"
#include "fieldmap.h"
#include "gpu_regs.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "option_menu.h"
#include "window.h"
#include "palette.h"
#include "constants/maps.h"


u16 *gOverworldTilemapBuffer_Bg1 = NULL;
u16 *gOverworldTilemapBuffer_Bg2 = NULL;
u16 *gOverworldTilemapBuffer_Bg3 = NULL;

static u16 sPortBg0[BG_SCREEN_SIZE / sizeof(u16)];
static u16 sPortBg1[BG_SCREEN_SIZE / sizeof(u16)];
static u16 sPortBg2[BG_SCREEN_SIZE / sizeof(u16)];
static u16 sPortBg3[BG_SCREEN_SIZE / sizeof(u16)];
static bool32 sPortWarpArrivalLocked;

static const struct BgTemplate sPortOverworldBgTemplates[] =
{
    { .bg = 0, .charBaseIndex = 2, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0 },
    { .bg = 1, .charBaseIndex = 0, .mapBaseIndex = 29, .screenSize = 0, .paletteMode = 0, .priority = 1, .baseTile = 0 },
    { .bg = 2, .charBaseIndex = 0, .mapBaseIndex = 28, .screenSize = 0, .paletteMode = 0, .priority = 2, .baseTile = 0 },
    { .bg = 3, .charBaseIndex = 0, .mapBaseIndex = 30, .screenSize = 0, .paletteMode = 0, .priority = 3, .baseTile = 0 },
};

static void PortTestOverworld_Main(void)
{
    if (!PortTestDialogue_Update())
    {
        if (!PortTestFieldMenu_Update())
            PortTestPlayer_Update();
    }

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

    // SaveBlock position is map-local. MAP_OFFSET is only added when querying
    // Emerald's bordered backup map grid.
    gSaveBlock1Ptr->pos.x = focusX;
    gSaveBlock1Ptr->pos.y = focusY;

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
    // A genuine map transition must never consume a snapshot saved only for
    // returning from a menu scene.
    PortTestNpc_ClearSavedSceneState();
    PortTestOverworld_LoadMapState(mapGroup, mapNum, focusX, focusY);
    PortTestNpc_LoadMap();
}

bool32 PortGame_TryTestWarpAt(s16 x, s16 y)
{
    const struct MapEvents *events = gMapHeader.events;
    if (events == NULL || events->warps == NULL)
        return FALSE;

    if (sPortWarpArrivalLocked)
    {
        bool32 stillOnArrivalWarp = FALSE;
        for (u32 i = 0; i < events->warpCount; ++i)
        {
            if (events->warps[i].x == x && events->warps[i].y == y)
            {
                stillOnArrivalWarp = TRUE;
                break;
            }
        }

        if (stillOnArrivalWarp)
            return FALSE;

        // Once the player has stepped off the destination warp, it can be
        // triggered normally again on a future step.
        sPortWarpArrivalLocked = FALSE;
    }

    for (u32 i = 0; i < events->warpCount; ++i)
    {
        const struct WarpEvent *warp = &events->warps[i];
        if (warp->x != x || warp->y != y)
            continue;

        const struct MapHeader *destHeader =
            Overworld_GetMapHeaderByGroupAndId(warp->mapGroup, warp->mapNum);
        if (destHeader == NULL || destHeader->events == NULL
         || destHeader->events->warps == NULL
         || warp->warpId >= destHeader->events->warpCount)
            return FALSE;

        // Land on the exact destination warp specified by Emerald's map data.
        // A small arrival lock suppresses the reciprocal warp until the player
        // has actually stepped away, avoiding both spawn offsets and loops.
        const struct WarpEvent *dest = &destHeader->events->warps[warp->warpId];
        PortGame_LoadTestMap(
            warp->mapGroup,
            warp->mapNum,
            dest->x,
            dest->y + PORT_PLAYER_MAP_Y_BIAS);
        sPortWarpArrivalLocked = TRUE;
        return TRUE;
    }

    return FALSE;
}


bool32 PortGame_TryTestCoordEventAt(s16 x, s16 y)
{
    const struct MapEvents *events = gMapHeader.events;
    if (events == NULL || events->coordEvents == NULL)
        return FALSE;

    for (u32 i = 0; i < events->coordEventCount; ++i)
    {
        const struct CoordEvent *event = &events->coordEvents[i];
        if (event->x != x || event->y != y)
            continue;
        if (event->script == NULL)
            return FALSE;

        // The Android slice currently stores a displayable dialogue string in
        // the script field. Later scripted-movement support can replace this
        // with full Emerald bytecode execution without changing trigger lookup.
        return PortTestDialogue_Open(event->script);
    }

    return FALSE;
}

static void PortTestOverworld_SetupScene(u16 mapGroup, u16 mapNum, s16 focusX, s16 focusY)
{
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    ResetBgsAndClearDma3BusyFlags(FALSE);
    ResetPaletteFade();

    InitBgsFromTemplates(0, sPortOverworldBgTemplates, ARRAY_COUNT(sPortOverworldBgTemplates));

    // BG0 is the fixed UI/dialogue layer. Reset its scroll explicitly so
    // previous menu state cannot wrap dialogue borders into ghost rectangles.
    ChangeBgX(0, 0, BG_COORD_SET);
    ChangeBgY(0, 0, BG_COORD_SET);
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    SetGpuReg(REG_OFFSET_WIN0H, 0);
    SetGpuReg(REG_OFFSET_WIN0V, 0);
    SetGpuReg(REG_OFFSET_WININ, 0);
    SetGpuReg(REG_OFFSET_WINOUT, 0);
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0);
    SetGpuReg(REG_OFFSET_BLDY, 0);

    memset(sPortBg0, 0, sizeof(sPortBg0));
    memset(sPortBg1, 0, sizeof(sPortBg1));
    memset(sPortBg2, 0, sizeof(sPortBg2));
    memset(sPortBg3, 0, sizeof(sPortBg3));

    gOverworldTilemapBuffer_Bg1 = sPortBg1;
    gOverworldTilemapBuffer_Bg2 = sPortBg2;
    gOverworldTilemapBuffer_Bg3 = sPortBg3;
    SetBgTilemapBuffer(0, sPortBg0);
    SetBgTilemapBuffer(1, gOverworldTilemapBuffer_Bg1);
    SetBgTilemapBuffer(2, gOverworldTilemapBuffer_Bg2);
    SetBgTilemapBuffer(3, gOverworldTilemapBuffer_Bg3);

    PortTestOverworld_LoadMapState(mapGroup, mapNum, focusX, focusY);

    ShowBg(1);
    ShowBg(2);
    ShowBg(3);

    PortTestPlayer_Init();
    PortTestNpc_LoadMap();
    PortTestFieldMenu_Init();
    PortTestDialogue_Init();

    SetMainCallback2(PortTestOverworld_Main);
}

void PortGame_OpenTestOptions(void)
{
    PortTestNpc_SaveSceneState();
    FreeAllWindowBuffers();
    gMain.savedCallback = PortGame_ReturnToTestOverworld;
    SetMainCallback2(CB2_InitOptionMenu);
}

void PortGame_ReturnToTestOverworld(void)
{
    const u16 mapGroup = gSaveBlock1Ptr->location.mapGroup;
    const u16 mapNum = gSaveBlock1Ptr->location.mapNum;
    const s16 focusX = gSaveBlock1Ptr->pos.x;
    const s16 focusY = gSaveBlock1Ptr->pos.y;

    PortTestOverworld_SetupScene(mapGroup, mapNum, focusX, focusY);
}

void PortGame_StartTestOverworld(void)
{
    sPortWarpArrivalLocked = FALSE;
    PortTestNpc_ClearSavedSceneState();
    PortTestOverworld_SetupScene(
        MAP_GROUP(MAP_LITTLEROOT_TOWN),
        MAP_NUM(MAP_LITTLEROOT_TOWN),
        10,
        10);
}
