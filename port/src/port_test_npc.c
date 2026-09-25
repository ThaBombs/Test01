#include "port_test_npc.h"
#include "port_test_dialogue.h"
#include "port_test_field_menu.h"
#include "port_test_overworld.h"

#include "global.h"
#include "field_camera.h"
#include "fieldmap.h"
#include "main.h"
#include "sprite.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/metatile_behaviors.h"

#define PORT_NPC_MAX 8
#define PORT_NPC_PAL_TAG_1 0x7F11
#define PORT_NPC_PAL_TAG_2 0x7F12
#define PORT_NPC_PAL_TAG_3 0x7F13
#define PORT_NPC_PAL_TAG_4 0x7F14

static const u8 sPortTextBirchAfterBattle[] =
    _("PROF. BIRCH: Thanks again!\nCome by my POKéMON LAB.");

enum
{
    PORT_NPC_FACE_SOUTH,
    PORT_NPC_FACE_NORTH,
    PORT_NPC_FACE_WEST,
    PORT_NPC_FACE_EAST,
};

static const u32 sPortTwinGfx[] =
    INCGFX_U32("graphics/object_events/pics/people/twin.png", ".4bpp", "-mwidth 2 -mheight 4");
static const u32 sPortFatManGfx[] =
    INCGFX_U32("graphics/object_events/pics/people/fat_man.png", ".4bpp", "-mwidth 2 -mheight 4");
static const u32 sPortBoy2Gfx[] =
    INCGFX_U32("graphics/object_events/pics/people/boy_2.png", ".4bpp", "-mwidth 2 -mheight 4");
static const u32 sPortScientist1Gfx[] =
    INCGFX_U32("graphics/object_events/pics/people/scientist_1.png", ".4bpp", "-mwidth 2 -mheight 4");
static const u32 sPortYoungsterGfx[] =
    INCGFX_U32("graphics/object_events/pics/people/youngster.png", ".4bpp", "-mwidth 2 -mheight 4");
static const u32 sPortProfBirchGfx[] =
    INCGFX_U32("graphics/object_events/pics/people/prof_birch.png", ".4bpp", "-mwidth 2 -mheight 4");
static const u32 sPortBirchsBagGfx[] =
    INCGFX_U32("graphics/object_events/pics/misc/birchs_bag.png", ".4bpp", "-mwidth 2 -mheight 2");
static const u32 sPortEnemyZigzagoonGfx[] =
    INCGFX_U32("graphics/object_events/pics/pokemon_old/enemy_zigzagoon.png", ".4bpp", "-mwidth 4 -mheight 4");

static const u16 sPortNpc1Palette[] =
    INCGFX_U16("graphics/object_events/palettes/npc_1.pal", ".gbapal");
static const u16 sPortNpc2Palette[] =
    INCGFX_U16("graphics/object_events/palettes/npc_2.pal", ".gbapal");
static const u16 sPortNpc3Palette[] =
    INCGFX_U16("graphics/object_events/palettes/npc_3.pal", ".gbapal");
static const u16 sPortEnemyZigzagoonPalette[] =
    INCGFX_U16("graphics/object_events/palettes/enemy_zigzagoon.pal", ".gbapal");

static const struct SpriteFrameImage sPortTwinFrames[] =
{
    overworld_ascending_frames(sPortTwinGfx, 2, 4),
};
static const struct SpriteFrameImage sPortFatManFrames[] =
{
    overworld_ascending_frames(sPortFatManGfx, 2, 4),
};
static const struct SpriteFrameImage sPortBoy2Frames[] =
{
    overworld_ascending_frames(sPortBoy2Gfx, 2, 4),
};
static const struct SpriteFrameImage sPortScientist1Frames[] =
{
    overworld_ascending_frames(sPortScientist1Gfx, 2, 4),
};
static const struct SpriteFrameImage sPortYoungsterFrames[] =
{
    overworld_ascending_frames(sPortYoungsterGfx, 2, 4),
};
static const struct SpriteFrameImage sPortProfBirchFrames[] =
{
    overworld_ascending_frames(sPortProfBirchGfx, 2, 4),
};
static const struct SpriteFrameImage sPortBirchsBagFrames[] =
{
    overworld_frame(sPortBirchsBagGfx, 2, 2, 0),
};
static const struct SpriteFrameImage sPortEnemyZigzagoonFrames[] =
{
    overworld_ascending_frames(sPortEnemyZigzagoonGfx, 4, 4),
};

