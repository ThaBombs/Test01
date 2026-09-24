#include "port_test_overworld.h"
#include "port_test_player.h"
#include "port_test_field_menu.h"
#include "port_test_dialogue.h"
#include "port_test_npc.h"
#include "port_test_battle.h"

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
#include "constants/metatile_behaviors.h"


u16 *gOverworldTilemapBuffer_Bg1 = NULL;
u16 *gOverworldTilemapBuffer_Bg2 = NULL;
u16 *gOverworldTilemapBuffer_Bg3 = NULL;

static u16 sPortBg0[BG_SCREEN_SIZE / sizeof(u16)];
static u16 sPortBg1[BG_SCREEN_SIZE / sizeof(u16)];
static u16 sPortBg2[BG_SCREEN_SIZE / sizeof(u16)];
static u16 sPortBg3[BG_SCREEN_SIZE / sizeof(u16)];
static bool32 sPortWarpArrivalLocked;
static u16 sPortEventVars[8];
static u8 sPortChosenStarter = PORT_STARTER_NONE;
static bool32 sPortFirstBattleComplete;

static const u8 sPortText_FirstBattleWon[] =
    _("PROF. BIRCH: Whew...\nYou saved me. Thanks a lot!\p"
      "Come by my POKéMON LAB later, okay?");
static const u8 sPortText_FirstBattleLost[] =
    _("PROF. BIRCH: Try again!\nUse the POKéMON in my BAG!");

static const struct BgTemplate sPortOverworldBgTemplates[] =
{
    { .bg = 0, .charBaseIndex = 2, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0 },
    { .bg = 1, .charBaseIndex = 0, .mapBaseIndex = 29, .screenSize = 0, .paletteMode = 0, .priority = 1, .baseTile = 0 },
    { .bg = 2, .charBaseIndex = 0, .mapBaseIndex = 28, .screenSize = 0, .paletteMode = 0, .priority = 2, .baseTile = 0 },
    { .bg = 3, .charBaseIndex = 0, .mapBaseIndex = 30, .screenSize = 0, .paletteMode = 0, .priority = 3, .baseTile = 0 },
};

