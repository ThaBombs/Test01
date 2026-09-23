#include "port_test_player.h"
#include "port_test_overworld.h"
#include "port_test_dialogue.h"
#include "port_test_npc.h"

#include "global.h"
#include "field_camera.h"
#include "fieldmap.h"
#include "gpu_regs.h"
#include "main.h"
#include "sprite.h"
#include "constants/event_bg.h"
#include "constants/maps.h"
#include "constants/map_types.h"
#include "constants/metatile_behaviors.h"

#define PORT_PLAYER_PAL_TAG 0x7F01
#define PORT_GRASS_PAL_TAG  0x7F02

enum
{
    PORT_ANIM_FACE_SOUTH,
    PORT_ANIM_FACE_NORTH,
    PORT_ANIM_FACE_WEST,
    PORT_ANIM_FACE_EAST,
    PORT_ANIM_WALK_SOUTH,
    PORT_ANIM_WALK_NORTH,
    PORT_ANIM_WALK_WEST,
    PORT_ANIM_WALK_EAST,
};

static const u32 sPortBrendanWalking[] =
    INCGFX_U32("graphics/object_events/pics/people/brendan/walking.png", ".4bpp", "-mwidth 2 -mheight 4");
static const u16 sPortBrendanPalette[] =
    INCGFX_U16("graphics/object_events/palettes/brendan.pal", ".gbapal");
static const u32 sPortTallGrassGfx[] =
    INCGFX_U32("graphics/field_effects/pics/tall_grass.png", ".4bpp", "-mwidth 2 -mheight 2");
static const u16 sPortGrassPalette[] =
    INCGFX_U16("graphics/field_effects/palettes/general_1.pal", ".gbapal");

static const struct SpriteFrameImage sPortBrendanFrames[] =
{
    overworld_ascending_frames(sPortBrendanWalking, 2, 4),
};
static const struct SpriteFrameImage sPortTallGrassFrames[] =
{
    overworld_ascending_frames(sPortTallGrassGfx, 2, 2),
};

static const struct OamData sPortBrendanOam =
{
    .shape = SPRITE_SHAPE(16x32),
    .size = SPRITE_SIZE(16x32),
    .priority = 2,
};
static const struct OamData sPortGrassOam =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 1,
};