static const struct OamData sPortNpcOam =
{
    .shape = SPRITE_SHAPE(16x32),
    .size = SPRITE_SIZE(16x32),
    .priority = 2,
};
static const struct OamData sPortNpcSmallOam =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 2,
};
static const struct OamData sPortNpcLargeOam =
{
    .shape = SPRITE_SHAPE(32x32),
    .size = SPRITE_SIZE(32x32),
    .priority = 2,
};

static const union AnimCmd sPortNpcFaceSouth[] =
{
    ANIMCMD_FRAME(0, 16),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sPortNpcFaceNorth[] =
{
    ANIMCMD_FRAME(1, 16),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sPortNpcFaceWest[] =
{
    ANIMCMD_FRAME(2, 16),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sPortNpcFaceEast[] =
{
    ANIMCMD_FRAME(2, 16, .hFlip = TRUE),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sPortNpcWalkSouth[] =
{
    ANIMCMD_FRAME(3, 4),
    ANIMCMD_FRAME(0, 4),
    ANIMCMD_FRAME(4, 4),
    ANIMCMD_FRAME(0, 4),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sPortNpcWalkNorth[] =
{
    ANIMCMD_FRAME(5, 4),
    ANIMCMD_FRAME(1, 4),
    ANIMCMD_FRAME(6, 4),
    ANIMCMD_FRAME(1, 4),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sPortNpcWalkWest[] =
{
    ANIMCMD_FRAME(7, 4),
    ANIMCMD_FRAME(2, 4),
    ANIMCMD_FRAME(8, 4),
    ANIMCMD_FRAME(2, 4),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sPortNpcWalkEast[] =
{
    ANIMCMD_FRAME(7, 4, .hFlip = TRUE),
    ANIMCMD_FRAME(2, 4, .hFlip = TRUE),
    ANIMCMD_FRAME(8, 4, .hFlip = TRUE),
    ANIMCMD_FRAME(2, 4, .hFlip = TRUE),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const sPortNpcAnims[] =
{
    [PORT_NPC_FACE_SOUTH] = sPortNpcFaceSouth,
    [PORT_NPC_FACE_NORTH] = sPortNpcFaceNorth,
    [PORT_NPC_FACE_WEST] = sPortNpcFaceWest,
    [PORT_NPC_FACE_EAST] = sPortNpcFaceEast,
    [4 + PORT_NPC_FACE_SOUTH] = sPortNpcWalkSouth,
    [4 + PORT_NPC_FACE_NORTH] = sPortNpcWalkNorth,
    [4 + PORT_NPC_FACE_WEST] = sPortNpcWalkWest,
    [4 + PORT_NPC_FACE_EAST] = sPortNpcWalkEast,
};

static const union AnimCmd sPortNpcStaticAnim[] =
{
    ANIMCMD_FRAME(0, 16),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd *const sPortNpcStaticAnims[] =
{
    sPortNpcStaticAnim,
};


static const struct SpritePalette sPortNpcPalette1 =
{
    .data = sPortNpc1Palette,
    .tag = PORT_NPC_PAL_TAG_1,
};
static const struct SpritePalette sPortNpcPalette2 =
{
    .data = sPortNpc2Palette,
    .tag = PORT_NPC_PAL_TAG_2,
};
static const struct SpritePalette sPortNpcPalette3 =
{
    .data = sPortNpc3Palette,
    .tag = PORT_NPC_PAL_TAG_3,
};
static const struct SpritePalette sPortNpcPalette4 =
{
    .data = sPortEnemyZigzagoonPalette,
    .tag = PORT_NPC_PAL_TAG_4,
};

static const struct SpriteTemplate sPortTwinTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_NPC_PAL_TAG_2,
    .oam = &sPortNpcOam,
    .anims = sPortNpcAnims,
    .images = sPortTwinFrames,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};
static const struct SpriteTemplate sPortFatManTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_NPC_PAL_TAG_1,
    .oam = &sPortNpcOam,
    .anims = sPortNpcAnims,
    .images = sPortFatManFrames,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};
static const struct SpriteTemplate sPortBoy2Template =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_NPC_PAL_TAG_1,
    .oam = &sPortNpcOam,
    .anims = sPortNpcAnims,
    .images = sPortBoy2Frames,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};
static const struct SpriteTemplate sPortScientist1Template =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_NPC_PAL_TAG_3,
    .oam = &sPortNpcOam,
    .anims = sPortNpcAnims,
    .images = sPortScientist1Frames,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};
static const struct SpriteTemplate sPortYoungsterTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_NPC_PAL_TAG_1,
    .oam = &sPortNpcOam,
    .anims = sPortNpcAnims,
    .images = sPortYoungsterFrames,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};
static const struct SpriteTemplate sPortProfBirchTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_NPC_PAL_TAG_3,
    .oam = &sPortNpcOam,
    .anims = sPortNpcAnims,
    .images = sPortProfBirchFrames,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};
static const struct SpriteTemplate sPortBirchsBagTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_NPC_PAL_TAG_2,
    .oam = &sPortNpcSmallOam,
    .anims = sPortNpcStaticAnims,
    .images = sPortBirchsBagFrames,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};
static const struct SpriteTemplate sPortEnemyZigzagoonTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_NPC_PAL_TAG_4,
    .oam = &sPortNpcLargeOam,
    .anims = sPortNpcAnims,
    .images = sPortEnemyZigzagoonFrames,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};


struct PortNpcRuntime
{
    const struct ObjectEventTemplate *event;
    u8 spriteId;
    s16 originX;
    s16 originY;
    s16 x;
    s16 y;
    s16 targetX;
    s16 targetY;
    u16 idleFrames;
    u8 stepFrames;
    s8 stepDx;
    s8 stepDy;
    u8 facing;
};

struct PortNpcSavedState
{
    s16 originX;
    s16 originY;
    s16 x;
    s16 y;
    s16 targetX;
    s16 targetY;
    u16 idleFrames;
    u8 stepFrames;
    s8 stepDx;
    s8 stepDy;
    u8 facing;
};

static struct PortNpcRuntime sPortNpcs[PORT_NPC_MAX];
static u8 sPortNpcCount;
static u32 sPortNpcRng = 0x51F15EEDu;

static struct PortNpcSavedState sPortNpcSaved[PORT_NPC_MAX];
static u8 sPortNpcSavedCount;
static u16 sPortNpcSavedMapGroup;
static u16 sPortNpcSavedMapNum;
static u32 sPortNpcSavedRng;
static bool32 sPortNpcRestorePending;

static u32 PortNpcNextRandom(void)
{
    sPortNpcRng = sPortNpcRng * 1664525u + 1013904223u;
    return sPortNpcRng;
}

static const struct SpriteTemplate *GetPortNpcSpriteTemplate(u16 graphicsId)
{
    switch (graphicsId)
    {
    case OBJ_EVENT_GFX_TWIN:
        return &sPortTwinTemplate;
    case OBJ_EVENT_GFX_FAT_MAN:
        return &sPortFatManTemplate;
    case OBJ_EVENT_GFX_BOY_2:
        return &sPortBoy2Template;
    case OBJ_EVENT_GFX_SCIENTIST_1:
        return &sPortScientist1Template;
    case OBJ_EVENT_GFX_YOUNGSTER:
        return &sPortYoungsterTemplate;
    case OBJ_EVENT_GFX_PROF_BIRCH:
        return &sPortProfBirchTemplate;
    case OBJ_EVENT_GFX_BIRCHS_BAG:
        return &sPortBirchsBagTemplate;
    case OBJ_EVENT_GFX_ZIGZAGOON_1:
        return &sPortEnemyZigzagoonTemplate;
    default:
        return NULL;
    }
}

static bool32 PortNpcIsInanimateGraphics(u16 graphicsId)
{
    return graphicsId == OBJ_EVENT_GFX_BIRCHS_BAG;
}

static s16 PortNpcVisualYOffset(u16 graphicsId)
{
    // 16x16 inanimate objects are centered within their map tile. The regular
    // 16x32 people sprites are anchored at the tile's upper edge instead.
    return graphicsId == OBJ_EVENT_GFX_BIRCHS_BAG ? 8 : 0;
}

static void UpdateNpcVisibility(struct PortNpcRuntime *npc)
{
    if (npc->spriteId >= MAX_SPRITES)
        return;

    struct Sprite *sprite = &gSprites[npc->spriteId];
    sprite->invisible =
        sprite->x < -16 || sprite->x > DISPLAY_WIDTH + 16
        || sprite->y < -32 || sprite->y > DISPLAY_HEIGHT + 32;
}

static void PositionNpcFromMap(struct PortNpcRuntime *npc)
{
    if (npc->spriteId >= MAX_SPRITES || npc->event == NULL)
        return;

    const s16 playerX = gSaveBlock1Ptr->pos.x;
    const s16 playerY = gSaveBlock1Ptr->pos.y - PORT_PLAYER_MAP_Y_BIAS;

    struct Sprite *sprite = &gSprites[npc->spriteId];
    sprite->x = DISPLAY_WIDTH / 2
        + (npc->x - playerX) * 16;
    sprite->y = DISPLAY_HEIGHT / 2
        + (npc->y - playerY) * 16
        + PortNpcVisualYOffset(npc->event->graphicsId);

    // Preserve exact sub-tile progress when returning from a scene rebuild
    // such as the Options menu.
    if (npc->stepFrames != 0)
    {
        const u8 completedFrames = 8 - npc->stepFrames;
        sprite->x += npc->stepDx * 2 * completedFrames;
        sprite->y += npc->stepDy * 2 * completedFrames;
    }

    UpdateNpcVisibility(npc);
}

static void DestroyPortNpcs(void)
{
    for (u32 i = 0; i < sPortNpcCount; ++i)
    {
        if (sPortNpcs[i].spriteId < MAX_SPRITES)
            DestroySprite(&gSprites[sPortNpcs[i].spriteId]);
    }

    memset(sPortNpcs, 0, sizeof(sPortNpcs));
    sPortNpcCount = 0;
}

void PortTestNpc_LoadMap(void)
{
    const bool32 restoreSavedState =
        sPortNpcRestorePending
        && sPortNpcSavedMapGroup == gSaveBlock1Ptr->location.mapGroup
        && sPortNpcSavedMapNum == gSaveBlock1Ptr->location.mapNum;

    DestroyPortNpcs();

    const struct MapEvents *events = gMapHeader.events;
    if (events == NULL || events->objectEvents == NULL)
        return;

    LoadSpritePalette(&sPortNpcPalette1);
    LoadSpritePalette(&sPortNpcPalette2);
    LoadSpritePalette(&sPortNpcPalette3);
    LoadSpritePalette(&sPortNpcPalette4);

    for (u32 i = 0; i < events->objectEventCount && sPortNpcCount < PORT_NPC_MAX; ++i)
    {
        const struct ObjectEventTemplate *event = &events->objectEvents[i];

        if (PortGame_IsFirstBattleComplete()
         && (event->graphicsId == OBJ_EVENT_GFX_BIRCHS_BAG
          || event->graphicsId == OBJ_EVENT_GFX_ZIGZAGOON_1))
            continue;

        const struct SpriteTemplate *template =
            GetPortNpcSpriteTemplate(event->graphicsId);
        if (template == NULL)
            continue;

        const u8 spriteId = CreateSprite(
            template,
            DISPLAY_WIDTH / 2,
            DISPLAY_HEIGHT / 2,
            1);
        if (spriteId >= MAX_SPRITES)
            continue;

        struct PortNpcRuntime *npc = &sPortNpcs[sPortNpcCount];
        npc->event = event;
        npc->spriteId = spriteId;
        npc->originX = event->x;
        npc->originY = event->y;
        npc->x = event->x;
        npc->y = event->y;
        npc->targetX = event->x;
        npc->targetY = event->y;
        npc->idleFrames = 24 + sPortNpcCount * 17;
        npc->stepFrames = 0;
        npc->stepDx = 0;
        npc->stepDy = 0;
        npc->facing = PORT_NPC_FACE_SOUTH;

        if (restoreSavedState && sPortNpcCount < sPortNpcSavedCount)
        {
            const struct PortNpcSavedState *saved =
                &sPortNpcSaved[sPortNpcCount];
            npc->originX = saved->originX;
            npc->originY = saved->originY;
            npc->x = saved->x;
            npc->y = saved->y;
            npc->targetX = saved->targetX;
            npc->targetY = saved->targetY;
            npc->idleFrames = saved->idleFrames;
            npc->stepFrames = saved->stepFrames;
            npc->stepDx = saved->stepDx;
            npc->stepDy = saved->stepDy;
            npc->facing = saved->facing;
        }

        if (PortNpcIsInanimateGraphics(event->graphicsId))
            StartSpriteAnimIfDifferent(&gSprites[spriteId], 0);
        else
            StartSpriteAnimIfDifferent(
                &gSprites[spriteId],
                npc->stepFrames != 0 ? 4 + npc->facing : npc->facing);
        ++sPortNpcCount;
    }

    if (restoreSavedState)
        sPortNpcRng = sPortNpcSavedRng;

    sPortNpcRestorePending = FALSE;
    sPortNpcSavedCount = 0;

    PortTestNpc_Update();
}

void PortTestNpc_Update(void)
{
    for (u32 i = 0; i < sPortNpcCount; ++i)
        PositionNpcFromMap(&sPortNpcs[i]);
}

void PortTestNpc_ApplyCameraDelta(s16 dx, s16 dy)
{
    if (dx == 0 && dy == 0)
        return;

    for (u32 i = 0; i < sPortNpcCount; ++i)
    {
        struct PortNpcRuntime *npc = &sPortNpcs[i];
        if (npc->spriteId >= MAX_SPRITES)
            continue;

        // BG scroll and world sprites move in opposite screen directions.
        // Applying the exact per-frame camera delta keeps NPCs pinned to map
        // pixels instead of recomputing them from the player's logical tile.
        gSprites[npc->spriteId].x -= dx;
        gSprites[npc->spriteId].y -= dy;
        UpdateNpcVisibility(npc);
    }
}


static bool32 PortNpcMetatileBlocksNorth(u8 behavior)
{
    return behavior == MB_IMPASSABLE_NORTH
        || behavior == MB_IMPASSABLE_NORTHEAST
        || behavior == MB_IMPASSABLE_NORTHWEST
        || behavior == MB_IMPASSABLE_SOUTH_AND_NORTH;
}

static bool32 PortNpcMetatileBlocksSouth(u8 behavior)
{
    return behavior == MB_IMPASSABLE_SOUTH
        || behavior == MB_IMPASSABLE_SOUTHEAST
        || behavior == MB_IMPASSABLE_SOUTHWEST
        || behavior == MB_IMPASSABLE_SOUTH_AND_NORTH;
}

static bool32 PortNpcMetatileBlocksWest(u8 behavior)
{
    return behavior == MB_IMPASSABLE_WEST
        || behavior == MB_IMPASSABLE_NORTHWEST
        || behavior == MB_IMPASSABLE_SOUTHWEST
        || behavior == MB_IMPASSABLE_WEST_AND_EAST
        || behavior == MB_SECRET_BASE_BREAKABLE_DOOR;
}

static bool32 PortNpcMetatileBlocksEast(u8 behavior)
{
    return behavior == MB_IMPASSABLE_EAST
        || behavior == MB_IMPASSABLE_NORTHEAST
        || behavior == MB_IMPASSABLE_SOUTHEAST
        || behavior == MB_IMPASSABLE_WEST_AND_EAST
        || behavior == MB_SECRET_BASE_BREAKABLE_DOOR;
}

static bool32 PortNpcDirectionBlocked(s16 x, s16 y, s16 targetX, s16 targetY, s16 dx, s16 dy)
{
    const u8 currentBehavior =
        MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET);
    const u8 targetBehavior =
        MapGridGetMetatileBehaviorAt(targetX + MAP_OFFSET, targetY + MAP_OFFSET);

    if (dy > 0)
        return PortNpcMetatileBlocksSouth(currentBehavior)
            || PortNpcMetatileBlocksNorth(targetBehavior);
    if (dy < 0)
        return PortNpcMetatileBlocksNorth(currentBehavior)
            || PortNpcMetatileBlocksSouth(targetBehavior);
    if (dx < 0)
        return PortNpcMetatileBlocksWest(currentBehavior)
            || PortNpcMetatileBlocksEast(targetBehavior);
    if (dx > 0)
        return PortNpcMetatileBlocksEast(currentBehavior)
            || PortNpcMetatileBlocksWest(targetBehavior);

    return FALSE;
}