static void PortTestOverworld_Main(void)
{
    const bool32 dialogueActive = PortTestDialogue_Update();
    if (!dialogueActive)
    {
        if (PortTestBattle_TryStartPending())
            return;
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

void PortGame_LoadTestConnectionMap(u16 mapGroup, u16 mapNum)
{
    const struct MapHeader *header =
        Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);
    if (header == NULL)
        return;

    // CameraMove has already translated gSaveBlock1Ptr->pos into the
    // destination map's coordinate space. Preserve that position and only
    // replace the map data/tiles needed for a seamless border crossing.
    PortTestNpc_ClearSavedSceneState();
    gMapHeader = *header;
    gSaveBlock1Ptr->location.mapGroup = mapGroup;
    gSaveBlock1Ptr->location.mapNum = mapNum;
    gSaveBlock1Ptr->location.warpId = WARP_ID_NONE;
    gSaveBlock1Ptr->location.x = -1;
    gSaveBlock1Ptr->location.y = -1;

    InitMap();
    CopyMapTilesetsToVram(gMapHeader.mapLayout);
    LoadMapTilesetPalettes(gMapHeader.mapLayout);
    PortTestNpc_LoadMap();
}

static bool32 IsHouseStairTransition(
    u16 srcGroup, u16 srcNum,
    u16 dstGroup, u16 dstNum)
{
    const bool32 brendanPair =
        ((srcGroup == MAP_GROUP(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F)
       && srcNum == MAP_NUM(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F)
       && dstGroup == MAP_GROUP(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F)
       && dstNum == MAP_NUM(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F))
      || (srcGroup == MAP_GROUP(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F)
       && srcNum == MAP_NUM(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F)
       && dstGroup == MAP_GROUP(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F)
       && dstNum == MAP_NUM(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F)));

    const bool32 mayPair =
        ((srcGroup == MAP_GROUP(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F)
       && srcNum == MAP_NUM(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F)
       && dstGroup == MAP_GROUP(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_2F)
       && dstNum == MAP_NUM(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_2F))
      || (srcGroup == MAP_GROUP(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_2F)
       && srcNum == MAP_NUM(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_2F)
       && dstGroup == MAP_GROUP(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F)
       && dstNum == MAP_NUM(MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F)));

    return brendanPair || mayPair;
}

static u8 ResolveWarpExitDirection(const struct MapHeader *header, const struct WarpEvent *dest)
{
    if (header == NULL || header->mapLayout == NULL || dest == NULL)
        return DIR_NONE;

    const s16 x = dest->x;
    const s16 y = dest->y;
    const s16 width = header->mapLayout->width;
    const s16 height = header->mapLayout->height;

    // Directional warp tiles carry the preferred exit direction themselves.
    // For diagonal stair warps, Emerald's own exit animation faces away from
    // the right/left stair edge respectively.
    const u8 behavior =
        MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET);
    switch (behavior)
    {
    case MB_EAST_ARROW_WARP:
        return DIR_EAST;
    case MB_WEST_ARROW_WARP:
        return DIR_WEST;
    case MB_NORTH_ARROW_WARP:
        return DIR_NORTH;
    case MB_SOUTH_ARROW_WARP:
    case MB_WATER_SOUTH_ARROW_WARP:
        return DIR_SOUTH;
    case MB_UP_RIGHT_STAIR_WARP:
    case MB_DOWN_RIGHT_STAIR_WARP:
        return DIR_WEST;
    case MB_UP_LEFT_STAIR_WARP:
    case MB_DOWN_LEFT_STAIR_WARP:
        return DIR_EAST;
    default:
        break;
    }

    // Older Emerald interiors often encode ordinary stairs as a normal/non-
    // animated warp at an edge rather than a directional stair behavior.
    // Resolve those geometrically: move away from the nearest map edge. This
    // gives the current bedroom stairs SOUTH, but also naturally handles
    // equivalent stairs placed on the other three sides of future maps.
    const s16 top = y;
    const s16 bottom = height - 1 - y;
    const s16 left = x;
    const s16 right = width - 1 - x;

    s16 nearest = top;
    u8 direction = DIR_SOUTH;
    if (bottom < nearest)
    {
        nearest = bottom;
        direction = DIR_NORTH;
    }
    if (left < nearest)
    {
        nearest = left;
        direction = DIR_EAST;
    }
    if (right < nearest)
        direction = DIR_WEST;

    return direction;
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

        const u16 sourceGroup = gSaveBlock1Ptr->location.mapGroup;
        const u16 sourceNum = gSaveBlock1Ptr->location.mapNum;
        const struct MapHeader *destHeader =
            Overworld_GetMapHeaderByGroupAndId(warp->mapGroup, warp->mapNum);
        if (destHeader == NULL || destHeader->events == NULL
         || destHeader->events->warps == NULL
         || warp->warpId >= destHeader->events->warpCount)
            return FALSE;

        const struct WarpEvent *dest = &destHeader->events->warps[warp->warpId];
        const bool32 isHouseStair =
            IsHouseStairTransition(
                sourceGroup, sourceNum,
                warp->mapGroup, warp->mapNum);

        // Load on the actual destination warp tile first. Stair exits then use
        // a real movement step away from that tile, so the player visibly
        // emerges instead of teleporting directly to the already-offset spot.
        PortGame_LoadTestMap(
            warp->mapGroup,
            warp->mapNum,
            dest->x,
            dest->y + PORT_PLAYER_MAP_Y_BIAS);

        sPortWarpArrivalLocked = TRUE;

        if (isHouseStair)
        {
            const u8 exitDirection =
                ResolveWarpExitDirection(&gMapHeader, dest);
            if (exitDirection != DIR_NONE)
                PortTestPlayer_BeginWarpExitStep(exitDirection);
        }

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

        // trigger/index are reused by the reduced Android slice as a compact
        // state-variable gate. trigger 0 remains unconditional, preserving the
        // original Littleroot warning events.
        if (event->trigger != 0)
        {
            if (event->trigger >= ARRAY_COUNT(sPortEventVars)
             || sPortEventVars[event->trigger] != event->index)
                continue;
        }

        // The Android slice currently stores a displayable dialogue string in
        // the script field. Advancing a gated variable after the message opens
        // gives us deterministic one-shot/sequential story triggers until the
        // full Emerald script engine and VarSet/VarGet state are linked.
        if (PortTestDialogue_Open(event->script))
        {
            if (event->trigger != 0)
                ++sPortEventVars[event->trigger];
            return TRUE;
        }
        return FALSE;
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

void PortGame_SetChosenStarter(u8 starter)
{
    if (starter <= PORT_STARTER_MUDKIP)
        sPortChosenStarter = starter;
}

u8 PortGame_GetChosenStarter(void)
{
    return sPortChosenStarter;
}

bool32 PortGame_IsFirstBattleComplete(void)
{
    return sPortFirstBattleComplete;
}

void PortGame_ReturnFromFirstBattle(bool32 won)
{
    const u16 mapGroup = gSaveBlock1Ptr->location.mapGroup;
    const u16 mapNum = gSaveBlock1Ptr->location.mapNum;
    const s16 focusX = gSaveBlock1Ptr->pos.x;
    const s16 focusY = gSaveBlock1Ptr->pos.y;

    if (won)
        sPortFirstBattleComplete = TRUE;

    PortTestNpc_ClearSavedSceneState();
    PortTestOverworld_SetupScene(
        mapGroup,
        mapNum,
        focusX,
        focusY);

    PortTestDialogue_Open(
        won
            ? sPortText_FirstBattleWon
            : sPortText_FirstBattleLost);
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
    sPortChosenStarter = PORT_STARTER_NONE;
    sPortFirstBattleComplete = FALSE;
    memset(sPortEventVars, 0, sizeof(sPortEventVars));
    PortTestNpc_ClearSavedSceneState();
    PortTestOverworld_SetupScene(
        MAP_GROUP(MAP_LITTLEROOT_TOWN),
        MAP_NUM(MAP_LITTLEROOT_TOWN),
        10,
        10);
}