static const union AnimCmd sFaceSouth[] =
{
    ANIMCMD_FRAME(0, 16),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sFaceNorth[] =
{
    ANIMCMD_FRAME(1, 16),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sFaceWest[] =
{
    ANIMCMD_FRAME(2, 16),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sFaceEast[] =
{
    ANIMCMD_FRAME(2, 16, .hFlip = TRUE),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sWalkSouth[] =
{
    ANIMCMD_FRAME(3, 4),
    ANIMCMD_FRAME(0, 4),
    ANIMCMD_FRAME(4, 4),
    ANIMCMD_FRAME(0, 4),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sWalkNorth[] =
{
    ANIMCMD_FRAME(5, 4),
    ANIMCMD_FRAME(1, 4),
    ANIMCMD_FRAME(6, 4),
    ANIMCMD_FRAME(1, 4),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sWalkWest[] =
{
    ANIMCMD_FRAME(7, 4),
    ANIMCMD_FRAME(2, 4),
    ANIMCMD_FRAME(8, 4),
    ANIMCMD_FRAME(2, 4),
    ANIMCMD_JUMP(0),
};
static const union AnimCmd sWalkEast[] =
{
    ANIMCMD_FRAME(7, 4, .hFlip = TRUE),
    ANIMCMD_FRAME(2, 4, .hFlip = TRUE),
    ANIMCMD_FRAME(8, 4, .hFlip = TRUE),
    ANIMCMD_FRAME(2, 4, .hFlip = TRUE),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const sPortBrendanAnims[] =
{
    sFaceSouth,
    sFaceNorth,
    sFaceWest,
    sFaceEast,
    sWalkSouth,
    sWalkNorth,
    sWalkWest,
    sWalkEast,
};

static const union AnimCmd sPortTallGrassAnim[] =
{
    ANIMCMD_FRAME(1, 6),
    ANIMCMD_FRAME(2, 6),
    ANIMCMD_FRAME(3, 6),
    ANIMCMD_FRAME(4, 6),
    ANIMCMD_FRAME(0, 6),
    ANIMCMD_END,
};
static const union AnimCmd *const sPortTallGrassAnims[] =
{
    sPortTallGrassAnim,
};

static const struct SpritePalette sPortBrendanSpritePalette =
{
    .data = sPortBrendanPalette,
    .tag = PORT_PLAYER_PAL_TAG,
};
static const struct SpritePalette sPortGrassSpritePalette =
{
    .data = sPortGrassPalette,
    .tag = PORT_GRASS_PAL_TAG,
};

static void PortGrassSpriteCallback(struct Sprite *sprite)
{
    // Position is advanced by the inverse camera delta each movement frame.
    // Do not derive it from the player's logical tile here: that made every
    // still-running grass effect snap along with the player at tile boundaries.
    if (sprite->animEnded)
        DestroySprite(sprite);
}

static void ApplyGrassCameraDelta(s16 dx, s16 dy)
{
    if (dx == 0 && dy == 0)
        return;

    for (u32 i = 0; i < MAX_SPRITES; ++i)
    {
        struct Sprite *sprite = &gSprites[i];
        if (!sprite->inUse || sprite->callback != PortGrassSpriteCallback)
            continue;

        sprite->x -= dx;
        sprite->y -= dy;
    }
}

static const struct SpriteTemplate sPortBrendanTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_PLAYER_PAL_TAG,
    .oam = &sPortBrendanOam,
    .anims = sPortBrendanAnims,
    .images = sPortBrendanFrames,
    .affineAnims = NULL,
    .callback = NULL,
};
static const struct SpriteTemplate sPortTallGrassTemplate =
{
    .tileTag = TAG_NONE,
    .paletteTag = PORT_GRASS_PAL_TAG,
    .oam = &sPortGrassOam,
    .anims = sPortTallGrassAnims,
    .images = sPortTallGrassFrames,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = PortGrassSpriteCallback,
};

static u32 sPortPlayerSpriteId = MAX_SPRITES;
static u8 sPortFacing = PORT_ANIM_FACE_SOUTH;
static u8 sPortStepFrames;
static s8 sPortStepDx;
static s8 sPortStepDy;
static bool32 sPortLedgeJump;
static bool32 sPortFacingLockUntilRelease;

static bool32 IsTallGrassBehavior(u8 behavior)
{
    return behavior == MB_TALL_GRASS
        || behavior == MB_CYCLING_ROAD_PULL_DOWN_GRASS;
}

static void SpawnTallGrassStepEffect(void)
{
    if (sPortPlayerSpriteId >= MAX_SPRITES)
        return;

    const s16 mapX = gSaveBlock1Ptr->pos.x;
    const s16 mapY = gSaveBlock1Ptr->pos.y - PORT_PLAYER_MAP_Y_BIAS;
    const u8 behavior =
        MapGridGetMetatileBehaviorAt(mapX + MAP_OFFSET, mapY + MAP_OFFSET);

    if (!IsTallGrassBehavior(behavior))
        return;

    const u8 spriteId = CreateSprite(
        &sPortTallGrassTemplate,
        DISPLAY_WIDTH / 2,
        DISPLAY_HEIGHT / 2 + 8,
        0);
    if (spriteId < MAX_SPRITES)
    {
        gSprites[spriteId].data[0] = mapX;
        gSprites[spriteId].data[1] = mapY;
    }
}

static bool32 IsMatchingLedgeBehavior(u8 behavior, s16 dx, s16 dy)
{
    if (dx > 0)
        return behavior == MB_JUMP_EAST;
    if (dx < 0)
        return behavior == MB_JUMP_WEST;
    if (dy > 0)
        return behavior == MB_JUMP_SOUTH;
    if (dy < 0)
        return behavior == MB_JUMP_NORTH;
    return FALSE;
}

static bool32 CanStartLedgeJump(s16 dx, s16 dy)
{
    const s16 currentMapX = gSaveBlock1Ptr->pos.x;
    const s16 currentMapY = gSaveBlock1Ptr->pos.y - PORT_PLAYER_MAP_Y_BIAS;
    const s16 ledgeMapX = currentMapX + dx;
    const s16 ledgeMapY = currentMapY + dy;
    const s16 landingMapX = currentMapX + dx * 2;
    const s16 landingMapY = currentMapY + dy * 2;
    const s16 ledgeGridX = ledgeMapX + MAP_OFFSET;
    const s16 ledgeGridY = ledgeMapY + MAP_OFFSET;
    const s16 landingGridX = landingMapX + MAP_OFFSET;
    const s16 landingGridY = landingMapY + MAP_OFFSET;
    const u8 ledgeBehavior =
        MapGridGetMetatileBehaviorAt(ledgeGridX, ledgeGridY);

    if (!IsMatchingLedgeBehavior(ledgeBehavior, dx, dy))
        return FALSE;

    // A ledge jump traverses both the ledge tile and the landing tile. Static
    // actors such as Birch/Zigzagoon must block either space; otherwise the
    // two-tile jump can visually pass straight through them.
    if (PortTestNpc_BlocksTile(ledgeMapX, ledgeMapY)
     || PortTestNpc_BlocksTile(landingMapX, landingMapY))
        return FALSE;
    if (GetMapBorderIdAt(landingGridX, landingGridY) == CONNECTION_INVALID)
        return FALSE;
    if (MapGridGetCollisionAt(landingGridX, landingGridY) != 0)
        return FALSE;

    return TRUE;
}

static void BeginLedgeJump(s16 dx, s16 dy, u8 faceAnim)
{
    sPortFacing = faceAnim;
    sPortStepDx = dx;
    sPortStepDy = dy;
    sPortStepFrames = 16;
    sPortLedgeJump = TRUE;
    gFieldCamera.movementSpeedX = dx * 2;
    gFieldCamera.movementSpeedY = dy * 2;
    gSprites[sPortPlayerSpriteId].y2 = 0;
    StartSpriteAnimIfDifferent(&gSprites[sPortPlayerSpriteId], faceAnim);
}

static bool32 IsWarpEventAt(s16 x, s16 y)
{
    const struct MapEvents *events = gMapHeader.events;
    if (events == NULL || events->warps == NULL)
        return FALSE;

    for (u32 i = 0; i < events->warpCount; ++i)
    {
        if (events->warps[i].x == x
         && events->warps[i].y == y)
            return TRUE;
    }

    return FALSE;
}


static bool32 BgEventMatchesFacing(const struct BgEvent *event)
{
    switch (event->kind)
    {
    case BG_EVENT_PLAYER_FACING_ANY:
        return TRUE;
    case BG_EVENT_PLAYER_FACING_NORTH:
        return sPortFacing == PORT_ANIM_FACE_NORTH;
    case BG_EVENT_PLAYER_FACING_SOUTH:
        return sPortFacing == PORT_ANIM_FACE_SOUTH;
    case BG_EVENT_PLAYER_FACING_EAST:
        return sPortFacing == PORT_ANIM_FACE_EAST;
    case BG_EVENT_PLAYER_FACING_WEST:
        return sPortFacing == PORT_ANIM_FACE_WEST;
    default:
        return FALSE;
    }
}

static bool32 TryInteractWithFacingEvent(void)
{
    const s16 playerX = gSaveBlock1Ptr->pos.x;
    const s16 playerY = gSaveBlock1Ptr->pos.y - PORT_PLAYER_MAP_Y_BIAS;
    s16 x = playerX;
    s16 y = playerY;

    switch (sPortFacing)
    {
    case PORT_ANIM_FACE_NORTH:
        --y;
        break;
    case PORT_ANIM_FACE_SOUTH:
        ++y;
        break;
    case PORT_ANIM_FACE_WEST:
        --x;
        break;
    case PORT_ANIM_FACE_EAST:
        ++x;
        break;
    }

    if (PortTestNpc_TryInteractAt(x, y, playerX, playerY))
        return TRUE;

    const struct MapEvents *events = gMapHeader.events;
    if (events == NULL || events->bgEvents == NULL)
        return FALSE;

    for (u32 i = 0; i < events->bgEventCount; ++i)
    {
        const struct BgEvent *event = &events->bgEvents[i];
        if (event->x != x || event->y != y || !BgEventMatchesFacing(event))
            continue;
        if (event->bgUnion.script == NULL)
            return FALSE;

        return PortTestDialogue_Open(event->bgUnion.script);
    }

    return FALSE;
}

static bool32 PortMetatileBlocksNorth(u8 behavior)
{
    return behavior == MB_IMPASSABLE_NORTH
        || behavior == MB_IMPASSABLE_NORTHEAST
        || behavior == MB_IMPASSABLE_NORTHWEST
        || behavior == MB_IMPASSABLE_SOUTH_AND_NORTH;
}

static bool32 PortMetatileBlocksSouth(u8 behavior)
{
    return behavior == MB_IMPASSABLE_SOUTH
        || behavior == MB_IMPASSABLE_SOUTHEAST
        || behavior == MB_IMPASSABLE_SOUTHWEST
        || behavior == MB_IMPASSABLE_SOUTH_AND_NORTH;
}

static bool32 PortMetatileBlocksWest(u8 behavior)
{
    return behavior == MB_IMPASSABLE_WEST
        || behavior == MB_IMPASSABLE_NORTHWEST
        || behavior == MB_IMPASSABLE_SOUTHWEST
        || behavior == MB_IMPASSABLE_WEST_AND_EAST
        || behavior == MB_SECRET_BASE_BREAKABLE_DOOR;
}

static bool32 PortMetatileBlocksEast(u8 behavior)
{
    return behavior == MB_IMPASSABLE_EAST
        || behavior == MB_IMPASSABLE_NORTHEAST
        || behavior == MB_IMPASSABLE_SOUTHEAST
        || behavior == MB_IMPASSABLE_WEST_AND_EAST
        || behavior == MB_SECRET_BASE_BREAKABLE_DOOR;
}

static bool32 IsElevationMismatch(
    s16 currentGridX,
    s16 currentGridY,
    s16 targetGridX,
    s16 targetGridY)
{
    const u8 currentElevation =
        MapGridGetElevationAt(currentGridX, currentGridY);
    const u8 targetElevation =
        MapGridGetElevationAt(targetGridX, targetGridY);

    // Match Emerald's IsElevationMismatchAt semantics for ordinary walking:
    // transition (0) and multi-level (15) tiles accept either elevation.
    if (currentElevation == ELEVATION_TRANSITION
     || currentElevation == ELEVATION_MULTI_LEVEL
     || targetElevation == ELEVATION_TRANSITION
     || targetElevation == ELEVATION_MULTI_LEVEL)
        return FALSE;

    return currentElevation != targetElevation;
}

static bool32 IsDirectionBlockedByMetatile(
    s16 currentGridX,
    s16 currentGridY,
    s16 targetGridX,
    s16 targetGridY,
    s16 dx,
    s16 dy)
{
    const u8 currentBehavior =
        MapGridGetMetatileBehaviorAt(currentGridX, currentGridY);
    const u8 targetBehavior =
        MapGridGetMetatileBehaviorAt(targetGridX, targetGridY);

    if (dy > 0)
        return PortMetatileBlocksSouth(currentBehavior)
            || PortMetatileBlocksNorth(targetBehavior);
    if (dy < 0)
        return PortMetatileBlocksNorth(currentBehavior)
            || PortMetatileBlocksSouth(targetBehavior);
    if (dx < 0)
        return PortMetatileBlocksWest(currentBehavior)
            || PortMetatileBlocksEast(targetBehavior);
    if (dx > 0)
        return PortMetatileBlocksEast(currentBehavior)
            || PortMetatileBlocksWest(targetBehavior);

    return FALSE;
}

static bool32 CanStartStep(s16 dx, s16 dy)
{
    const s16 currentMapX = gSaveBlock1Ptr->pos.x;
    const s16 currentMapY = gSaveBlock1Ptr->pos.y - PORT_PLAYER_MAP_Y_BIAS;
    const s16 targetMapX = currentMapX + dx;
    const s16 targetMapY = currentMapY + dy;
    const s16 currentGridX = currentMapX + MAP_OFFSET;
    const s16 currentGridY = currentMapY + MAP_OFFSET;
    const s16 targetGridX = targetMapX + MAP_OFFSET;
    const s16 targetGridY = targetMapY + MAP_OFFSET;
    const bool32 targetIsWarp = IsWarpEventAt(targetMapX, targetMapY);

    if (PortTestNpc_BlocksTile(targetMapX, targetMapY))
        return FALSE;

    // Match Emerald's static collision checks: undefined borders are solid,
    // while a populated map connection remains a valid edge to walk across.
    if (!targetIsWarp
     && GetMapBorderIdAt(targetGridX, targetGridY) == CONNECTION_INVALID)
        return FALSE;

    // Use the same static-map collision pieces that Emerald relies on for
    // ordinary walking: collision bits, directional edges, and elevation.
    // Warp tiles remain enterable even when the doorway block itself carries
    // a collision bit.
    if (!targetIsWarp && MapGridGetCollisionAt(targetGridX, targetGridY) != 0)
        return FALSE;
    if (!targetIsWarp
     && IsDirectionBlockedByMetatile(
            currentGridX, currentGridY,
            targetGridX, targetGridY,
            dx, dy))
        return FALSE;
    // The reduced Android player does not yet track a persistent overworld
    // elevation like Emerald's ObjectEvent does. Restrict this check to indoor
    // maps, where it is needed for tables/counters, so outdoor ledges do not
    // block the player several tiles before the actual jump edge.
    if (!targetIsWarp
     && gMapHeader.mapType == MAP_TYPE_INDOOR
     && IsElevationMismatch(
            currentGridX, currentGridY,
            targetGridX, targetGridY))
        return FALSE;

    return TRUE;
}

static void BeginStep(s16 dx, s16 dy, u8 faceAnim, u8 walkAnim)
{
    sPortFacing = faceAnim;

    if (CanStartLedgeJump(dx, dy))
    {
        BeginLedgeJump(dx, dy, faceAnim);
        return;
    }

    if (!CanStartStep(dx, dy))
    {
        StartSpriteAnimIfDifferent(&gSprites[sPortPlayerSpriteId], faceAnim);
        return;
    }

    sPortStepDx = dx;
    sPortStepDy = dy;
    sPortStepFrames = 8;
    sPortLedgeJump = FALSE;

    // CameraUpdateNoObjectRefresh consumes pixel speeds and updates/redraws the
    // map one tile boundary at a time. Two pixels for eight frames is a native
    // 16-pixel Emerald tile step instead of the old instant tile snap.
    gFieldCamera.movementSpeedX = dx * 2;
    gFieldCamera.movementSpeedY = dy * 2;
    StartSpriteAnimIfDifferent(&gSprites[sPortPlayerSpriteId], walkAnim);
}

static void TryBeginStepFromKeys(u16 keys)
{
    if (keys & DPAD_UP)
        BeginStep(0, -1, PORT_ANIM_FACE_NORTH, PORT_ANIM_WALK_NORTH);
    else if (keys & DPAD_DOWN)
        BeginStep(0, 1, PORT_ANIM_FACE_SOUTH, PORT_ANIM_WALK_SOUTH);
    else if (keys & DPAD_LEFT)
        BeginStep(-1, 0, PORT_ANIM_FACE_WEST, PORT_ANIM_WALK_WEST);
    else if (keys & DPAD_RIGHT)
        BeginStep(1, 0, PORT_ANIM_FACE_EAST, PORT_ANIM_WALK_EAST);
}

void PortTestPlayer_Init(void)
{
    ResetSpriteData();
    FreeAllSpritePalettes();
    ClearSpriteCopyRequests();
    ResetCameraUpdateInfo();

    LoadSpritePalette(&sPortBrendanSpritePalette);
    LoadSpritePalette(&sPortGrassSpritePalette);
    sPortPlayerSpriteId = CreateSprite(
        &sPortBrendanTemplate,
        DISPLAY_WIDTH / 2,
        DISPLAY_HEIGHT / 2,
        0);

    sPortFacing = PORT_ANIM_FACE_SOUTH;
    sPortStepFrames = 0;
    sPortStepDx = 0;
    sPortStepDy = 0;
    sPortLedgeJump = FALSE;
    sPortFacingLockUntilRelease = FALSE;
    StartSpriteAnim(&gSprites[sPortPlayerSpriteId], sPortFacing);

    SetGpuRegBits(
        REG_OFFSET_DISPCNT,
        DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);

    AnimateSprites();
    BuildOamBuffer();
    LoadOam();
    ProcessSpriteCopyRequests();
}

void PortTestPlayer_Update(void)
{
    if (sPortPlayerSpriteId >= MAX_SPRITES)
        return;

    // A held direction used to enter a staircase used to overwrite the
    // destination-facing direction on the very next frame. Require the player
    // to release the d-pad once after a staircase warp, matching the visual
    // pause of the original transition.
    if (sPortFacingLockUntilRelease)
    {
        if (gMain.heldKeys & DPAD_ANY)
        {
            PortTestNpc_UpdateMovement(
                gSaveBlock1Ptr->pos.x,
                gSaveBlock1Ptr->pos.y - PORT_PLAYER_MAP_Y_BIAS);
            AnimateSprites();
            BuildOamBuffer();
            LoadOam();
            ProcessSpriteCopyRequests();
            return;
        }

        sPortFacingLockUntilRelease = FALSE;
    }

    if (sPortStepFrames == 0
     && (gMain.newKeys & A_BUTTON)
     && TryInteractWithFacingEvent())
    {
        AnimateSprites();
        BuildOamBuffer();
        LoadOam();
        ProcessSpriteCopyRequests();
        return;
    }

    if (sPortStepFrames != 0)
    {
        const s16 cameraDx = gFieldCamera.movementSpeedX;
        const s16 cameraDy = gFieldCamera.movementSpeedY;

        CameraUpdateNoObjectRefresh();
        PortTestNpc_ApplyCameraDelta(cameraDx, cameraDy);
        ApplyGrassCameraDelta(cameraDx, cameraDy);
        --sPortStepFrames;

        if (sPortLedgeJump)
        {
            const u8 elapsed = 16 - sPortStepFrames;
            const u8 arc = elapsed <= 8 ? elapsed : 16 - elapsed;
            gSprites[sPortPlayerSpriteId].y2 = -(s16)arc;
        }

        if (sPortStepFrames == 0)
        {
            gFieldCamera.movementSpeedX = 0;
            gFieldCamera.movementSpeedY = 0;
            sPortStepDx = 0;
            sPortStepDy = 0;
            sPortLedgeJump = FALSE;
            gSprites[sPortPlayerSpriteId].y2 = 0;
            StartSpriteAnimIfDifferent(&gSprites[sPortPlayerSpriteId], sPortFacing);
            SpawnTallGrassStepEffect();

            const s16 playerX = gSaveBlock1Ptr->pos.x;
            const s16 playerY =
                gSaveBlock1Ptr->pos.y - PORT_PLAYER_MAP_Y_BIAS;
            if (PortGame_TryTestWarpAt(playerX, playerY))
            {
                gFieldCamera.movementSpeedX = 0;
                gFieldCamera.movementSpeedY = 0;
                return;
            }

            if (PortGame_TryTestCoordEventAt(playerX, playerY))
            {
                AnimateSprites();
                BuildOamBuffer();
                LoadOam();
                ProcessSpriteCopyRequests();
                return;
            }

            // Continue walking seamlessly when the direction is still held.
            if (gMain.heldKeys & DPAD_ANY)
                TryBeginStepFromKeys(gMain.heldKeys);
        }
    }
    else
    {
        if (gMain.newKeys & DPAD_ANY)
            TryBeginStepFromKeys(gMain.newKeys);
        else if (gMain.heldKeys & DPAD_ANY)
            TryBeginStepFromKeys(gMain.heldKeys);
    }

    PortTestNpc_UpdateMovement(
        gSaveBlock1Ptr->pos.x,
        gSaveBlock1Ptr->pos.y - PORT_PLAYER_MAP_Y_BIAS);

    AnimateSprites();
    BuildOamBuffer();
    LoadOam();
    ProcessSpriteCopyRequests();
}


void PortTestPlayer_SetFacingDirection(u8 direction)
{
    if (sPortPlayerSpriteId >= MAX_SPRITES)
        return;

    switch (direction)
    {
    case DIR_NORTH:
        sPortFacing = PORT_ANIM_FACE_NORTH;
        break;
    case DIR_WEST:
        sPortFacing = PORT_ANIM_FACE_WEST;
        break;
    case DIR_EAST:
        sPortFacing = PORT_ANIM_FACE_EAST;
        break;
    case DIR_SOUTH:
    default:
        sPortFacing = PORT_ANIM_FACE_SOUTH;
        break;
    }

    StartSpriteAnimIfDifferent(&gSprites[sPortPlayerSpriteId], sPortFacing);
    sPortFacingLockUntilRelease = TRUE;
}