static bool32 PortNpcIsWarpTile(s16 x, s16 y)
{
    const struct MapEvents *events = gMapHeader.events;
    if (events == NULL || events->warps == NULL)
        return FALSE;

    for (u32 i = 0; i < events->warpCount; ++i)
    {
        if (events->warps[i].x == x && events->warps[i].y == y)
            return TRUE;
    }

    return FALSE;
}

static bool32 PortNpcTileReservedByOther(u32 npcIndex, s16 x, s16 y)
{
    for (u32 i = 0; i < sPortNpcCount; ++i)
    {
        if (i == npcIndex)
            continue;

        const struct PortNpcRuntime *other = &sPortNpcs[i];
        if ((other->x == x && other->y == y)
         || (other->stepFrames != 0
          && other->targetX == x
          && other->targetY == y))
            return TRUE;
    }

    return FALSE;
}

static bool32 PortNpcCanStartStep(
    u32 npcIndex,
    s16 dx,
    s16 dy,
    s16 playerX,
    s16 playerY)
{
    const struct PortNpcRuntime *npc = &sPortNpcs[npcIndex];
    const s16 targetX = npc->x + dx;
    const s16 targetY = npc->y + dy;
    const s16 rangeX = npc->event->movementRangeX;
    const s16 rangeY = npc->event->movementRangeY;

    if (targetX < npc->originX - rangeX || targetX > npc->originX + rangeX)
        return FALSE;
    if (targetY < npc->originY - rangeY || targetY > npc->originY + rangeY)
        return FALSE;

    if (targetX == playerX && targetY == playerY)
        return FALSE;
    if (PortNpcTileReservedByOther(npcIndex, targetX, targetY))
        return FALSE;
    if (PortNpcIsWarpTile(targetX, targetY))
        return FALSE;

    if (MapGridGetCollisionAt(targetX + MAP_OFFSET, targetY + MAP_OFFSET) != 0)
        return FALSE;

    if (PortNpcDirectionBlocked(npc->x, npc->y, targetX, targetY, dx, dy))
        return FALSE;

    return TRUE;
}

