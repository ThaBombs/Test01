#include "port_test_player.h"
#include "port_test_overworld.h"

#include "global.h"
#include "field_camera.h"
#include "fieldmap.h"
#include "gpu_regs.h"
#include "main.h"
#include "sprite.h"
#include "constants/maps.h"

#define PORT_PLAYER_PAL_TAG 0x7F01

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

static const struct SpriteFrameImage sPortBrendanFrames[] =
{
    overworld_ascending_frames(sPortBrendanWalking, 2, 4),
};

static const struct OamData sPortBrendanOam =
{
    .shape = SPRITE_SHAPE(16x32),
    .size = SPRITE_SIZE(16x32),
    .priority = 2,
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

static const struct SpritePalette sPortBrendanSpritePalette =
{
    .data = sPortBrendanPalette,
    .tag = PORT_PLAYER_PAL_TAG,
};

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

static u32 sPortPlayerSpriteId = MAX_SPRITES;
static u8 sPortFacing = PORT_ANIM_FACE_SOUTH;
static u8 sPortStepFrames;
static s8 sPortStepDx;
static s8 sPortStepDy;

static bool32 IsStaticEventObstacle(s16 x, s16 y)
{
    const struct MapEvents *events = gMapHeader.events;
    if (events == NULL || events->bgEvents == NULL)
        return FALSE;

    for (u32 i = 0; i < events->bgEventCount; ++i)
    {
        if (events->bgEvents[i].x == x && events->bgEvents[i].y == y)
            return TRUE;
    }

    return FALSE;
}

static bool32 CanStartStep(s16 dx, s16 dy)
{
    const s16 targetMapX = gSaveBlock1Ptr->pos.x + dx;
    const s16 targetMapY = gSaveBlock1Ptr->pos.y + dy;
    const s16 targetGridX = targetMapX + MAP_OFFSET;
    const s16 targetGridY = targetMapY + MAP_OFFSET;

    if (MapGridGetCollisionAt(targetGridX, targetGridY) != 0)
        return FALSE;
    if (IsStaticEventObstacle(targetMapX, targetMapY))
        return FALSE;

    return TRUE;
}

static void BeginStep(s16 dx, s16 dy, u8 faceAnim, u8 walkAnim)
{
    sPortFacing = faceAnim;

    if (!CanStartStep(dx, dy))
    {
        StartSpriteAnimIfDifferent(&gSprites[sPortPlayerSpriteId], faceAnim);
        return;
    }

    sPortStepDx = dx;
    sPortStepDy = dy;
    sPortStepFrames = 8;

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
    sPortPlayerSpriteId = CreateSprite(
        &sPortBrendanTemplate,
        DISPLAY_WIDTH / 2,
        DISPLAY_HEIGHT / 2,
        0);

    sPortFacing = PORT_ANIM_FACE_SOUTH;
    sPortStepFrames = 0;
    sPortStepDx = 0;
    sPortStepDy = 0;
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

    if (sPortStepFrames != 0)
    {
        CameraUpdateNoObjectRefresh();
        --sPortStepFrames;

        if (sPortStepFrames == 0)
        {
            gFieldCamera.movementSpeedX = 0;
            gFieldCamera.movementSpeedY = 0;
            sPortStepDx = 0;
            sPortStepDy = 0;
            StartSpriteAnimIfDifferent(&gSprites[sPortPlayerSpriteId], sPortFacing);

            const s16 playerX = gSaveBlock1Ptr->pos.x;
            const s16 playerY = gSaveBlock1Ptr->pos.y;
            if (PortGame_TryTestWarpAt(playerX, playerY))
            {
                gFieldCamera.movementSpeedX = 0;
                gFieldCamera.movementSpeedY = 0;
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

    AnimateSprites();
    BuildOamBuffer();
    LoadOam();
    ProcessSpriteCopyRequests();
}