static bool32 PortNpcSupportsWandering(const struct PortNpcRuntime *npc)
{
    return npc->event != NULL
        && (npc->event->movementType == MOVEMENT_TYPE_WANDER_AROUND
         || npc->event->movementType == MOVEMENT_TYPE_WANDER_AROUND_SLOWER);
}

static void PortNpcTryBeginStep(u32 npcIndex, s16 playerX, s16 playerY)
{
    static const s8 sDx[4] = {0, 0, -1, 1};
    static const s8 sDy[4] = {1, -1, 0, 0};

    struct PortNpcRuntime *npc = &sPortNpcs[npcIndex];
    const u32 start = PortNpcNextRandom() & 3u;

    for (u32 attempt = 0; attempt < 4; ++attempt)
    {
        const u32 direction = (start + attempt) & 3u;
        const s16 dx = sDx[direction];
        const s16 dy = sDy[direction];

        if (!PortNpcCanStartStep(npcIndex, dx, dy, playerX, playerY))
            continue;

        npc->stepDx = dx;
        npc->stepDy = dy;
        npc->targetX = npc->x + dx;
        npc->targetY = npc->y + dy;
        npc->stepFrames = 8;
        npc->facing = (u8)direction;

        if (npc->spriteId < MAX_SPRITES)
            StartSpriteAnimIfDifferent(
                &gSprites[npc->spriteId],
                4 + npc->facing);
        return;
    }

    npc->idleFrames = 18 + (PortNpcNextRandom() & 31u);
}


void PortTestNpc_SaveSceneState(void)
{
    sPortNpcSavedMapGroup = gSaveBlock1Ptr->location.mapGroup;
    sPortNpcSavedMapNum = gSaveBlock1Ptr->location.mapNum;
    sPortNpcSavedRng = sPortNpcRng;
    sPortNpcSavedCount = sPortNpcCount;

    for (u32 i = 0; i < sPortNpcCount; ++i)
    {
        const struct PortNpcRuntime *npc = &sPortNpcs[i];
        struct PortNpcSavedState *saved = &sPortNpcSaved[i];

        saved->originX = npc->originX;
        saved->originY = npc->originY;
        saved->x = npc->x;
        saved->y = npc->y;
        saved->targetX = npc->targetX;
        saved->targetY = npc->targetY;
        saved->idleFrames = npc->idleFrames;
        saved->stepFrames = npc->stepFrames;
        saved->stepDx = npc->stepDx;
        saved->stepDy = npc->stepDy;
        saved->facing = npc->facing;
    }

    sPortNpcRestorePending = TRUE;
}

void PortTestNpc_ClearSavedSceneState(void)
{
    sPortNpcRestorePending = FALSE;
    sPortNpcSavedCount = 0;
}

void PortTestNpc_UpdateMovement(s16 playerX, s16 playerY)
{
    for (u32 i = 0; i < sPortNpcCount; ++i)
    {
        struct PortNpcRuntime *npc = &sPortNpcs[i];

        if (npc->spriteId >= MAX_SPRITES || npc->event == NULL)
            continue;

        if (npc->stepFrames != 0)
        {
            gSprites[npc->spriteId].x += npc->stepDx * 2;
            gSprites[npc->spriteId].y += npc->stepDy * 2;
            --npc->stepFrames;

            if (npc->stepFrames == 0)
            {
                npc->x = npc->targetX;
                npc->y = npc->targetY;
                npc->stepDx = 0;
                npc->stepDy = 0;
                npc->idleFrames = 28 + (PortNpcNextRandom() & 63u);
                StartSpriteAnimIfDifferent(
                    &gSprites[npc->spriteId],
                    npc->facing);
            }

            UpdateNpcVisibility(npc);
            continue;
        }

        if (!PortNpcSupportsWandering(npc))
            continue;

        if (npc->idleFrames != 0)
        {
            --npc->idleFrames;
            continue;
        }

        PortNpcTryBeginStep(i, playerX, playerY);
    }
}

bool32 PortTestNpc_BlocksTile(s16 x, s16 y)
{
    for (u32 i = 0; i < sPortNpcCount; ++i)
    {
        const struct PortNpcRuntime *npc = &sPortNpcs[i];
        if ((npc->x == x && npc->y == y)
         || (npc->stepFrames != 0
          && npc->targetX == x
          && npc->targetY == y))
            return TRUE;
    }

    return FALSE;
}

bool32 PortTestNpc_TryInteractAt(s16 x, s16 y, s16 playerX, s16 playerY)
{
    for (u32 i = 0; i < sPortNpcCount; ++i)
    {
        struct PortNpcRuntime *npc = &sPortNpcs[i];
        const struct ObjectEventTemplate *event = npc->event;
        if (event == NULL
         || npc->stepFrames != 0
         || npc->x != x
         || npc->y != y)
            continue;

        if (event->graphicsId == OBJ_EVENT_GFX_BIRCHS_BAG)
            return PortTestFieldMenu_OpenStarterChoice();

        if (event->graphicsId == OBJ_EVENT_GFX_PROF_BIRCH
         && PortGame_IsFirstBattleComplete())
            return PortTestDialogue_Open(sPortTextBirchAfterBattle);

        if (!PortNpcIsInanimateGraphics(event->graphicsId))
        {
            u8 faceAnim = PORT_NPC_FACE_SOUTH;
            if (playerY < npc->y)
                faceAnim = PORT_NPC_FACE_NORTH;
            else if (playerY > npc->y)
                faceAnim = PORT_NPC_FACE_SOUTH;
            else if (playerX < npc->x)
                faceAnim = PORT_NPC_FACE_WEST;
            else if (playerX > npc->x)
                faceAnim = PORT_NPC_FACE_EAST;

            npc->facing = faceAnim;
            npc->idleFrames = 60;
            if (npc->spriteId < MAX_SPRITES)
                StartSpriteAnimIfDifferent(&gSprites[npc->spriteId], faceAnim);
        }

        if (event->script != NULL)
            return PortTestDialogue_Open(event->script);

        return TRUE;
    }

    return FALSE;
}
